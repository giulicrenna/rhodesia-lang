/**
 * @file Compiler.hpp
 * @brief Bytecode compiler for the Rhodesia language.
 *
 * The Compiler walks an AST (via ASTVisitor) and emits bytecode into
 * Chunk objects.  Top-level code uses LOAD_GLOBAL / STORE_GLOBAL; function
 * bodies use LOAD_LOCAL / STORE_LOCAL; closures capture upvalues.
 */

#pragma once
#ifndef RHODESIA_COMPILER_HPP
#define RHODESIA_COMPILER_HPP

#include "AST.hpp"
#include "Bytecode.hpp"
#include "Builtins.hpp"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace Rhodesia {

// ---------------------------------------------------------------------------
// CompilerError
// ---------------------------------------------------------------------------

class CompilerError : public std::runtime_error {
public:
    explicit CompilerError(const std::string& msg)
        : std::runtime_error("CompilerError: " + msg) {}
};

// ---------------------------------------------------------------------------
// Compiler
// ---------------------------------------------------------------------------

class Compiler : public ASTVisitor {
public:
    // -----------------------------------------------------------------------
    // Supporting types
    // -----------------------------------------------------------------------

    struct Local {
        std::string name;
        RhoType     type;
        int         depth; ///< Scope depth at which the local was declared
    };

    struct UpvalueInfo {
        bool        isLocal; ///< true  -> from enclosing function's local slot
                             ///< false -> from enclosing function's upvalue
        int         index;   ///< Slot / upvalue index in the enclosing function
        std::string name;
    };

    // -----------------------------------------------------------------------
    // Per-function compiler state
    // -----------------------------------------------------------------------

    struct CompilerState {
        std::shared_ptr<Chunk>      chunk;
        std::vector<Local>          locals;
        std::vector<UpvalueInfo>    upvalues;
        int                         scopeDepth  = 0;
        CompilerState*              enclosing   = nullptr;
        bool                        isTopLevel  = false;

        // Loop control
        std::vector<int>            breakJumps;
        std::vector<int>            continueJumps;
        int                         loopStart   = -1;

        explicit CompilerState(std::string chunkName, bool topLevel = false)
            : chunk(std::make_shared<Chunk>()), isTopLevel(topLevel)
        {
            chunk->name = std::move(chunkName);
        }
    };

    // -----------------------------------------------------------------------
    // Public API
    // -----------------------------------------------------------------------

    Compiler() = default;

    /**
     * @brief Compile a full program into the "main" chunk.
     * @return Shared pointer to the main Chunk (ready for the VM).
     */
    std::shared_ptr<Chunk> compile(ProgramNode& program) {
        pushState("<main>", /*isTopLevel=*/true);
        program.accept(*this);
        chunk().emit(Opcode::HALT);
        return stateStack_.back()->chunk;
    }

    // -----------------------------------------------------------------------
    // ASTVisitor – Expressions
    // -----------------------------------------------------------------------

    void visit(IntLiteralNode& node) override {
        int idx = chunk().addConstant(RhoValue{node.value});
        chunk().emit(Opcode::LOAD_CONST, idx);
    }

    void visit(FloatLiteralNode& node) override {
        int idx = chunk().addConstant(RhoValue{node.value});
        chunk().emit(Opcode::LOAD_CONST, idx);
    }

    void visit(BoolLiteralNode& node) override {
        chunk().emit(node.value ? Opcode::LOAD_TRUE : Opcode::LOAD_FALSE);
    }

    void visit(StringLiteralNode& node) override {
        int idx = chunk().addConstant(RhoValue{node.value});
        chunk().emit(Opcode::LOAD_CONST, idx);
    }

    void visit(NullLiteralNode& /*node*/) override {
        chunk().emit(Opcode::LOAD_NULL);
    }

    void visit(VectorLiteralNode& node) override {
        for (auto& elem : node.elements) {
            elem->accept(*this);
        }
        chunk().emit(Opcode::BUILD_VEC, static_cast<int32_t>(node.elements.size()));
    }

