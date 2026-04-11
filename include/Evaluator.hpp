/**
 * @file Evaluator.hpp
 * @brief AST Interpreter for Rhodesia language
 *
 * Implements the Visitor pattern to evaluate AST nodes.
 * Uses Eigen for all matrix/vector operations.
 */

#ifndef RHODESIA_EVALUATOR_HPP
#define RHODESIA_EVALUATOR_HPP

#include "AST.hpp"
#include "RhoValue.hpp"
#include "SymbolTable.hpp"
#include "Builtins.hpp"
#include "Error.hpp"
#include "ModuleLoader.hpp"
#include <iostream>

namespace Rhodesia {

/**
 * @brief Control flow exception for return statements
 */
class ReturnValue : public std::exception {
public:
    explicit ReturnValue(RhoValue val) : value_(std::move(val)) {}
    RhoValue value_;
};

/**
 * @brief Interpreter that evaluates Rhodesia AST
 */
class Evaluator : public ASTVisitor {
public:
    Evaluator() : result_(int64_t(0)) {}

    /**
     * @brief Evaluate a program
     * @return Final result value
     */
    RhoValue evaluate(ProgramNode& program) {
        program.accept(*this);
        return result_;
    }

    /**
     * @brief Access to symbol table for debugging/introspection
     */
    SymbolTable& symbols() { return symbols_; }

    /**
     * @brief Access to module loader
     */
    ModuleLoader& moduleLoader() { return moduleLoader_; }

    // ========================================================================
    // Expression Visitors
    // ========================================================================

    void visit(IntLiteralNode& node) override {
        result_ = node.value;
    }

    void visit(FloatLiteralNode& node) override {
        result_ = node.value;
    }

    void visit(BoolLiteralNode& node) override {
        result_ = node.value;
    }

    void visit(StringLiteralNode& node) override {
        result_ = node.value;
    }

    void visit(NullLiteralNode& node) override {
        result_ = std::make_shared<RhoNull>();
    }

    void visit(VectorLiteralNode& node) override {
        Eigen::VectorXd vec(node.elements.size());
        for (size_t i = 0; i < node.elements.size(); ++i) {
            node.elements[i]->accept(*this);
            // Convert both int and float64 to double for vector storage
            vec(i) = toDouble(result_);
        }
        result_ = vec;
    }

    void visit(MatrixLiteralNode& node) override {
        if (node.rows.empty()) {
            result_ = Eigen::MatrixXd(0, 0);
            return;
        }

        size_t rows = node.rows.size();
        size_t cols = node.rows[0].size();

        Eigen::MatrixXd mat(rows, cols);
        for (size_t i = 0; i < rows; ++i) {
            if (node.rows[i].size() != cols) {
                throw RuntimeError("Matrix rows must have consistent length", node.location);
            }
            for (size_t j = 0; j < cols; ++j) {
                node.rows[i][j]->accept(*this);
                mat(i, j) = toDouble(result_);
            }
        }
        result_ = mat;
    }

    void visit(IdentifierNode& node) override {
        result_ = symbols_.lookup(node.name, node.location);
    }

    void visit(BinaryOpNode& node) override {
        node.left->accept(*this);
        RhoValue left = result_;

        node.right->accept(*this);
        RhoValue right = result_;

        result_ = applyBinaryOp(node.op, left, right, node.location);
    }

    void visit(UnaryOpNode& node) override {
        node.operand->accept(*this);
        RhoValue operand = result_;

        if (node.op == UnaryOp::Neg) {
            result_ = applyNegation(operand, node.location);
        } else if (node.op == UnaryOp::Not) {
            result_ = !isTruthy(operand);
        } else if (node.op == UnaryOp::BitNot) {
            result_ = applyBitNot(operand, node.location);
        }
    }

    void visit(TernaryOpNode& node) override {
        // Evaluate condition
        node.condition->accept(*this);
        bool conditionValue = isTruthy(result_);

        // Only evaluate the corresponding branch (lazy evaluation)
        if (conditionValue) {
            node.trueExpr->accept(*this);
        } else {
            node.falseExpr->accept(*this);
        }
        // result_ is already set by the evaluated branch
    }

    void visit(FunctionCallNode& node) override {
        // Check for built-in functions first
        if (Builtins::instance().isBuiltin(node.name)) {
            std::vector<RhoValue> args;
            for (auto& arg : node.arguments) {
                arg->accept(*this);
                args.push_back(result_);
            }
            result_ = Builtins::instance().call(node.name, args, node.location);
            return;
        }

        // Check if it's a variable holding a function (lambda or function value)
        if (symbols_.exists(node.name)) {
            RhoValue maybeFunc = symbols_.lookup(node.name, node.location);
            if (std::holds_alternative<std::shared_ptr<RhoFunction>>(maybeFunc)) {
                auto func = std::get<std::shared_ptr<RhoFunction>>(maybeFunc);

                // Evaluate arguments
                std::vector<RhoValue> args;
                for (auto& arg : node.arguments) {
                    arg->accept(*this);
                    args.push_back(result_);
                }

                // Call the function value
                result_ = callLambda(func, args, node.location);
                return;
            }
        }

        // Check for user-defined functions
        const auto* funcSig = symbols_.lookupFunction(node.name);
        if (!funcSig) {
            throw RuntimeError::undefinedFunction(node.name, node.location);
        }

        // Validate argument count
        if (node.arguments.size() != funcSig->params.size()) {
            throw ArgumentError(node.name, "expected " + std::to_string(funcSig->params.size()) +
                               " arguments, got " + std::to_string(node.arguments.size()), node.location);
        }

        // Evaluate arguments and create new scope
        std::vector<RhoValue> args;
        for (auto& arg : node.arguments) {
            arg->accept(*this);
            args.push_back(result_);
        }

        // Create function scope
        ScopeGuard funcGuard(symbols_);

        // Bind parameters
        for (size_t i = 0; i < args.size(); ++i) {
            symbols_.declare(funcSig->params[i].name, funcSig->params[i].type, args[i], node.location);
        }

        // Execute function body
        try {
            funcSig->declaration->body->accept(*this);
            // If no return statement, return void (0)
            result_ = int64_t(0);
        } catch (const ReturnValue& ret) {
            result_ = ret.value_;
        }
    }

    void visit(MemberAccessNode& node) override {
        // Check if it's a record field access (variable.field)
        if (node.arguments.empty() && symbols_.exists(node.object)) {
            RhoValue obj = symbols_.lookup(node.object, node.location);
            if (std::holds_alternative<std::shared_ptr<RhoRecord>>(obj)) {
                auto rec = std::get<std::shared_ptr<RhoRecord>>(obj);
                try {
                    result_ = rec->getField(node.member);
                    return;
                } catch (const std::runtime_error& e) {
                    throw RuntimeError(e.what(), node.location);
                }
            }
        }

        // Check if it's a module constant (e.g., math.PI without arguments)
        if (node.arguments.empty() && Builtins::instance().isModuleConstant(node.object, node.member)) {
            result_ = Builtins::instance().getModuleConstant(node.object, node.member, node.location);
            return;
        }

        // Handle module.function() calls (e.g., math.zeros(5))
        if (Builtins::instance().isModuleFunction(node.object, node.member)) {
            std::vector<RhoValue> args;
            for (auto& arg : node.arguments) {
                arg->accept(*this);
                args.push_back(result_);
            }
            result_ = Builtins::instance().callModule(node.object, node.member, args, node.location);
            return;
        }

        throw RuntimeError("Unknown module or member: " + node.object + "." + node.member, node.location);
    }

