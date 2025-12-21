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
        result_ = node.value ? int64_t(1) : int64_t(0);
    }

    void visit(StringLiteralNode& node) override {
        result_ = node.value;
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
            result_ = isTruthy(operand) ? int64_t(0) : int64_t(1);
        }
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

        // Evaluate slice specifications
        std::vector<EvaluatedSlice> evalSlices;
        for (auto& slice : node.slices) {
            EvaluatedSlice evalSlice;

            if (slice.start.has_value()) {
                slice.start.value()->accept(*this);
                evalSlice.start = static_cast<size_t>(toInt(result_));
            }

            if (slice.end.has_value()) {
                slice.end.value()->accept(*this);
                evalSlice.end = static_cast<size_t>(toInt(result_));
            }

            evalSlices.push_back(evalSlice);
        }

        result_ = applySlicing(target, evalSlices, node.location);
    }

    // ========================================================================
    // Statement Visitors
    // ========================================================================

    void visit(VarDeclNode& node) override {
        node.initializer->accept(*this);
        // Type check: ensure initializer type is compatible with declared type
        RhoType initType = getValueType(result_);
        if (initType != node.type) {
            throw TypeError::expectedType(
                "Variable initialization",
                node.type, initType);
        }
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

    void visit(IncludeNode& node) override {
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

    // ========================================================================
    // Helper Types
    // ========================================================================

    /**
     * @brief Evaluated slice specification with concrete indices
     */
    struct EvaluatedSlice {
        std::optional<size_t> start;
        std::optional<size_t> end;
    };

    // ========================================================================
    // Helper Functions
    // ========================================================================

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
            } else {
                return true;  // vectors/matrices are always truthy
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
                return (isTruthy(left) && isTruthy(right)) ? int64_t(1) : int64_t(0);
            case BinaryOp::Or:
                return (isTruthy(left) || isTruthy(right)) ? int64_t(1) : int64_t(0);
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
     * @brief Apply slicing operation
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

                // Determine actual start and end indices
                size_t start = slice.start.value_or(0);
                size_t end = slice.end.value_or(vecSize);

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

                // Determine actual start and end indices for rows
                size_t rowStart = rowSlice.start.value_or(0);
                size_t rowEnd = rowSlice.end.value_or(numRows);

                // Determine actual start and end indices for columns
                size_t colStart = colSlice.start.value_or(0);
                size_t colEnd = colSlice.end.value_or(numCols);

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

        double l = toDouble(left);
        double r = toDouble(right);

        bool result;
        switch (op) {
            case BinaryOp::Eq: result = (l == r); break;
            case BinaryOp::Ne: result = (l != r); break;
            case BinaryOp::Lt: result = (l < r); break;
            case BinaryOp::Gt: result = (l > r); break;
            case BinaryOp::Le: result = (l <= r); break;
            case BinaryOp::Ge: result = (l >= r); break;
            default: result = false;
        }

        return result ? int64_t(1) : int64_t(0);
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
};

} // namespace Rhodesia

#endif // RHODESIA_EVALUATOR_HPP