    void visit(MatrixLiteralNode& node) override {
        int rows = static_cast<int>(node.rows.size());
        int cols = rows > 0 ? static_cast<int>(node.rows[0].size()) : 0;
        for (auto& row : node.rows) {
            for (auto& elem : row) {
                elem->accept(*this);
            }
        }
        int32_t operand = static_cast<int32_t>((rows << 16) | cols);
        chunk().emit(Opcode::BUILD_MAT, operand);
    }

    void visit(SetLiteralNode& node) override {
        for (auto& elem : node.elements) {
            elem->accept(*this);
        }
        chunk().emit(Opcode::BUILD_SET, static_cast<int32_t>(node.elements.size()));
    }

    void visit(TupleLiteralNode& node) override {
        for (auto& elem : node.elements) {
            elem->accept(*this);
        }
        chunk().emit(Opcode::BUILD_TUPLE, static_cast<int32_t>(node.elements.size()));
    }

    void visit(RecordLiteralNode& node) override {
        for (auto& [fieldName, fieldVal] : node.fields) {
            int nameIdx = chunk().addConstant(RhoValue{fieldName});
            chunk().emit(Opcode::LOAD_CONST, nameIdx);
            fieldVal->accept(*this);
        }
        chunk().emit(Opcode::BUILD_RECORD, static_cast<int32_t>(node.fields.size()));
    }

    void visit(IdentifierNode& node) override {
        // 1. Try local
        int localIdx = resolveLocal(current_, node.name);
        if (localIdx != -1) {
            chunk().emit(Opcode::LOAD_LOCAL, localIdx);
            return;
        }
        // 2. Try upvalue
        int upvalIdx = resolveUpvalue(current_, node.name);
        if (upvalIdx != -1) {
            chunk().emit(Opcode::LOAD_UPVALUE, upvalIdx);
            return;
        }
        // 3. Global
        int nameIdx = chunk().addName(node.name);
        chunk().emit(Opcode::LOAD_GLOBAL, nameIdx);
    }

    void visit(BinaryOpNode& node) override {
        if (node.op == BinaryOp::And) {
            // Short-circuit AND
            node.left->accept(*this);
            int jumpFalse = chunk().emitJump(Opcode::JUMP_IF_FALSE_PEEK);
            chunk().emit(Opcode::POP);
            node.right->accept(*this);
            chunk().patchJump(jumpFalse);
            return;
        }
        if (node.op == BinaryOp::Or) {
            // Short-circuit OR
            node.left->accept(*this);
            int jumpTrue = chunk().emitJump(Opcode::JUMP_IF_TRUE_PEEK);
            chunk().emit(Opcode::POP);
            node.right->accept(*this);
            chunk().patchJump(jumpTrue);
            return;
        }

        node.left->accept(*this);
        node.right->accept(*this);

        switch (node.op) {
            case BinaryOp::Add: chunk().emit(Opcode::ADD); break;
            case BinaryOp::Sub: chunk().emit(Opcode::SUB); break;
            case BinaryOp::Mul: chunk().emit(Opcode::MUL); break;
            case BinaryOp::Div: chunk().emit(Opcode::DIV); break;
            case BinaryOp::Mod: chunk().emit(Opcode::MOD); break;
            case BinaryOp::Eq:  chunk().emit(Opcode::EQ);  break;
            case BinaryOp::Ne:  chunk().emit(Opcode::NE);  break;
            case BinaryOp::Lt:  chunk().emit(Opcode::LT);  break;
            case BinaryOp::Gt:  chunk().emit(Opcode::GT);  break;
            case BinaryOp::Le:  chunk().emit(Opcode::LE);  break;
            case BinaryOp::Ge:  chunk().emit(Opcode::GE);  break;
            default:
                throw CompilerError("Unexpected binary operator");
        }
    }

    void visit(UnaryOpNode& node) override {
        node.operand->accept(*this);
        switch (node.op) {
            case UnaryOp::Neg: chunk().emit(Opcode::NEGATE); break;
            case UnaryOp::Not: chunk().emit(Opcode::NOT);    break;
        }
    }