    void visit(IndexAccessNode& node) override {
        node.target->accept(*this);
        RhoValue target = result_;

        // Check if this is map access with string key
        if (std::holds_alternative<std::shared_ptr<RhoMap>>(target) && node.indices.size() == 1) {
            node.indices[0]->accept(*this);
            if (std::holds_alternative<std::string>(result_)) {
                auto map = std::get<std::shared_ptr<RhoMap>>(target);
                result_ = map->get(std::get<std::string>(result_));
                return;
            }
        }

        // Check if this is tuple access
        if (std::holds_alternative<std::shared_ptr<RhoTuple>>(target) && node.indices.size() == 1) {
            node.indices[0]->accept(*this);
            size_t idx = static_cast<size_t>(toInt(result_));
            auto tup = std::get<std::shared_ptr<RhoTuple>>(target);
            try {
                result_ = tup->get(idx);
            } catch (const std::out_of_range& e) {
                throw RuntimeError(e.what(), node.location);
            }
            return;
        }

        // Check if this is array access
        if (std::holds_alternative<std::shared_ptr<RhoArray>>(target) && node.indices.size() == 1) {
            node.indices[0]->accept(*this);
            size_t idx = static_cast<size_t>(toInt(result_));
            auto arr = std::get<std::shared_ptr<RhoArray>>(target);
            result_ = arr->get(idx);
            return;
        }

        // Otherwise, use numeric indexing (vectors/matrices)
        std::vector<size_t> indices;
        for (auto& idx : node.indices) {
            idx->accept(*this);
            indices.push_back(static_cast<size_t>(toInt(result_)));
        }

        result_ = applyIndexing(target, indices, node.location);
    }

    void visit(SliceNode& node) override {
        node.target->accept(*this);
        RhoValue target = result_;

        // Evaluate slice specifications - now supporting negative indices
        std::vector<EvaluatedSlice> evalSlices;
        for (auto& slice : node.slices) {
            EvaluatedSlice evalSlice;

            if (slice.start.has_value()) {
                slice.start.value()->accept(*this);
                evalSlice.start = toInt(result_);  // Keep as int64_t to support negatives
            }

            if (slice.end.has_value()) {
                slice.end.value()->accept(*this);
                evalSlice.end = toInt(result_);  // Keep as int64_t to support negatives
            }

            evalSlices.push_back(evalSlice);
        }

        result_ = applySlicing(target, evalSlices, node.location);
    }

    void visit(LambdaNode& node) override {
        // Create a closure by capturing the current environment
        std::unordered_map<std::string, RhoValue> closure;

        // Capture variables from current scope, but avoid circular references
        // by NOT capturing other functions (which may themselves have closures)
        auto allSymbols = symbols_.getAllCurrentScopeSymbols();
        for (const auto& [name, symbol] : allSymbols) {
            // Only capture non-function values to avoid circular references
            // Functions will be resolved at call time via the symbol table
            if (getValueType(symbol.value) != RhoType::Function) {
                closure[name] = symbol.value;
            }
        }

        // Extract parameter names
        std::vector<std::string> paramNames;
        for (const auto& param : node.params) {
            paramNames.push_back(param.name);
        }

        // Create a non-owning shared_ptr to the lambda body
        // The AST node is owned by the parent ProgramNode and lives for the entire execution
        // Using a custom deleter that does nothing to avoid double-free
        std::shared_ptr<void> bodyPtr(node.body.get(), [](void*){});

        // Create RhoFunction with closure
        auto func = std::make_shared<RhoFunction>(
            std::move(paramNames),
            bodyPtr,
            node.isExpression,
            std::move(closure)
        );

        result_ = func;
    }

    void visit(SetLiteralNode& node) override {
        auto set = std::make_shared<RhoSet>();
        for (auto& elem : node.elements) {
            elem->accept(*this);
            set->add(result_);
        }
        result_ = set;
    }

    void visit(TupleLiteralNode& node) override {
        std::vector<RhoValue> elements;
        elements.reserve(node.elements.size());
        for (auto& elem : node.elements) {
            elem->accept(*this);
            elements.push_back(result_);
        }
        result_ = std::make_shared<RhoTuple>(std::move(elements));
    }

    void visit(RecordLiteralNode& node) override {
        auto record = std::make_shared<RhoRecord>();
        for (auto& [key, valueExpr] : node.fields) {
            valueExpr->accept(*this);
            record->setField(key, result_);
        }
        result_ = record;
    }

    // ========================================================================
    // Statement Visitors
    // ========================================================================

    void visit(VarDeclNode& node) override {
        node.initializer->accept(*this);
        // Type check and convert if needed
        RhoType initType = getValueType(result_);

        // Convert value to declared type if compatible
        result_ = convertToType(result_, node.type, initType, node.location);

        symbols_.declare(node.name, node.type, result_, node.location);
    }

    void visit(AssignmentNode& node) override {
        if (node.indices.empty()) {
            // Simple assignment
            node.value->accept(*this);
            symbols_.assign(node.name, result_, node.location);
        } else {
            // Indexed assignment
            // First get the current value
            RhoValue current = symbols_.lookup(node.name, node.location);

            // Evaluate the value to assign
            node.value->accept(*this);
            RhoValue newValue = result_;

            // Check if this is map assignment with string key
            if (std::holds_alternative<std::shared_ptr<RhoMap>>(current) && node.indices.size() == 1) {
                node.indices[0]->accept(*this);
                if (std::holds_alternative<std::string>(result_)) {
                    auto map = std::get<std::shared_ptr<RhoMap>>(current);
                    map->set(std::get<std::string>(result_), newValue);
                    // No need to reassign, map is modified in place
                    return;
                }
            }

            // Check if this is array assignment
            if (std::holds_alternative<std::shared_ptr<RhoArray>>(current) && node.indices.size() == 1) {
                node.indices[0]->accept(*this);
                size_t idx = static_cast<size_t>(toInt(result_));
                auto arr = std::get<std::shared_ptr<RhoArray>>(current);
                arr->set(idx, newValue);
                // No need to reassign, array is modified in place
                return;
            }

            // Otherwise, use numeric indexing (vectors/matrices)
            // Evaluate indices
            std::vector<size_t> indices;
            for (auto& idx : node.indices) {
                idx->accept(*this);
                indices.push_back(static_cast<size_t>(toInt(result_)));
            }

            // Apply indexed assignment
            result_ = applyIndexedAssignment(current, indices, newValue, node.location);
            symbols_.assign(node.name, result_, node.location);
        }
    }