    void visit(TernaryOpNode& node) override {
        node.condition->accept(*this);
        int jumpFalse = chunk().emitJump(Opcode::JUMP_IF_FALSE);
        node.trueExpr->accept(*this);
        int jumpEnd = chunk().emitJump(Opcode::JUMP);
        chunk().patchJump(jumpFalse);
        node.falseExpr->accept(*this);
        chunk().patchJump(jumpEnd);
    }

    void visit(FunctionCallNode& node) override {
        if (Builtins::instance().isBuiltin(node.name)) {
            for (auto& arg : node.arguments) {
                arg->accept(*this);
            }
            int nameIdx = chunk().addName(node.name);
            int argc    = static_cast<int>(node.arguments.size());
            // Encoding: (name_idx << 8) | argc  — matches Bytecode.hpp comment
            int32_t operand = static_cast<int32_t>((nameIdx << 8) | argc);
            chunk().emit(Opcode::CALL_BUILTIN, operand);
        } else {
            // Load the callee
            int nameIdx = resolveLocal(current_, node.name);
            if (nameIdx != -1) {
                chunk().emit(Opcode::LOAD_LOCAL, nameIdx);
            } else {
                int upvalIdx = resolveUpvalue(current_, node.name);
                if (upvalIdx != -1) {
                    chunk().emit(Opcode::LOAD_UPVALUE, upvalIdx);
                } else {
                    int gIdx = chunk().addName(node.name);
                    chunk().emit(Opcode::LOAD_GLOBAL, gIdx);
                }
            }
            for (auto& arg : node.arguments) {
                arg->accept(*this);
            }
            chunk().emit(Opcode::CALL, static_cast<int32_t>(node.arguments.size()));
        }
    }

    void visit(MemberAccessNode& node) override {
        std::string fullName = node.object + "." + node.member;

        if (node.arguments.empty()) {
            // If the object is a known built-in module, emit LOAD_MODULE_CONST.
            // Otherwise, treat it as a record field access: load the variable
            // and emit MEMBER_GET.
            if (Builtins::instance().isModule(node.object)) {
                int nameIdx = chunk().addName(fullName);
                chunk().emit(Opcode::LOAD_MODULE_CONST, nameIdx);
            } else {
                // Record field access: load the record variable, then get field
                int localIdx = resolveLocal(current_, node.object);
                if (localIdx != -1) {
                    chunk().emit(Opcode::LOAD_LOCAL, localIdx);
                } else {
                    int upvalIdx = resolveUpvalue(current_, node.object);
                    if (upvalIdx != -1) {
                        chunk().emit(Opcode::LOAD_UPVALUE, upvalIdx);
                    } else {
                        int nameIdx = chunk().addName(node.object);
                        chunk().emit(Opcode::LOAD_GLOBAL, nameIdx);
                    }
                }
                int fieldIdx = chunk().addName(node.member);
                chunk().emit(Opcode::MEMBER_GET, fieldIdx);
            }
        } else {
            // Module function call: object.member(args...)
            for (auto& arg : node.arguments) {
                arg->accept(*this);
            }
            int nameIdx = chunk().addName(fullName);
            int argc    = static_cast<int>(node.arguments.size());
            // CALL_MODULE: operand = name_idx; argc follows as next instruction operand
            chunk().emit(Opcode::CALL_MODULE, nameIdx);
            chunk().emit(Opcode::NOP, argc); // argc encoded as operand of NOP sentinel
        }
    }

    void visit(ChainedMemberAccessNode& node) override {
        node.object->accept(*this);               // push record onto stack
        int nameIdx = chunk().addName(node.field);
        chunk().emit(Opcode::MEMBER_GET, nameIdx); // pop + push field value
    }

    void visit(IndexAccessNode& node) override {
        node.target->accept(*this);
        if (node.indices.size() == 1) {
            node.indices[0]->accept(*this);
            chunk().emit(Opcode::INDEX_GET);
        } else if (node.indices.size() == 2) {
            node.indices[0]->accept(*this);
            node.indices[1]->accept(*this);
            chunk().emit(Opcode::INDEX_GET_2D);
        } else {
            throw CompilerError("IndexAccessNode: unsupported index count " +
                                std::to_string(node.indices.size()));
        }
    }

    void visit(SliceNode& node) override {
        node.target->accept(*this);
        // Only the first slice spec is compiled here (vector slicing).
        // Matrix slicing (two specs) would require extended VM support.
        if (node.slices.empty()) {
            throw CompilerError("SliceNode: no slice specifications");
        }
        const SliceSpec& spec = node.slices[0];
        int32_t flags = 0;
        if (spec.start.has_value()) {
            (*spec.start)->accept(*this);
            flags |= 1;
        }
        if (spec.end.has_value()) {
            (*spec.end)->accept(*this);
            flags |= 2;
        }
        chunk().emit(Opcode::SLICE_GET, flags);
    }

    void visit(LambdaNode& node) override {
        // Create a new CompilerState for the lambda body
        pushState("<lambda>", /*isTopLevel=*/false);
        CompilerState* lambdaState = current_;
        lambdaState->chunk->arity = static_cast<int>(node.params.size());

        // Parameters become the first locals
        for (auto& param : node.params) {
            RhoType paramType = param.type.value_or(RhoType::Unknown);
            addLocal(param.name, paramType);
        }
        lambdaState->chunk->numLocals = static_cast<int>(lambdaState->locals.size());

        // Compile body
        if (node.isExpression) {
            node.body->accept(*this);
            chunk().emit(Opcode::RETURN);
        } else {
            // body is a BlockNode wrapped as ExprNode — accept it directly
            node.body->accept(*this);
            chunk().emit(Opcode::RETURN_NULL);
        }

        // Capture upvalue descriptors into the chunk
        for (auto& uv : lambdaState->upvalues) {
            lambdaState->chunk->upvalueDescs.push_back({uv.isLocal, uv.index});
        }
        // numLocals is the high-water mark from addLocal(); do NOT reset to
        // locals.size() which may be smaller after endScope() calls.

        std::shared_ptr<Chunk> innerChunk = lambdaState->chunk;
        popState();

        // Store the inner chunk as a RhoFunction constant in the outer chunk.
        // The "__compiled__" marker lets the VM detect this as bytecode.
        std::unordered_map<std::string, RhoValue> compiledMarker;
        compiledMarker["__compiled__"] = int64_t(1);
        compiledMarker["__upvalue_id__"] = int64_t(0); // placeholder; real ID assigned by MAKE_CLOSURE
        auto fn = std::make_shared<RhoFunction>(
            std::vector<std::string>{},  // params by name not needed in bytecode path
            std::static_pointer_cast<void>(innerChunk),
            node.isExpression,
            compiledMarker
        );
        int constIdx = chunk().addConstant(RhoValue{fn});
        chunk().emit(Opcode::MAKE_CLOSURE, constIdx);
    }

    // -----------------------------------------------------------------------
    // ASTVisitor – Statements
    // -----------------------------------------------------------------------

    void visit(VarDeclNode& node) override {
        // Compile initializer
        if (node.initializer) {
            node.initializer->accept(*this);
        } else {
            chunk().emit(Opcode::LOAD_NULL);
        }

        // Optional type coercion
        if (node.type != RhoType::Unknown && node.type != RhoType::Null) {
            chunk().emit(Opcode::COERCE, static_cast<int32_t>(node.type));
        }

        if (isGlobalScope()) {
            int nameIdx = chunk().addName(node.name);
            chunk().emit(Opcode::STORE_GLOBAL, nameIdx);
        } else {
            int localIdx = addLocal(node.name, node.type);
            chunk().emit(Opcode::STORE_LOCAL, localIdx);
        }
    }

    void visit(AssignmentNode& node) override {
        if (node.indices.empty()) {
            // Simple variable assignment
            node.value->accept(*this);
            emitStore(node.name);
        } else if (node.indices.size() == 1) {
            // v[i] = value
            emitLoad(node.name);
            node.indices[0]->accept(*this);
            node.value->accept(*this);
            chunk().emit(Opcode::INDEX_SET);
        } else if (node.indices.size() == 2) {
            // m[i, j] = value
            emitLoad(node.name);
            node.indices[0]->accept(*this);
            node.indices[1]->accept(*this);
            node.value->accept(*this);
            chunk().emit(Opcode::INDEX_SET_2D);
        } else {
            throw CompilerError("AssignmentNode: unsupported index count");
        }
    }