    void visit(TupleDestructureNode& node) override {
        node.rhs->accept(*this);
        RhoValue rhs = result_;

        // Unpack tuple
        if (std::holds_alternative<std::shared_ptr<RhoTuple>>(rhs)) {
            auto tup = std::get<std::shared_ptr<RhoTuple>>(rhs);
            if (tup->size() != node.targets.size()) {
                throw RuntimeError("Tuple size mismatch in destructuring: expected " +
                    std::to_string(node.targets.size()) + " but got " +
                    std::to_string(tup->size()), node.location);
            }
            for (size_t i = 0; i < node.targets.size(); i++) {
                RhoValue val = convertToType(tup->get(i), node.targets[i].type,
                    getValueType(tup->get(i)), node.location);
                symbols_.declare(node.targets[i].name, node.targets[i].type, val, node.location);
            }
        }
        // Unpack vector (for functions returning vec)
        else if (std::holds_alternative<Eigen::VectorXd>(rhs)) {
            const auto& vec = std::get<Eigen::VectorXd>(rhs);
            if (static_cast<size_t>(vec.size()) != node.targets.size()) {
                throw RuntimeError("Vector size mismatch in destructuring: expected " +
                    std::to_string(node.targets.size()) + " but got " +
                    std::to_string(vec.size()), node.location);
            }
            for (size_t i = 0; i < node.targets.size(); i++) {
                RhoValue elem = vec(static_cast<Eigen::Index>(i));
                RhoValue val = convertToType(elem, node.targets[i].type,
                    getValueType(elem), node.location);
                symbols_.declare(node.targets[i].name, node.targets[i].type, val, node.location);
            }
        }
        else {
            throw RuntimeError("Cannot destructure non-tuple value", node.location);
        }
    }

    void visit(ExprStmtNode& node) override {
        node.expression->accept(*this);
    }

    void visit(ReturnNode& node) override {
        if (node.value) {
            node.value->accept(*this);
            throw ReturnValue(result_);
        } else {
            throw ReturnValue(int64_t(0));
        }
    }

    void visit(BlockNode& node) override {
        ScopeGuard blockGuard(symbols_);
        for (auto& stmt : node.statements) {
            stmt->accept(*this);
        }
    }

    void visit(FunctionDeclNode& node) override {
        symbols_.declareFunction(node.name, &node);
    }

    void visit(ForLoopNode& node) override {
        node.iterable->accept(*this);
        RhoValue iterable = result_;

        if (std::holds_alternative<std::shared_ptr<RangeGenerator>>(iterable)) {
            // Handle RangeGenerator (lazy evaluation)
            auto rangeGen = std::get<std::shared_ptr<RangeGenerator>>(iterable);
            rangeGen->reset(); // Reset to start

            // Create scope once outside the loop
            ScopeGuard iterGuard(symbols_);
            // Declare the iterator variable once
            symbols_.declare(node.iterVar, RhoType::Float64, 0.0, node.location);

            while (rangeGen->hasNext()) {
                // Fast unchecked assignment for performance (type is guaranteed to be float64)
                symbols_.assignUnchecked(node.iterVar, rangeGen->next());

                try {
                    node.body->accept(*this);
                } catch (const BreakException&) {
                    break;
                } catch (const ContinueException&) {
                    continue;
                }
            }
        }
        else if (std::holds_alternative<Eigen::VectorXd>(iterable)) {
            // Handle regular vector (backward compatibility)
            const Eigen::VectorXd& vec = std::get<Eigen::VectorXd>(iterable);

            // Create scope once outside the loop
            ScopeGuard iterGuard(symbols_);
            // Declare the iterator variable once (with initial value from vec if non-empty)
            double initialVal = vec.size() > 0 ? vec(0) : 0.0;
            symbols_.declare(node.iterVar, RhoType::Float64, initialVal, node.location);

            for (Eigen::Index i = 0; i < vec.size(); ++i) {
                // Fast unchecked assignment for performance (type is guaranteed to be float64)
                symbols_.assignUnchecked(node.iterVar, vec(i));

                try {
                    node.body->accept(*this);
                } catch (const BreakException&) {
                    break;
                } catch (const ContinueException&) {
                    continue;
                }
            }
        }
        else {
            throw RuntimeError::invalidForLoopIterable(typeToString(getValueType(iterable)), node.location);
        }
    }

    void visit(WhileLoopNode& node) override {
        while (true) {
            node.condition->accept(*this);
            if (!isTruthy(result_)) break;

            ScopeGuard iterGuard(symbols_);

            try {
                node.body->accept(*this);
            } catch (const BreakException&) {
                break;
            } catch (const ContinueException&) {
                continue;
            }
        }
    }

    void visit(IfStmtNode& node) override {
        node.condition->accept(*this);

        if (isTruthy(result_)) {
            node.thenBranch->accept(*this);
        } else if (node.elseBranch) {
            node.elseBranch->accept(*this);
        }
    }

    void visit(BreakNode& node) override {
        throw BreakException();
    }

    void visit(ContinueNode& node) override {
        throw ContinueException();
    }

    void visit(UsingNode& node) override {
        // Evaluate the resource expression (e.g., io.open(...))
        node.resourceExpr->accept(*this);
        RhoValue resource = result_;

        // Declare the variable in current scope
        symbols_.declare(node.varName, getValueType(resource), resource, node.location);

        // Execute the body
        try {
            node.body->accept(*this);
        } catch (...) {
            // Even if there's an exception, we need to close the resource
            // Call io.close() on the resource
            closeResource(resource, node.location);
            throw; // Re-throw the exception
        }

        // Normally close the resource after execution
        closeResource(resource, node.location);
    }

    void visit(ThrowNode& node) override {
        // Evaluate the expression to throw
        node.expression->accept(*this);
        RhoValue exceptionValue = result_;

        // Throw a UserException with the evaluated value
        throw UserException(exceptionValue, node.location);
    }

    void visit(TryCatchNode& node) override {
        try {
            // Execute the try block
            node.tryBody->accept(*this);
        } catch (const UserException& e) {
            // User-thrown exception - bind to catch variable
            ScopeGuard catchGuard(symbols_);

            // Determine the type and value to bind
            RhoValue exceptionValue;
            RhoType exceptionType;

            if (e.hasValue()) {
                exceptionValue = e.value();
                exceptionType = getValueType(exceptionValue);
            } else {
                // If no value, create a string with the error message
                exceptionValue = std::string(e.what());
                exceptionType = RhoType::String;
            }

            // Declare the exception variable in the catch scope
            symbols_.declare(node.catchClause.exceptionVar, exceptionType, exceptionValue, node.location);

            // Execute the catch block
            node.catchClause.body->accept(*this);
        } catch (const RhoError& e) {
            // Built-in Rhodesia error - convert to string and bind to catch variable
            ScopeGuard catchGuard(symbols_);

            // Bind the error message as a string
            std::string errorMsg = e.what();
            symbols_.declare(node.catchClause.exceptionVar, RhoType::String, errorMsg, node.location);

            // Execute the catch block
            node.catchClause.body->accept(*this);
        }
    }