    void visit(TupleDestructureNode& node) override {
        throw std::runtime_error("Tuple destructuring not yet supported in VM mode");
    }

    void visit(ExprStmtNode& node) override {
        node.expression->accept(*this);
        chunk().emit(Opcode::POP);
    }

    void visit(ReturnNode& node) override {
        if (node.value) {
            node.value->accept(*this);
            chunk().emit(Opcode::RETURN);
        } else {
            chunk().emit(Opcode::RETURN_NULL);
        }
    }

    void visit(BlockNode& node) override {
        beginScope();
        for (auto& stmt : node.statements) {
            stmt->accept(*this);
        }
        endScope();
    }

    void visit(FunctionDeclNode& node) override {
        // Compile function body into a new chunk
        pushState(node.name, /*isTopLevel=*/false);
        CompilerState* fnState = current_;
        fnState->chunk->arity = static_cast<int>(node.params.size());

        // Parameters become the first locals in the function chunk
        for (auto& param : node.params) {
            addLocal(param.name, param.type);
        }
        fnState->chunk->numLocals = static_cast<int>(fnState->locals.size());

        // Compile body statements without an extra scope (params already in scope)
        for (auto& stmt : node.body->statements) {
            stmt->accept(*this);
        }
        // Implicit return null at end of function
        chunk().emit(Opcode::RETURN_NULL);

        // Capture upvalue descriptors
        for (auto& uv : fnState->upvalues) {
            fnState->chunk->upvalueDescs.push_back({uv.isLocal, uv.index});
        }
        // numLocals is already the high-water mark tracked by addLocal(); do NOT
        // overwrite it with the (shrunken) locals.size() after endScope() calls.

        // Store param types
        for (auto& param : node.params) {
            fnState->chunk->paramTypes.push_back(param.type);
        }

        std::shared_ptr<Chunk> innerChunk = fnState->chunk;
        popState();

        // Create the RhoFunction constant with "__compiled__" marker.
        std::unordered_map<std::string, RhoValue> compiledMarker;
        compiledMarker["__compiled__"] = int64_t(1);
        compiledMarker["__upvalue_id__"] = int64_t(0);
        auto fn = std::make_shared<RhoFunction>(
            std::vector<std::string>{},
            std::static_pointer_cast<void>(innerChunk),
            /*isExpr=*/false,
            compiledMarker
        );
        int constIdx = chunk().addConstant(RhoValue{fn});
        chunk().emit(Opcode::MAKE_CLOSURE, constIdx);

        // Bind the function to the global (functions are always globals)
        int nameIdx = chunk().addName(node.name);
        chunk().emit(Opcode::STORE_GLOBAL, nameIdx);
    }

    void visit(IfStmtNode& node) override {
        node.condition->accept(*this);

        int jumpFalse = chunk().emitJump(Opcode::JUMP_IF_FALSE);

        // Then branch
        for (auto& stmt : node.thenBranch->statements) {
            stmt->accept(*this);
        }

        if (node.elseBranch) {
            int jumpEnd = chunk().emitJump(Opcode::JUMP);
            chunk().patchJump(jumpFalse);
            for (auto& stmt : node.elseBranch->statements) {
                stmt->accept(*this);
            }
            chunk().patchJump(jumpEnd);
        } else {
            chunk().patchJump(jumpFalse);
        }
    }

    void visit(WhileLoopNode& node) override {
        // Save outer loop context
        int savedLoopStart             = current_->loopStart;
        std::vector<int> savedBreaks   = std::move(current_->breakJumps);
        std::vector<int> savedContinues = std::move(current_->continueJumps);
        current_->breakJumps.clear();
        current_->continueJumps.clear();

        int loopStart = chunk().currentPos();
        current_->loopStart = loopStart;

        node.condition->accept(*this);
        int jumpExit = chunk().emitJump(Opcode::JUMP_IF_FALSE);

        // Body
        beginScope();
        for (auto& stmt : node.body->statements) {
            stmt->accept(*this);
        }
        endScope();

        chunk().emit(Opcode::JUMP, loopStart);

        int exitPos = chunk().currentPos();
        chunk().patchJump(jumpExit);

        // Patch break jumps to exit
        for (int idx : current_->breakJumps) {
            chunk().patchJumpTo(idx, exitPos);
        }
        // Patch continue jumps to loop start
        for (int idx : current_->continueJumps) {
            chunk().patchJumpTo(idx, loopStart);
        }

        // Restore outer loop context
        current_->loopStart     = savedLoopStart;
        current_->breakJumps    = std::move(savedBreaks);
        current_->continueJumps = std::move(savedContinues);
    }

    void visit(ForLoopNode& node) override {
        // Save outer loop context
        int savedLoopStart              = current_->loopStart;
        std::vector<int> savedBreaks    = std::move(current_->breakJumps);
        std::vector<int> savedContinues = std::move(current_->continueJumps);
        current_->breakJumps.clear();
        current_->continueJumps.clear();

        // Compile iterable and create the iterator
        node.iterable->accept(*this);
        chunk().emit(Opcode::MAKE_ITER);

        // Loop header
        int loopStart = chunk().currentPos();
        current_->loopStart = loopStart;

        // ITER_NEXT: if exhausted, jump to exit (operand patched below)
        int iterNextIdx = chunk().emit(Opcode::ITER_NEXT, 0 /*placeholder*/);

        // Iterator variable lives inside the loop scope
        beginScope();
        int iterVarIdx = addLocal(node.iterVar, RhoType::Unknown);
        chunk().emit(Opcode::STORE_LOCAL, iterVarIdx);

        // Compile loop body statements
        for (auto& stmt : node.body->statements) {
            stmt->accept(*this);
        }

        endScope();  // pops iterVar local

        // Jump back to loop header
        chunk().emit(Opcode::JUMP, loopStart);

        // Patch ITER_NEXT exit to current position
        int exitPos = chunk().currentPos();
        chunk().patchJumpTo(iterNextIdx, exitPos);

        // Pop the iterator off the stack
        chunk().emit(Opcode::ITER_POP);

        int afterIterPop = chunk().currentPos();

        // Patch break jumps to after ITER_POP
        for (int idx : current_->breakJumps) {
            chunk().patchJumpTo(idx, afterIterPop);
        }
        // Patch continue jumps to loop header (re-check iterator)
        for (int idx : current_->continueJumps) {
            chunk().patchJumpTo(idx, loopStart);
        }

        // Restore outer loop context
        current_->loopStart     = savedLoopStart;
        current_->breakJumps    = std::move(savedBreaks);
        current_->continueJumps = std::move(savedContinues);
    }

    void visit(BreakNode& /*node*/) override {
        int jumpIdx = chunk().emitJump(Opcode::JUMP);
        current_->breakJumps.push_back(jumpIdx);
    }

    void visit(ContinueNode& /*node*/) override {
        if (current_->loopStart >= 0) {
            // Jump directly to loop start if already known
            chunk().emit(Opcode::JUMP, current_->loopStart);
        } else {
            // Will be patched later
            int jumpIdx = chunk().emitJump(Opcode::JUMP);
            current_->continueJumps.push_back(jumpIdx);
        }
    }

    void visit(ThrowNode& node) override {
        node.expression->accept(*this);
        chunk().emit(Opcode::THROW);
    }

    void visit(TryCatchNode& node) override {
        // SETUP_TRY(catch_target) — patched after try body
        int setupTryIdx = chunk().emit(Opcode::SETUP_TRY, 0 /*placeholder*/);

        // Compile try body
        node.tryBody->accept(*this);

        // Tear down the try frame if no exception
        chunk().emit(Opcode::TEAR_DOWN_TRY);

        // Jump over the catch block
        int jumpEnd = chunk().emitJump(Opcode::JUMP);

        // Patch SETUP_TRY to point here (catch block start)
        chunk().patchJump(setupTryIdx);

        // Catch block
        beginScope();
        int catchVarIdx = addLocal(node.catchClause.exceptionVar, RhoType::Unknown);
        chunk().emit(Opcode::CATCH_BIND, catchVarIdx);

        node.catchClause.body->accept(*this);

        endScope();

        // Patch end jump
        chunk().patchJump(jumpEnd);
    }