    void visit(MatchStmtNode& node) override {
        node.scrutinee->accept(*this);
        RhoValue scrutinee = result_;

        for (auto& matchCase : node.cases) {
            if (!matchCase.pattern) {
                // Wildcard: always matches
                matchCase.body->accept(*this);
                return;
            }

            matchCase.pattern->accept(*this);
            if (rhoValuesEqual(scrutinee, result_)) {
                matchCase.body->accept(*this);
                return;
            }
        }
        // No case matched — no-op (exhaustive check is user's responsibility)
    }

    void visit(IncludeNode& node) override {
        // Check for built-in modules that don't have .rho files
        if (node.moduleName == "datetime") {
            importBuiltinModule("datetime", node.symbols, node.location);
            return;
        }

        // Load the module
        ProgramNode* moduleAst = moduleLoader_.loadModule(node.moduleName, node.location);

        // Create a temporary evaluator to execute the module in isolation
        Evaluator moduleEvaluator;
        // Share module cache by setting the same base directory
        moduleEvaluator.moduleLoader_.setBaseDirectory(
            moduleLoader_.getBaseDirectory()
        );
        moduleAst->accept(moduleEvaluator);

        // Import symbols from the module's symbol table
        if (node.symbols.empty()) {
            // Import all symbols
            importAllSymbols(moduleEvaluator.symbols_, node.location);
        } else {
            // Import only specified symbols (with optional aliases)
            for (const auto& importSpec : node.symbols) {
                importSymbol(moduleEvaluator.symbols_, importSpec.symbolName,
                           importSpec.getImportedName(), node.location);
            }
        }
    }

    void visit(ProgramNode& node) override {
        for (auto& stmt : node.statements) {
            stmt->accept(*this);
        }
    }

private:
    SymbolTable symbols_;
    RhoValue result_;
    ModuleLoader moduleLoader_;

    // ========================================================================
    // Module Import Helpers
    // ========================================================================

    /**
     * @brief Import a specific symbol from a module's symbol table with optional alias
     * @param moduleSymbols Symbol table of the module
     * @param symbolName Original name in the module
     * @param importedName Name to use in the importing scope (can be an alias)
     * @param location Source location for error reporting
     */
    void importSymbol(SymbolTable& moduleSymbols, const std::string& symbolName,
                     const std::string& importedName, const SourceLocation& location) {
        // Try to import variable
        try {
            RhoValue value = moduleSymbols.lookup(symbolName, location);
            RhoType type = getValueType(value);
            symbols_.declare(importedName, type, value, location);
            return;
        } catch (const RuntimeError&) {
            // Not a variable, try function
        }

        // Try to import function
        const auto* funcSig = moduleSymbols.lookupFunction(symbolName);
        if (funcSig) {
            symbols_.declareFunction(importedName, funcSig->declaration);
            return;
        }

        // Symbol not found
        throw RuntimeError("Symbol '" + symbolName + "' not found in module", location);
    }

    /**
     * @brief Import all symbols from a module's symbol table
     */
    void importAllSymbols(SymbolTable& moduleSymbols, const SourceLocation& location) {
        // Import all global variables
        auto globalSymbols = moduleSymbols.getAllGlobalSymbols();
        for (const auto& [name, symbol] : globalSymbols) {
            symbols_.declare(name, symbol.type, symbol.value, location);
        }

        // Import all functions
        auto allFunctions = moduleSymbols.getAllFunctions();
        for (const auto& [name, funcSig] : allFunctions) {
            symbols_.declareFunction(name, funcSig.declaration);
        }
    }

    /**
     * @brief Import a built-in module's functions as first-class RhoFunction values
     */
    void importBuiltinModule(const std::string& moduleName,
                             const std::vector<ImportSpec>& symbols,
                             const SourceLocation& loc) {
        auto& builtins = Builtins::instance();

        auto importOne = [&](const std::string& funcName, const std::string& importedName) {
            if (!builtins.isModuleFunction(moduleName, funcName) &&
                !builtins.isModuleConstant(moduleName, funcName)) {
                throw RuntimeError("Symbol '" + funcName + "' not found in module '" + moduleName + "'", loc);
            }
            if (builtins.isModuleConstant(moduleName, funcName)) {
                RhoValue val = builtins.getModuleConstant(moduleName, funcName, loc);
                symbols_.declare(importedName, getValueType(val), val, loc);
                return;
            }
            // Wrap built-in module function as a native RhoFunction
            auto native = std::make_shared<RhoFunction>(
                [moduleName, funcName](const std::vector<RhoValue>& args) -> RhoValue {
                    return Builtins::instance().callModule(moduleName, funcName, args, {});
                }
            );
            symbols_.declare(importedName, RhoType::Function,
                             std::shared_ptr<RhoFunction>(native), loc);
        };

        if (symbols.empty()) {
            throw RuntimeError(
                "include " + moduleName + ": selective import required (e.g. include " +
                moduleName + "{now, today})", loc);
        }

        for (const auto& spec : symbols) {
            importOne(spec.symbolName, spec.getImportedName());
        }
    }

    // ========================================================================
    // Helper Types
    // ========================================================================

    /**
     * @brief Evaluated slice specification with concrete indices
     * Uses int64_t to support negative indices (Python-style)
     */
    struct EvaluatedSlice {
        std::optional<int64_t> start;
        std::optional<int64_t> end;
    };

    // ========================================================================
    // Helper Functions
    // ========================================================================

    /**
     * @brief Convert value to target type
     */
    RhoValue convertToType(const RhoValue& value, RhoType targetType, RhoType sourceType, SourceLocation loc) {
        // Same type, no conversion needed
        if (sourceType == targetType) {
            return value;
        }

        // Allow Int (default) to accept all integer types
        if (targetType == RhoType::Int && (
            sourceType == RhoType::Int8 || sourceType == RhoType::Int16 ||
            sourceType == RhoType::Int32 || sourceType == RhoType::UInt8 ||
            sourceType == RhoType::UInt16 || sourceType == RhoType::UInt32 ||
            sourceType == RhoType::UInt64 || sourceType == RhoType::Byte)) {
            // Convert to int64_t
            return static_cast<int64_t>(toInt(value));
        }

        // Convert integer literals to specific integer types
        if (sourceType == RhoType::Int) {
            int64_t intVal = std::get<int64_t>(value);

            switch (targetType) {
                case RhoType::Int8:
                    if (intVal < INT8_MIN || intVal > INT8_MAX)
                        throw TypeError("Value out of range for int8", loc);
                    return static_cast<int8_t>(intVal);
                case RhoType::Int16:
                    if (intVal < INT16_MIN || intVal > INT16_MAX)
                        throw TypeError("Value out of range for int16", loc);
                    return static_cast<int16_t>(intVal);
                case RhoType::Int32:
                    if (intVal < INT32_MIN || intVal > INT32_MAX)
                        throw TypeError("Value out of range for int32", loc);
                    return static_cast<int32_t>(intVal);
                case RhoType::UInt8:
                case RhoType::Byte:
                    if (intVal < 0 || intVal > UINT8_MAX)
                        throw TypeError("Value out of range for uint8/byte", loc);
                    return static_cast<uint8_t>(intVal);
                case RhoType::UInt16:
                    if (intVal < 0 || intVal > UINT16_MAX)
                        throw TypeError("Value out of range for uint16", loc);
                    return static_cast<uint16_t>(intVal);
                case RhoType::UInt32:
                    if (intVal < 0 || intVal > UINT32_MAX)
                        throw TypeError("Value out of range for uint32", loc);
                    return static_cast<uint32_t>(intVal);
                case RhoType::UInt64:
                    if (intVal < 0)
                        throw TypeError("Value out of range for uint64", loc);
                    return static_cast<uint64_t>(intVal);
                default:
                    break;
            }
        }

        // If no conversion found, check type compatibility
        if (sourceType != targetType) {
            throw TypeError::expectedType("Type conversion", targetType, sourceType);
        }

        return value;
    }