    void visit(MatchStmtNode& node) override {
        // Compile scrutinee; it stays on stack as comparison base
        node.scrutinee->accept(*this);

        std::vector<int> endJumps;

        for (auto& matchCase : node.cases) {
            if (matchCase.pattern) {
                // DUP scrutinee, compile pattern, compare
                chunk().emit(Opcode::DUP);
                matchCase.pattern->accept(*this);
                chunk().emit(Opcode::EQ);
                int jumpNextCase = chunk().emitJump(Opcode::JUMP_IF_FALSE);

                // Pattern matched — compile body
                matchCase.body->accept(*this);

                // Jump to end
                int jumpEnd = chunk().emitJump(Opcode::JUMP);
                endJumps.push_back(jumpEnd);

                // Patch "skip this case" jump
                chunk().patchJump(jumpNextCase);
            } else {
                // Wildcard: always matches
                matchCase.body->accept(*this);

                int jumpEnd = chunk().emitJump(Opcode::JUMP);
                endJumps.push_back(jumpEnd);
            }
        }

        // Pop the scrutinee (no case matched or after all cases)
        chunk().emit(Opcode::POP);

        // Patch all end-of-case jumps to here
        for (int idx : endJumps) {
            chunk().patchJump(idx);
        }
    }

    void visit(UsingNode& node) override {
        // Compile resource expression
        node.resourceExpr->accept(*this);

        // Bind resource to a local (or global)
        if (isGlobalScope()) {
            int nameIdx = chunk().addName(node.varName);
            chunk().emit(Opcode::STORE_GLOBAL, nameIdx);
        } else {
            int localIdx = addLocal(node.varName, RhoType::Unknown);
            chunk().emit(Opcode::STORE_LOCAL, localIdx);
        }

        // Compile body
        node.body->accept(*this);

        // Auto-close: load the resource and call io.close
        emitLoad(node.varName);
        int closeNameIdx = chunk().addName("io.close");
        chunk().emit(Opcode::CALL_MODULE, closeNameIdx);
        chunk().emit(Opcode::NOP, 1); // argc = 1
        chunk().emit(Opcode::POP);   // discard return value of close
    }

    void visit(IncludeNode& /*node*/) override {
        // Include is handled at the module level via Builtins singletons.
        // At bytecode level we emit a NOP since modules are globally accessible.
        chunk().emit(Opcode::NOP);
    }

    void visit(ProgramNode& node) override {
        for (auto& stmt : node.statements) {
            stmt->accept(*this);
        }
    }

private:
    // -----------------------------------------------------------------------
    // State management
    // -----------------------------------------------------------------------

    CompilerState*                              current_ = nullptr;
    std::vector<std::unique_ptr<CompilerState>> stateStack_;

    void pushState(const std::string& name, bool isTopLevel) {
        auto state = std::make_unique<CompilerState>(name, isTopLevel);
        state->enclosing = current_;
        current_ = state.get();
        stateStack_.push_back(std::move(state));
    }

    void popState() {
        assert(!stateStack_.empty());
        stateStack_.pop_back();
        current_ = stateStack_.empty() ? nullptr : stateStack_.back().get();
    }

    // -----------------------------------------------------------------------
    // Chunk accessor
    // -----------------------------------------------------------------------

    Chunk& chunk() {
        assert(current_ != nullptr);
        return *current_->chunk;
    }

    bool isGlobalScope() const {
        return current_->isTopLevel && current_->scopeDepth == 0;
    }

    // -----------------------------------------------------------------------
    // Scope helpers
    // -----------------------------------------------------------------------

    void beginScope() {
        ++current_->scopeDepth;
    }