    /**
     * @brief Structural equality between two RhoValues (used by match)
     */
    bool rhoValuesEqual(const RhoValue& a, const RhoValue& b) const {
        // Allow int/float cross-type comparison
        if (std::holds_alternative<int64_t>(a) && std::holds_alternative<double>(b)) {
            return static_cast<double>(std::get<int64_t>(a)) == std::get<double>(b);
        }
        if (std::holds_alternative<double>(a) && std::holds_alternative<int64_t>(b)) {
            return std::get<double>(a) == static_cast<double>(std::get<int64_t>(b));
        }
        if (a.index() != b.index()) return false;

        return std::visit([&b](const auto& arg_a) -> bool {
            using T = std::decay_t<decltype(arg_a)>;
            if constexpr (std::is_same_v<T, int64_t>  || std::is_same_v<T, int8_t>   ||
                          std::is_same_v<T, int16_t>  || std::is_same_v<T, int32_t>  ||
                          std::is_same_v<T, uint8_t>  || std::is_same_v<T, uint16_t> ||
                          std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t> ||
                          std::is_same_v<T, double>   || std::is_same_v<T, bool>     ||
                          std::is_same_v<T, std::string>) {
                return arg_a == std::get<T>(b);
            } else if constexpr (std::is_same_v<T, std::shared_ptr<RhoNull>>) {
                return true;  // null == null
            }
            return false;
        }, a);
    }

    /**
     * @brief Check if a value is "truthy"
     */
    bool isTruthy(const RhoValue& value) const {
        return std::visit([](const auto& arg) -> bool {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int64_t>) {
                return arg != 0;
            } else if constexpr (std::is_same_v<T, double>) {
                return arg != 0.0;
            } else if constexpr (std::is_same_v<T, bool>) {
                return arg;
            } else {
                return true;  // vectors/matrices/strings are always truthy
            }
        }, value);
    }

    /**
     * @brief Apply negation operator
     */
    RhoValue applyNegation(const RhoValue& value, SourceLocation loc) {
        return std::visit([&loc](const auto& arg) -> RhoValue {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int64_t>) {
                return -arg;
            } else if constexpr (std::is_same_v<T, double>) {
                return -arg;
            } else if constexpr (std::is_same_v<T, Eigen::VectorXd>) {
                return Eigen::VectorXd(-arg);
            } else if constexpr (std::is_same_v<T, Eigen::MatrixXd>) {
                return Eigen::MatrixXd(-arg);
            } else {
                throw TypeError::incompatibleUnaryOp("-", getValueType(arg));
            }
        }, value);
    }

    /**
     * @brief Apply bitwise NOT operator (~)
     */
    RhoValue applyBitNot(const RhoValue& value, SourceLocation loc) {
        return std::visit([&loc](const auto& arg) -> RhoValue {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int64_t>)  return ~arg;
            if constexpr (std::is_same_v<T, int8_t>)   return static_cast<int8_t>(~arg);
            if constexpr (std::is_same_v<T, int16_t>)  return static_cast<int16_t>(~arg);
            if constexpr (std::is_same_v<T, int32_t>)  return static_cast<int32_t>(~arg);
            if constexpr (std::is_same_v<T, uint8_t>)  return static_cast<uint8_t>(~arg);
            if constexpr (std::is_same_v<T, uint16_t>) return static_cast<uint16_t>(~arg);
            if constexpr (std::is_same_v<T, uint32_t>) return static_cast<uint32_t>(~arg);
            if constexpr (std::is_same_v<T, uint64_t>) return ~arg;
            if constexpr (std::is_same_v<T, bool>)     return !arg;
            throw TypeError::incompatibleUnaryOp("~", getValueType(arg));
        }, value);
    }

    /**
     * @brief Apply bitwise binary operator
     */
    RhoValue applyBitwise(BinaryOp op, const RhoValue& left, const RhoValue& right, SourceLocation loc) {
        auto toIntVal = [](const RhoValue& v) -> int64_t {
            return std::visit([](const auto& arg) -> int64_t {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, bool>) return arg ? 1 : 0;
                if constexpr (std::is_integral_v<T>)   return static_cast<int64_t>(arg);
                return 0;
            }, v);
        };

        RhoType lt = getValueType(left);
        RhoType rt = getValueType(right);

        auto isIntegral = [](RhoType t) {
            return t == RhoType::Int   || t == RhoType::Int8  || t == RhoType::Int16  ||
                   t == RhoType::Int32 || t == RhoType::UInt8 || t == RhoType::UInt16 ||
                   t == RhoType::UInt32 || t == RhoType::UInt64 || t == RhoType::Byte ||
                   t == RhoType::Bool;
        };

        if (!isIntegral(lt) || !isIntegral(rt)) {
            throw TypeError::incompatibleBinaryOp(binaryOpToString(op), lt, rt);
        }

        int64_t l = toIntVal(left);
        int64_t r = toIntVal(right);
        int64_t result;

        switch (op) {
            case BinaryOp::BitAnd: result = l & r; break;
            case BinaryOp::BitOr:  result = l | r; break;
            case BinaryOp::BitXor: result = l ^ r; break;
            case BinaryOp::Shl:
                if (r < 0 || r >= 64) throw RuntimeError("Shift amount out of range", loc);
                result = l << r; break;
            case BinaryOp::Shr:
                if (r < 0 || r >= 64) throw RuntimeError("Shift amount out of range", loc);
                result = l >> r; break;
            default: result = 0;
        }

        // Preserve original type if both sides are the same type
        if (lt == rt) {
            return std::visit([&](const auto& arg) -> RhoValue {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, bool>)     return result != 0;
                if constexpr (std::is_same_v<T, int8_t>)   return static_cast<int8_t>(result);
                if constexpr (std::is_same_v<T, int16_t>)  return static_cast<int16_t>(result);
                if constexpr (std::is_same_v<T, int32_t>)  return static_cast<int32_t>(result);
                if constexpr (std::is_same_v<T, uint8_t>)  return static_cast<uint8_t>(result);
                if constexpr (std::is_same_v<T, uint16_t>) return static_cast<uint16_t>(result);
                if constexpr (std::is_same_v<T, uint32_t>) return static_cast<uint32_t>(result);
                if constexpr (std::is_same_v<T, uint64_t>) return static_cast<uint64_t>(result);
                return result; // int64_t
            }, left);
        }
        return result; // default: int64_t
    }

    /**
     * @brief Apply binary operation
     */
    RhoValue applyBinaryOp(BinaryOp op, const RhoValue& left, const RhoValue& right,
                           SourceLocation loc) {
        RhoType leftType = getValueType(left);
        RhoType rightType = getValueType(right);

        // Comparison operators
        if (op >= BinaryOp::Eq && op <= BinaryOp::Ge) {
            return applyComparison(op, left, right, loc);
        }

        // Arithmetic operators
        switch (op) {
            case BinaryOp::Add:
                return applyAdd(left, right, loc);
            case BinaryOp::Sub:
                return applySub(left, right, loc);
            case BinaryOp::Mul:
                return applyMul(left, right, loc);
            case BinaryOp::Div:
                return applyDiv(left, right, loc);
            case BinaryOp::Mod:
                return applyMod(left, right, loc);
            case BinaryOp::And:
                return isTruthy(left) && isTruthy(right);
            case BinaryOp::Or:
                return isTruthy(left) || isTruthy(right);
            case BinaryOp::BitAnd:
                return applyBitwise(op, left, right, loc);
            case BinaryOp::BitOr:
                return applyBitwise(op, left, right, loc);
            case BinaryOp::BitXor:
                return applyBitwise(op, left, right, loc);
            case BinaryOp::Shl:
                return applyBitwise(op, left, right, loc);
            case BinaryOp::Shr:
                return applyBitwise(op, left, right, loc);
            default:
                throw TypeError::incompatibleBinaryOp(binaryOpToString(op), leftType, rightType);
        }
    }

    /**
     * @brief Apply indexing operation
     */
    RhoValue applyIndexing(const RhoValue& target, const std::vector<size_t>& indices, SourceLocation loc) {
        return std::visit([&](const auto& arg) -> RhoValue {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, Eigen::VectorXd>) {
                if (indices.size() != 1) {
                    throw RuntimeError("Vector indexing requires 1 index", loc);
                }
                size_t idx = indices[0];
                if (idx >= static_cast<size_t>(arg.size())) {
                    throw RuntimeError::indexOutOfBounds(idx, arg.size(), loc);
                }
                return arg(idx);
            } else if constexpr (std::is_same_v<T, Eigen::MatrixXd>) {
                if (indices.size() != 2) {
                    throw RuntimeError("Matrix indexing requires 2 indices", loc);
                }
                size_t row = indices[0], col = indices[1];
                if (row >= static_cast<size_t>(arg.rows()) || col >= static_cast<size_t>(arg.cols())) {
                    throw RuntimeError::invalidMatrixIndex(row, col, arg.rows(), arg.cols(), loc);
                }
                return arg(row, col);
            } else {
                throw RuntimeError("Indexing only supported for vectors and matrices", loc);
            }
        }, target);
    }

    /**
     * @brief Apply indexed assignment
     */
    RhoValue applyIndexedAssignment(const RhoValue& target, const std::vector<size_t>& indices,
                                   const RhoValue& newValue, SourceLocation loc) {
        return std::visit([&](const auto& arg) -> RhoValue {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, Eigen::VectorXd>) {
                if (indices.size() != 1) {
                    throw RuntimeError("Vector indexing requires 1 index", loc);
                }
                size_t idx = indices[0];
                if (idx >= static_cast<size_t>(arg.size())) {
                    throw RuntimeError::indexOutOfBounds(idx, arg.size(), loc);
                }
                Eigen::VectorXd result = arg;
                result(idx) = toDouble(newValue);
                return result;
            } else if constexpr (std::is_same_v<T, Eigen::MatrixXd>) {
                if (indices.size() != 2) {
                    throw RuntimeError("Matrix indexing requires 2 indices", loc);
                }
                size_t row = indices[0], col = indices[1];
                if (row >= static_cast<size_t>(arg.rows()) || col >= static_cast<size_t>(arg.cols())) {
                    throw RuntimeError::invalidMatrixIndex(row, col, arg.rows(), arg.cols(), loc);
                }
                Eigen::MatrixXd result = arg;
                result(row, col) = toDouble(newValue);
                return result;
            } else {
                throw RuntimeError("Indexed assignment only supported for vectors and matrices", loc);
            }
        }, target);
    }

    /**
     * @brief Normalize a slice index (handle negative indices Python-style)
     * @param idx The index (can be negative)
     * @param size The size of the dimension
     * @return Normalized non-negative index
     */
    size_t normalizeSliceIndex(int64_t idx, size_t size, SourceLocation loc) {
        if (idx < 0) {
            // Negative index: count from end
            int64_t normalized = static_cast<int64_t>(size) + idx;
            if (normalized < 0) {
                throw RuntimeError("Negative slice index " + std::to_string(idx) +
                                 " out of bounds for size " + std::to_string(size), loc);
            }
            return static_cast<size_t>(normalized);
        } else {
            return static_cast<size_t>(idx);
        }
    }

    /**
     * @brief Apply slicing operation with support for negative indices
     */
    RhoValue applySlicing(const RhoValue& target, const std::vector<EvaluatedSlice>& slices, SourceLocation loc) {
        return std::visit([&](const auto& arg) -> RhoValue {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, Eigen::VectorXd>) {
                if (slices.size() != 1) {
                    throw RuntimeError("Vector slicing requires 1 slice specification", loc);
                }

                const auto& slice = slices[0];
                size_t vecSize = static_cast<size_t>(arg.size());

                // Determine actual start and end indices (handle negatives)
                size_t start = slice.start.has_value()
                    ? normalizeSliceIndex(slice.start.value(), vecSize, loc)
                    : 0;
                size_t end = slice.end.has_value()
                    ? normalizeSliceIndex(slice.end.value(), vecSize, loc)
                    : vecSize;

                // Validate bounds
                if (start >= vecSize) {
                    throw RuntimeError("Slice start index " + std::to_string(start) +
                                     " out of bounds for vector of size " + std::to_string(vecSize), loc);
                }
                if (end > vecSize) {
                    throw RuntimeError("Slice end index " + std::to_string(end) +
                                     " out of bounds for vector of size " + std::to_string(vecSize), loc);
                }
                if (start >= end) {
                    throw RuntimeError("Slice start index must be less than end index", loc);
                }

                // Extract slice using Eigen's segment
                size_t sliceSize = end - start;
                return Eigen::VectorXd(arg.segment(start, sliceSize));

            } else if constexpr (std::is_same_v<T, Eigen::MatrixXd>) {
                if (slices.size() != 2) {
                    throw RuntimeError("Matrix slicing requires 2 slice specifications (row, col)", loc);
                }

                const auto& rowSlice = slices[0];
                const auto& colSlice = slices[1];

                size_t numRows = static_cast<size_t>(arg.rows());
                size_t numCols = static_cast<size_t>(arg.cols());

                // Determine actual start and end indices for rows (handle negatives)
                size_t rowStart = rowSlice.start.has_value()
                    ? normalizeSliceIndex(rowSlice.start.value(), numRows, loc)
                    : 0;
                size_t rowEnd = rowSlice.end.has_value()
                    ? normalizeSliceIndex(rowSlice.end.value(), numRows, loc)
                    : numRows;

                // Determine actual start and end indices for columns (handle negatives)
                size_t colStart = colSlice.start.has_value()
                    ? normalizeSliceIndex(colSlice.start.value(), numCols, loc)
                    : 0;
                size_t colEnd = colSlice.end.has_value()
                    ? normalizeSliceIndex(colSlice.end.value(), numCols, loc)
                    : numCols;

                // Validate bounds
                if (rowStart >= numRows) {
                    throw RuntimeError("Row slice start index " + std::to_string(rowStart) +
                                     " out of bounds for matrix with " + std::to_string(numRows) + " rows", loc);
                }
                if (rowEnd > numRows) {
                    throw RuntimeError("Row slice end index " + std::to_string(rowEnd) +
                                     " out of bounds for matrix with " + std::to_string(numRows) + " rows", loc);
                }
                if (colStart >= numCols) {
                    throw RuntimeError("Column slice start index " + std::to_string(colStart) +
                                     " out of bounds for matrix with " + std::to_string(numCols) + " columns", loc);
                }
                if (colEnd > numCols) {
                    throw RuntimeError("Column slice end index " + std::to_string(colEnd) +
                                     " out of bounds for matrix with " + std::to_string(numCols) + " columns", loc);
                }
                if (rowStart >= rowEnd) {
                    throw RuntimeError("Row slice start index must be less than end index", loc);
                }
                if (colStart >= colEnd) {
                    throw RuntimeError("Column slice start index must be less than end index", loc);
                }

                // Extract slice using Eigen's block
                size_t rowCount = rowEnd - rowStart;
                size_t colCount = colEnd - colStart;
                return Eigen::MatrixXd(arg.block(rowStart, colStart, rowCount, colCount));

            } else {
                throw RuntimeError("Slicing only supported for vectors and matrices", loc);
            }
        }, target);
    }

    // ========================================================================
    // Arithmetic Operation Implementations
    // ========================================================================

    RhoValue applyAdd(const RhoValue& left, const RhoValue& right, SourceLocation loc) {
        return std::visit([&loc](const auto& l, const auto& r) -> RhoValue {
            using L = std::decay_t<decltype(l)>;
            using R = std::decay_t<decltype(r)>;

            // Scalar + Scalar
            if constexpr (std::is_same_v<L, int64_t> && std::is_same_v<R, int64_t>) {
                return l + r;
            }
            else if constexpr ((std::is_same_v<L, double> || std::is_same_v<L, int64_t>) &&
                              (std::is_same_v<R, double> || std::is_same_v<R, int64_t>)) {
                return static_cast<double>(l) + static_cast<double>(r);
            }
            // Vector + Vector
            else if constexpr (std::is_same_v<L, Eigen::VectorXd> && std::is_same_v<R, Eigen::VectorXd>) {
                if (l.size() != r.size()) {
                    throw RuntimeError::dimensionMismatch("+",
                        std::to_string(l.size()), std::to_string(r.size()), loc);
                }
                return Eigen::VectorXd(l + r);
            }
            // Matrix + Matrix
            else if constexpr (std::is_same_v<L, Eigen::MatrixXd> && std::is_same_v<R, Eigen::MatrixXd>) {
                if (l.rows() != r.rows() || l.cols() != r.cols()) {
                    throw RuntimeError::dimensionMismatch("+",
                        std::to_string(l.rows()) + "x" + std::to_string(l.cols()),
                        std::to_string(r.rows()) + "x" + std::to_string(r.cols()), loc);
                }
                return Eigen::MatrixXd(l + r);
            }
            else {
                throw TypeError::incompatibleBinaryOp("+",
                    getValueType(RhoValue(l)), getValueType(RhoValue(r)));
            }
        }, left, right);
    }

    RhoValue applySub(const RhoValue& left, const RhoValue& right, SourceLocation loc) {
        return std::visit([&loc](const auto& l, const auto& r) -> RhoValue {
            using L = std::decay_t<decltype(l)>;
            using R = std::decay_t<decltype(r)>;

            if constexpr (std::is_same_v<L, int64_t> && std::is_same_v<R, int64_t>) {
                return l - r;
            }
            else if constexpr ((std::is_same_v<L, double> || std::is_same_v<L, int64_t>) &&
                              (std::is_same_v<R, double> || std::is_same_v<R, int64_t>)) {
                return static_cast<double>(l) - static_cast<double>(r);
            }
            else if constexpr (std::is_same_v<L, Eigen::VectorXd> && std::is_same_v<R, Eigen::VectorXd>) {
                if (l.size() != r.size()) {
                    throw RuntimeError::dimensionMismatch("-",
                        std::to_string(l.size()), std::to_string(r.size()), loc);
                }
                return Eigen::VectorXd(l - r);
            }
            else if constexpr (std::is_same_v<L, Eigen::MatrixXd> && std::is_same_v<R, Eigen::MatrixXd>) {
                if (l.rows() != r.rows() || l.cols() != r.cols()) {
                    throw RuntimeError::dimensionMismatch("-",
                        std::to_string(l.rows()) + "x" + std::to_string(l.cols()),
                        std::to_string(r.rows()) + "x" + std::to_string(r.cols()), loc);
                }
                return Eigen::MatrixXd(l - r);
            }
            else {
                throw TypeError::incompatibleBinaryOp("-",
                    getValueType(RhoValue(l)), getValueType(RhoValue(r)));
            }
        }, left, right);
    }

    RhoValue applyMul(const RhoValue& left, const RhoValue& right, SourceLocation loc) {
        return std::visit([&loc](const auto& l, const auto& r) -> RhoValue {
            using L = std::decay_t<decltype(l)>;
            using R = std::decay_t<decltype(r)>;

            // Scalar * Scalar
            if constexpr (std::is_same_v<L, int64_t> && std::is_same_v<R, int64_t>) {
                return l * r;
            }
            else if constexpr ((std::is_same_v<L, double> || std::is_same_v<L, int64_t>) &&
                              (std::is_same_v<R, double> || std::is_same_v<R, int64_t>)) {
                return static_cast<double>(l) * static_cast<double>(r);
            }
            // Scalar * Vector
            else if constexpr ((std::is_same_v<L, double> || std::is_same_v<L, int64_t>) &&
                              std::is_same_v<R, Eigen::VectorXd>) {
                return Eigen::VectorXd(static_cast<double>(l) * r);
            }
            else if constexpr (std::is_same_v<L, Eigen::VectorXd> &&
                              (std::is_same_v<R, double> || std::is_same_v<R, int64_t>)) {
                return Eigen::VectorXd(l * static_cast<double>(r));
            }
            // Scalar * Matrix
            else if constexpr ((std::is_same_v<L, double> || std::is_same_v<L, int64_t>) &&
                              std::is_same_v<R, Eigen::MatrixXd>) {
                return Eigen::MatrixXd(static_cast<double>(l) * r);
            }
            else if constexpr (std::is_same_v<L, Eigen::MatrixXd> &&
                              (std::is_same_v<R, double> || std::is_same_v<R, int64_t>)) {
                return Eigen::MatrixXd(l * static_cast<double>(r));
            }
            // Matrix * Matrix
            else if constexpr (std::is_same_v<L, Eigen::MatrixXd> && std::is_same_v<R, Eigen::MatrixXd>) {
                if (l.cols() != r.rows()) {
                    throw RuntimeError::dimensionMismatch("*",
                        std::to_string(l.rows()) + "x" + std::to_string(l.cols()),
                        std::to_string(r.rows()) + "x" + std::to_string(r.cols()), loc);
                }
                return Eigen::MatrixXd(l * r);
            }
            // Matrix * Vector
            else if constexpr (std::is_same_v<L, Eigen::MatrixXd> && std::is_same_v<R, Eigen::VectorXd>) {
                if (l.cols() != r.size()) {
                    throw RuntimeError::dimensionMismatch("*",
                        std::to_string(l.rows()) + "x" + std::to_string(l.cols()),
                        std::to_string(r.size()), loc);
                }
                return Eigen::VectorXd(l * r);
            }
            else {
                throw TypeError::incompatibleBinaryOp("*",
                    getValueType(RhoValue(l)), getValueType(RhoValue(r)));
            }
        }, left, right);
    }

    RhoValue applyDiv(const RhoValue& left, const RhoValue& right, SourceLocation loc) {
        return std::visit([&loc](const auto& l, const auto& r) -> RhoValue {
            using L = std::decay_t<decltype(l)>;
            using R = std::decay_t<decltype(r)>;

            // Scalar / Scalar
            if constexpr ((std::is_same_v<L, double> || std::is_same_v<L, int64_t>) &&
                         (std::is_same_v<R, double> || std::is_same_v<R, int64_t>)) {
                double divisor = static_cast<double>(r);
                if (divisor == 0.0) {
                    throw RuntimeError::divisionByZero(loc);
                }
                return static_cast<double>(l) / divisor;
            }
            // Vector / Scalar
            else if constexpr (std::is_same_v<L, Eigen::VectorXd> &&
                              (std::is_same_v<R, double> || std::is_same_v<R, int64_t>)) {
                double divisor = static_cast<double>(r);
                if (divisor == 0.0) {
                    throw RuntimeError::divisionByZero(loc);
                }
                return Eigen::VectorXd(l / divisor);
            }
            // Matrix / Scalar
            else if constexpr (std::is_same_v<L, Eigen::MatrixXd> &&
                              (std::is_same_v<R, double> || std::is_same_v<R, int64_t>)) {
                double divisor = static_cast<double>(r);
                if (divisor == 0.0) {
                    throw RuntimeError::divisionByZero(loc);
                }
                return Eigen::MatrixXd(l / divisor);
            }
            else {
                throw TypeError::incompatibleBinaryOp("/",
                    getValueType(RhoValue(l)), getValueType(RhoValue(r)));
            }
        }, left, right);
    }

    RhoValue applyMod(const RhoValue& left, const RhoValue& right, SourceLocation loc) {
        if (!isScalar(left) || !isScalar(right)) {
            throw TypeError::incompatibleBinaryOp("%", getValueType(left), getValueType(right));
        }

        int64_t l = toInt(left);
        int64_t r = toInt(right);

        if (r == 0) {
            throw RuntimeError::divisionByZero(loc);
        }

        return l % r;
    }

    RhoValue applyComparison(BinaryOp op, const RhoValue& left, const RhoValue& right,
                             SourceLocation loc) {
        if (!isScalar(left) || !isScalar(right)) {
            throw TypeError::incompatibleBinaryOp(binaryOpToString(op),
                getValueType(left), getValueType(right));
        }

        // Handle bool comparisons specially (only == and != make sense)
        if (std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right)) {
            bool l = std::get<bool>(left);
            bool r = std::get<bool>(right);

            switch (op) {
                case BinaryOp::Eq: return l == r;
                case BinaryOp::Ne: return l != r;
                default:
                    throw TypeError::incompatibleBinaryOp(binaryOpToString(op),
                        RhoType::Bool, RhoType::Bool);
            }
        }

        // For numeric types, convert to double and compare
        double l = toDouble(left);
        double r = toDouble(right);

        switch (op) {
            case BinaryOp::Eq: return l == r;
            case BinaryOp::Ne: return l != r;
            case BinaryOp::Lt: return l < r;
            case BinaryOp::Gt: return l > r;
            case BinaryOp::Le: return l <= r;
            case BinaryOp::Ge: return l >= r;
            default: return false;
        }
    }

    /**
     * @brief Close a resource (file handle) by calling io.close()
     */
    void closeResource(const RhoValue& resource, SourceLocation loc) {
        // Resource should be an int (file handle)
        if (!std::holds_alternative<int64_t>(resource)) {
            // If it's not an int, we can't close it via io.close()
            return;
        }

        int64_t handle = std::get<int64_t>(resource);

        // Call io.close(handle)
        std::vector<RhoValue> args = {handle};
        try {
            Builtins::instance().callModule("io", "close", args, loc);
        } catch (...) {
            // Ignore errors during cleanup
            // We don't want to mask the original exception
        }
    }

    /**
     * @brief Call a lambda/function value
     * @param func Function object to call
     * @param args Arguments to pass
     * @param loc Source location for error reporting
     * @return Result value
     */
    RhoValue callLambda(std::shared_ptr<RhoFunction> func,
                       const std::vector<RhoValue>& args,
                       SourceLocation loc) {
        // Check if it's a native function
        if (func->isNative()) {
            return func->callNative(args);
        }

        // Validate argument count
        if (args.size() != func->arity()) {
            throw ArgumentError("<lambda>",
                "expected " + std::to_string(func->arity()) +
                " arguments, got " + std::to_string(args.size()), loc);
        }

        // Create new scope for function execution
        ScopeGuard funcGuard(symbols_);

        // Restore closure environment (non-function values only)
        // Functions will be resolved via the existing symbol table
        for (const auto& [name, value] : func->closure()) {
            symbols_.declare(name, getValueType(value), value, loc);
        }

        // Bind parameters
        const auto& params = func->params();
        for (size_t i = 0; i < args.size(); ++i) {
            symbols_.declare(params[i], getValueType(args[i]), args[i], loc);
        }

        // Get the body and execute it
        void* bodyPtr = func->body().get();

        if (func->isExpression()) {
            // Expression lambda: evaluate expression and return result
            ExprNode* exprNode = static_cast<ExprNode*>(bodyPtr);
            exprNode->accept(*this);
            return result_;
        } else {
            // Block lambda: execute block, handle return
            BlockNode* blockNode = static_cast<BlockNode*>(bodyPtr);
            try {
                blockNode->accept(*this);
                // If no return statement, return void (0)
                return int64_t(0);
            } catch (const ReturnValue& ret) {
                return ret.value_;
            }
        }
    }
};

} // namespace Rhodesia

#endif // RHODESIA_EVALUATOR_HPP