    void endScope() {
        assert(current_->scopeDepth > 0);

        // Locals live at pre-allocated stack slots (bp + index) and must NOT be
        // POPped when their scope exits — the slot simply becomes dead and the
        // compiler stops generating references to it.  The frame is cleaned up as
        // a whole on RETURN / HALT.  Slot indices are reused automatically because
        // addLocal() assigns idx = current locals.size(), and we shrink the list
        // here without emitting any POP instructions.
        int depth = current_->scopeDepth;
        while (!current_->locals.empty() &&
               current_->locals.back().depth == depth) {
            current_->locals.pop_back();
        }

        --current_->scopeDepth;
    }

    // -----------------------------------------------------------------------
    // Local / upvalue resolution
    // -----------------------------------------------------------------------

    /**
     * @brief Add a local variable to the current scope.
     * @return Index of the new local slot.
     */
    int addLocal(const std::string& name, RhoType type) {
        current_->locals.push_back({name, type, current_->scopeDepth});
        int idx = static_cast<int>(current_->locals.size()) - 1;
        if (idx + 1 > current_->chunk->numLocals) {
            current_->chunk->numLocals = idx + 1;
        }
        return idx;
    }

    /**
     * @brief Search for a local by name in the given state.
     * @return Slot index or -1 if not found.
     */
    static int resolveLocal(CompilerState* state, const std::string& name) {
        for (int i = static_cast<int>(state->locals.size()) - 1; i >= 0; --i) {
            if (state->locals[i].name == name) {
                return i;
            }
        }
        return -1;
    }

    /**
     * @brief Add an upvalue record to the given state.
     * @return Upvalue index.
     */
    static int addUpvalue(CompilerState* state, bool isLocal, int index,
                          const std::string& name) {
        // Check if already captured
        for (int i = 0; i < static_cast<int>(state->upvalues.size()); ++i) {
            auto& uv = state->upvalues[i];
            if (uv.isLocal == isLocal && uv.index == index) {
                return i;
            }
        }
        state->upvalues.push_back({isLocal, index, name});
        return static_cast<int>(state->upvalues.size()) - 1;
    }

    /**
     * @brief Recursively resolve an upvalue, walking up the enclosing chain.
     * @return Upvalue index in the current function's upvalue table, or -1.
     */
    static int resolveUpvalue(CompilerState* state, const std::string& name) {
        if (state->enclosing == nullptr) {
            return -1; // reached the top without finding the variable
        }

        // Is it a local in the immediately enclosing function?
        int localIdx = resolveLocal(state->enclosing, name);
        if (localIdx != -1) {
            return addUpvalue(state, /*isLocal=*/true, localIdx, name);
        }

        // Recurse upward
        int upvalIdx = resolveUpvalue(state->enclosing, name);
        if (upvalIdx != -1) {
            return addUpvalue(state, /*isLocal=*/false, upvalIdx, name);
        }

        return -1;
    }

    // -----------------------------------------------------------------------
    // Emit load / store for a named variable (handles local/upvalue/global)
    // -----------------------------------------------------------------------

    void emitLoad(const std::string& name) {
        int localIdx = resolveLocal(current_, name);
        if (localIdx != -1) {
            chunk().emit(Opcode::LOAD_LOCAL, localIdx);
            return;
        }
        int upvalIdx = resolveUpvalue(current_, name);
        if (upvalIdx != -1) {
            chunk().emit(Opcode::LOAD_UPVALUE, upvalIdx);
            return;
        }
        int nameIdx = chunk().addName(name);
        chunk().emit(Opcode::LOAD_GLOBAL, nameIdx);
    }

    void emitStore(const std::string& name) {
        int localIdx = resolveLocal(current_, name);
        if (localIdx != -1) {
            chunk().emit(Opcode::STORE_LOCAL, localIdx);
            return;
        }
        int upvalIdx = resolveUpvalue(current_, name);
        if (upvalIdx != -1) {
            chunk().emit(Opcode::STORE_UPVALUE, upvalIdx);
            return;
        }
        int nameIdx = chunk().addName(name);
        chunk().emit(Opcode::STORE_GLOBAL, nameIdx);
    }
};

} // namespace Rhodesia

#endif // RHODESIA_COMPILER_HPP
