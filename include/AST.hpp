/**
 * @file AST.hpp
 * @brief Abstract Syntax Tree node definitions for Rhodesia
 * 
 * Defines all AST node types using the Visitor pattern for evaluation.
 */

#ifndef RHODESIA_AST_HPP
#define RHODESIA_AST_HPP

#include "Token.hpp"
#include "RhoValue.hpp"
#include <memory>
#include <vector>
#include <string>
#include <optional>

namespace Rhodesia {

// Forward declarations
class ASTVisitor;

/*
 Base Classes
*/

/**
 * @brief Base class for all AST nodes
 */
class ASTNode {
public:
    virtual ~ASTNode() = default;
    
    SourceLocation location;
    
    virtual void accept(ASTVisitor& visitor) = 0;
    
protected:
    ASTNode() = default;
    explicit ASTNode(SourceLocation loc) : location(loc) {}
};

using ASTNodePtr = std::unique_ptr<ASTNode>;

/**
 * @brief Base class for expression nodes (produce values)
 */
class ExprNode : public ASTNode {
public:
    // Optional type annotation (for type checking)
    std::optional<RhoType> inferredType;
    
protected:
    ExprNode() = default;
    explicit ExprNode(SourceLocation loc) : ASTNode(loc) {}
};

using ExprPtr = std::unique_ptr<ExprNode>;

/**
 * @brief Base class for statement nodes (perform actions)
 */
class StmtNode : public ASTNode {
protected:
    StmtNode() = default;
    explicit StmtNode(SourceLocation loc) : ASTNode(loc) {}
};

using StmtPtr = std::unique_ptr<StmtNode>;

/*
 Expression Nodes
*/

/**
 * @brief Integer literal: 42
 */
class IntLiteralNode : public ExprNode {
public:
    int64_t value;
    
    explicit IntLiteralNode(int64_t val, SourceLocation loc = {})
        : ExprNode(loc), value(val) {
        inferredType = RhoType::Int;
    }
    
    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Float literal: 3.14
 */
class FloatLiteralNode : public ExprNode {
public:
    double value;
    
    explicit FloatLiteralNode(double val, SourceLocation loc = {})
        : ExprNode(loc), value(val) {
        inferredType = RhoType::Float64;
    }
    
    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Boolean literal: true, false
 */
class BoolLiteralNode : public ExprNode {
public:
    bool value;
    
    explicit BoolLiteralNode(bool val, SourceLocation loc = {})
        : ExprNode(loc), value(val) {}
    
    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief String literal: "hello"
 */
class StringLiteralNode : public ExprNode {
public:
    std::string value;

    explicit StringLiteralNode(std::string val, SourceLocation loc = {})
        : ExprNode(loc), value(std::move(val)) {}

    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Null literal: null
 */
class NullLiteralNode : public ExprNode {
public:
    explicit NullLiteralNode(SourceLocation loc = {})
        : ExprNode(loc) {}

    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Vector literal: [1.0, 2.0, 3.0]
 */
class VectorLiteralNode : public ExprNode {
public:
    std::vector<ExprPtr> elements;
    
    explicit VectorLiteralNode(std::vector<ExprPtr> elems, SourceLocation loc = {})
        : ExprNode(loc), elements(std::move(elems)) {
        inferredType = RhoType::Vec;
    }
    
    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Matrix literal: [[1, 2], [3, 4]]
 */
class MatrixLiteralNode : public ExprNode {
public:
    std::vector<std::vector<ExprPtr>> rows;
    
    explicit MatrixLiteralNode(std::vector<std::vector<ExprPtr>> r, SourceLocation loc = {})
        : ExprNode(loc), rows(std::move(r)) {
        inferredType = RhoType::Mat;
    }
    
    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Variable reference: x, myVar
 */
class IdentifierNode : public ExprNode {
public:
    std::string name;
    
    explicit IdentifierNode(std::string n, SourceLocation loc = {})
        : ExprNode(loc), name(std::move(n)) {}
    
    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Binary operator types
 */
enum class BinaryOp {
    Add,      // +
    Sub,      // -
    Mul,      // *
    Div,      // /
    Mod,      // %
    Eq,       // ==
    Ne,       // !=
    Lt,       // <
    Gt,       // >
    Le,       // <=
    Ge,       // >=
    And,      // and
    Or,       // or
    BitAnd,   // &
    BitOr,    // |
    BitXor,   // ^
    Shl,      // <<
    Shr       // >>
};

inline std::string binaryOpToString(BinaryOp op) {
    switch (op) {
        case BinaryOp::Add: return "+";
        case BinaryOp::Sub: return "-";
        case BinaryOp::Mul: return "*";
        case BinaryOp::Div: return "/";
        case BinaryOp::Mod: return "%";
        case BinaryOp::Eq:  return "==";
        case BinaryOp::Ne:  return "!=";
        case BinaryOp::Lt:  return "<";
        case BinaryOp::Gt:  return ">";
        case BinaryOp::Le:  return "<=";
        case BinaryOp::Ge:  return ">=";
        case BinaryOp::And:    return "and";
        case BinaryOp::Or:     return "or";
        case BinaryOp::BitAnd: return "&";
        case BinaryOp::BitOr:  return "|";
        case BinaryOp::BitXor: return "^";
        case BinaryOp::Shl:    return "<<";
        case BinaryOp::Shr:    return ">>";
    }
    return "?";
}

/**
 * @brief Binary operation: a + b, x * y
 */
class BinaryOpNode : public ExprNode {
public:
    BinaryOp op;
    ExprPtr left;
    ExprPtr right;
    
    BinaryOpNode(BinaryOp o, ExprPtr l, ExprPtr r, SourceLocation loc = {})
        : ExprNode(loc), op(o), left(std::move(l)), right(std::move(r)) {}
    
    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Unary operator types
 */
enum class UnaryOp {
    Neg,     // -
    Not,     // not
    BitNot   // ~
};

inline std::string unaryOpToString(UnaryOp op) {
    switch (op) {
        case UnaryOp::Neg:    return "-";
        case UnaryOp::Not:    return "not";
        case UnaryOp::BitNot: return "~";
    }
    return "?";
}

/**
 * @brief Unary operation: -x, not b
 */
class UnaryOpNode : public ExprNode {
public:
    UnaryOp op;
    ExprPtr operand;

    UnaryOpNode(UnaryOp o, ExprPtr expr, SourceLocation loc = {})
        : ExprNode(loc), op(o), operand(std::move(expr)) {}

    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Ternary conditional operator: cond ? expr1 : expr2
 */
class TernaryOpNode : public ExprNode {
public:
    ExprPtr condition;
    ExprPtr trueExpr;
    ExprPtr falseExpr;

    TernaryOpNode(ExprPtr cond, ExprPtr tExpr, ExprPtr fExpr, SourceLocation loc = {})
        : ExprNode(loc), condition(std::move(cond)), trueExpr(std::move(tExpr)), falseExpr(std::move(fExpr)) {}

    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Function call: norm(v), inv(m)
 */
class FunctionCallNode : public ExprNode {
public:
    std::string name;
    std::vector<ExprPtr> arguments;

    FunctionCallNode(std::string n, std::vector<ExprPtr> args, SourceLocation loc = {})
        : ExprNode(loc), name(std::move(n)), arguments(std::move(args)) {}

    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Member access: math.zeros, math.transpose
 */
class MemberAccessNode : public ExprNode {
public:
    std::string object;              // "math" or record variable name
    std::string member;              // "zeros" or field name
    std::vector<ExprPtr> arguments;  // Function arguments if it's a call
    bool isCalled = false;           // true when () was present (rec.fn() vs rec.fn)

    MemberAccessNode(std::string obj, std::string memb, std::vector<ExprPtr> args = {},
                     SourceLocation loc = {}, bool called = false)
        : ExprNode(loc), object(std::move(obj)), member(std::move(memb)),
          arguments(std::move(args)), isCalled(called) {}

    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Chained member access: expr.field  (where expr is not a plain identifier)
 *        Supports: a.b.c, a.b.c(), fun().field, etc.
 */
class ChainedMemberAccessNode : public ExprNode {
public:
    ExprPtr object;                  // any expression
    std::string field;               // field name
    std::vector<ExprPtr> arguments;  // arguments when called as a method
    bool isCalled = false;           // true when () was present

    ChainedMemberAccessNode(ExprPtr obj, std::string fld,
                             std::vector<ExprPtr> args = {},
                             SourceLocation loc = {}, bool called = false)
        : ExprNode(loc), object(std::move(obj)), field(std::move(fld)),
          arguments(std::move(args)), isCalled(called) {}

    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Index access: v[i], m[i, j]
 */
class IndexAccessNode : public ExprNode {
public:
    ExprPtr target;
    std::vector<ExprPtr> indices;

    IndexAccessNode(ExprPtr t, std::vector<ExprPtr> idx, SourceLocation loc = {})
        : ExprNode(loc), target(std::move(t)), indices(std::move(idx)) {}

    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Slice specification: start:end or just :end or start:
 */
struct SliceSpec {
    std::optional<ExprPtr> start;  // nullptr means start from beginning
    std::optional<ExprPtr> end;    // nullptr means go to end

    SliceSpec() = default;
    SliceSpec(std::optional<ExprPtr> s, std::optional<ExprPtr> e)
        : start(std::move(s)), end(std::move(e)) {}
};

/**
 * @brief Slice access: v<1:5>, m<0:2, 1:3>
 */
class SliceNode : public ExprNode {
public:
    ExprPtr target;
    std::vector<SliceSpec> slices;  // One for vectors, two for matrices

    SliceNode(ExprPtr t, std::vector<SliceSpec> s, SourceLocation loc = {})
        : ExprNode(loc), target(std::move(t)), slices(std::move(s)) {}

    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Lambda parameter (can be typed or untyped)
 */
struct LambdaParam {
    std::string name;
    std::optional<RhoType> type;  // Optional type annotation

    LambdaParam(std::string n, std::optional<RhoType> t = std::nullopt)
        : name(std::move(n)), type(t) {}
};

/**
 * @brief Lambda expression: lambda(x, y) { x + y } or fn(x) => x * 2
 */
class LambdaNode : public ExprNode {
public:
    std::vector<LambdaParam> params;
    std::unique_ptr<ExprNode> body;  // Single expression or block
    bool isExpression;  // true if body is a single expression (=> syntax)

    LambdaNode(std::vector<LambdaParam> p, std::unique_ptr<ExprNode> b,
               bool isExpr = false, SourceLocation loc = {})
        : ExprNode(loc), params(std::move(p)), body(std::move(b)), isExpression(isExpr) {
        inferredType = RhoType::Function;
    }

    void accept(ASTVisitor& visitor) override;
};

/*
 Statement Nodes
*/

/**
 * @brief Variable declaration: tipo: nombre = valor
 */
class VarDeclNode : public StmtNode {
public:
    RhoType type;
    std::string name;
    ExprPtr initializer;
    
    VarDeclNode(RhoType t, std::string n, ExprPtr init, SourceLocation loc = {})
        : StmtNode(loc), type(t), name(std::move(n)), initializer(std::move(init)) {}
    
    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Assignment: nombre = valor
 */
class AssignmentNode : public StmtNode {
public:
    std::string name;
    ExprPtr value;
    
    // For indexed assignment: v[i] = x
    std::vector<ExprPtr> indices;
    
    AssignmentNode(std::string n, ExprPtr val, SourceLocation loc = {})
        : StmtNode(loc), name(std::move(n)), value(std::move(val)) {}
    
    AssignmentNode(std::string n, std::vector<ExprPtr> idx, ExprPtr val, SourceLocation loc = {})
        : StmtNode(loc), name(std::move(n)), value(std::move(val)), indices(std::move(idx)) {}
    
    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Tuple destructuring assignment: (type: name, type: name, ...) = expr
 */
struct TupleDestructureTarget {
    RhoType type;
    std::string name;
};

class TupleDestructureNode : public StmtNode {
public:
    std::vector<TupleDestructureTarget> targets;
    ExprPtr rhs;

    TupleDestructureNode(std::vector<TupleDestructureTarget> tgts, ExprPtr r, SourceLocation loc = {})
        : StmtNode(loc), targets(std::move(tgts)), rhs(std::move(r)) {}

    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Expression statement: print(x)
 */
class ExprStmtNode : public StmtNode {
public:
    ExprPtr expression;
    
    explicit ExprStmtNode(ExprPtr expr, SourceLocation loc = {})
        : StmtNode(loc), expression(std::move(expr)) {}
    
    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Return statement: return expr
 */
class ReturnNode : public StmtNode {
public:
    ExprPtr value;  // May be nullptr for void return
    
    explicit ReturnNode(ExprPtr val = nullptr, SourceLocation loc = {})
        : StmtNode(loc), value(std::move(val)) {}
    
    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Block of statements: { ... }
 */
class BlockNode : public StmtNode {
public:
    std::vector<StmtPtr> statements;
    
    explicit BlockNode(std::vector<StmtPtr> stmts = {}, SourceLocation loc = {})
        : StmtNode(loc), statements(std::move(stmts)) {}
    
    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Function parameter
 */
struct FunctionParam {
    RhoType type;
    std::string name;
    SourceLocation location;
};

/**
 * @brief Function declaration: fun name(args) -> type { body }
 */
class FunctionDeclNode : public StmtNode {
public:
    std::string name;
    std::vector<FunctionParam> params;
    RhoType returnType;
    std::unique_ptr<BlockNode> body;
    
    FunctionDeclNode(std::string n, std::vector<FunctionParam> p, 
                     RhoType ret, std::unique_ptr<BlockNode> b, 
                     SourceLocation loc = {})
        : StmtNode(loc), name(std::move(n)), params(std::move(p)),
          returnType(ret), body(std::move(b)) {}
    
    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief For loop: for i in range(n) { body }
 */
class ForLoopNode : public StmtNode {
public:
    std::string iterVar;
    ExprPtr iterable;  // Usually a range() call
    std::unique_ptr<BlockNode> body;
    
    ForLoopNode(std::string var, ExprPtr iter, std::unique_ptr<BlockNode> b,
                SourceLocation loc = {})
        : StmtNode(loc), iterVar(std::move(var)), iterable(std::move(iter)),
          body(std::move(b)) {}
    
    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief While loop: while cond { body }
 */
class WhileLoopNode : public StmtNode {
public:
    ExprPtr condition;
    std::unique_ptr<BlockNode> body;
    
    WhileLoopNode(ExprPtr cond, std::unique_ptr<BlockNode> b, SourceLocation loc = {})
        : StmtNode(loc), condition(std::move(cond)), body(std::move(b)) {}
    
    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief If statement: if cond { then } else { else }
 */
class IfStmtNode : public StmtNode {
public:
    ExprPtr condition;
    std::unique_ptr<BlockNode> thenBranch;
    std::unique_ptr<BlockNode> elseBranch;  // May be nullptr
    
    IfStmtNode(ExprPtr cond, std::unique_ptr<BlockNode> then,
               std::unique_ptr<BlockNode> els = nullptr, SourceLocation loc = {})
        : StmtNode(loc), condition(std::move(cond)),
          thenBranch(std::move(then)), elseBranch(std::move(els)) {}
    
    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Break statement
 */
class BreakNode : public StmtNode {
public:
    explicit BreakNode(SourceLocation loc = {}) : StmtNode(loc) {}
    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Continue statement
 */
class ContinueNode : public StmtNode {
public:
    explicit ContinueNode(SourceLocation loc = {}) : StmtNode(loc) {}
    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Using statement: using expr as var { body }
 * Automatically calls io.close() on the variable when exiting the block
 */
class UsingNode : public StmtNode {
public:
    ExprPtr resourceExpr;     // Expression that returns the resource (e.g., io.open(...))
    std::string varName;      // Variable name to bind the resource to
    StmtPtr body;             // Block to execute

    UsingNode(ExprPtr expr, std::string name, StmtPtr bodyStmt, SourceLocation loc = {})
        : StmtNode(loc), resourceExpr(std::move(expr)), varName(std::move(name)),
          body(std::move(bodyStmt)) {}

    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Import specification for a single symbol with optional alias
 */
struct ImportSpec {
    std::string symbolName;  // Original name in module
    std::string alias;       // Alias name (empty if no alias)

    ImportSpec(std::string name, std::string aliasName = "")
        : symbolName(std::move(name)), alias(std::move(aliasName)) {}

    // Get the name to use in the importing scope
    std::string getImportedName() const {
        return alias.empty() ? symbolName : alias;
    }
};

/**
 * @brief Include statement: include module_name{symbol1, symbol2 as alias}
 */
class IncludeNode : public StmtNode {
public:
    std::string moduleName;
    std::vector<ImportSpec> symbols;  // Symbols to import (empty means import all)

    IncludeNode(std::string module, std::vector<ImportSpec> syms, SourceLocation loc = {})
        : StmtNode(loc), moduleName(std::move(module)), symbols(std::move(syms)) {}

    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Throw statement: throw expr
 */
class ThrowNode : public StmtNode {
public:
    ExprPtr expression;  // Expression to throw (can be string, error object, etc.)

    explicit ThrowNode(ExprPtr expr, SourceLocation loc = {})
        : StmtNode(loc), expression(std::move(expr)) {}

    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Catch clause for try/catch
 */
struct CatchClause {
    std::string exceptionVar;  // Variable name to bind the exception to
    StmtPtr body;              // Catch block body

    CatchClause(std::string var, StmtPtr catchBody)
        : exceptionVar(std::move(var)), body(std::move(catchBody)) {}
};

/**
 * @brief Try/catch statement: try { body } catch exceptionVar { handler }
 */
class TryCatchNode : public StmtNode {
public:
    StmtPtr tryBody;           // Try block body
    CatchClause catchClause;   // Catch clause

    TryCatchNode(StmtPtr tryStmt, CatchClause catchCl, SourceLocation loc = {})
        : StmtNode(loc), tryBody(std::move(tryStmt)), catchClause(std::move(catchCl)) {}

    void accept(ASTVisitor& visitor) override;
};

/*
 New Literal Nodes: Set, Tuple, Record
*/

/**
 * @brief Set literal: {1, 2, 3}
 */
class SetLiteralNode : public ExprNode {
public:
    std::vector<ExprPtr> elements;

    explicit SetLiteralNode(std::vector<ExprPtr> elems, SourceLocation loc = {})
        : ExprNode(loc), elements(std::move(elems)) {
        inferredType = RhoType::Set;
    }

    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Tuple literal: (1, "hello", 3.14)
 */
class TupleLiteralNode : public ExprNode {
public:
    std::vector<ExprPtr> elements;

    explicit TupleLiteralNode(std::vector<ExprPtr> elems, SourceLocation loc = {})
        : ExprNode(loc), elements(std::move(elems)) {
        inferredType = RhoType::Tuple;
    }

    void accept(ASTVisitor& visitor) override;
};

/**
 * @brief Record literal: { name: "Juan", age: 30 }
 */
class RecordLiteralNode : public ExprNode {
public:
    std::vector<std::pair<std::string, ExprPtr>> fields;

    explicit RecordLiteralNode(std::vector<std::pair<std::string, ExprPtr>> f, SourceLocation loc = {})
        : ExprNode(loc), fields(std::move(f)) {
        inferredType = RhoType::Record;
    }

    void accept(ASTVisitor& visitor) override;
};

/*
 Match Statement
*/

/**
 * @brief Single arm in a match statement
 */
struct MatchCase {
    ExprPtr pattern;               // nullptr = wildcard (_)
    std::unique_ptr<BlockNode> body;
    SourceLocation location;

    MatchCase(ExprPtr p, std::unique_ptr<BlockNode> b, SourceLocation loc = {})
        : pattern(std::move(p)), body(std::move(b)), location(loc) {}
};

/**
 * @brief Match statement: match expr { pattern -> { body } ... }
 */
class MatchStmtNode : public StmtNode {
public:
    ExprPtr scrutinee;
    std::vector<MatchCase> cases;

    MatchStmtNode(ExprPtr s, std::vector<MatchCase> c, SourceLocation loc = {})
        : StmtNode(loc), scrutinee(std::move(s)), cases(std::move(c)) {}

    void accept(ASTVisitor& visitor) override;
};

/*
 Program Node (Root)
*/

/**
 * @brief Root node containing all top-level declarations
 */
class ProgramNode : public ASTNode {
public:
    std::vector<StmtPtr> statements;
    
    explicit ProgramNode(std::vector<StmtPtr> stmts = {})
        : statements(std::move(stmts)) {}
    
    void accept(ASTVisitor& visitor) override;
};

/*
 Visitor Interface
*/

/**
 * @brief Visitor pattern interface for AST traversal
 */
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    
    // Expressions
    virtual void visit(IntLiteralNode& node) = 0;
    virtual void visit(FloatLiteralNode& node) = 0;
    virtual void visit(BoolLiteralNode& node) = 0;
    virtual void visit(StringLiteralNode& node) = 0;
    virtual void visit(NullLiteralNode& node) = 0;
    virtual void visit(VectorLiteralNode& node) = 0;
    virtual void visit(MatrixLiteralNode& node) = 0;
    virtual void visit(IdentifierNode& node) = 0;
    virtual void visit(BinaryOpNode& node) = 0;
    virtual void visit(UnaryOpNode& node) = 0;
    virtual void visit(TernaryOpNode& node) = 0;
    virtual void visit(FunctionCallNode& node) = 0;
    virtual void visit(MemberAccessNode& node) = 0;
    virtual void visit(ChainedMemberAccessNode& node) = 0;
    virtual void visit(IndexAccessNode& node) = 0;
    virtual void visit(SliceNode& node) = 0;
    virtual void visit(LambdaNode& node) = 0;
    virtual void visit(SetLiteralNode& node) = 0;
    virtual void visit(TupleLiteralNode& node) = 0;
    virtual void visit(RecordLiteralNode& node) = 0;

    // Statements
    virtual void visit(VarDeclNode& node) = 0;
    virtual void visit(AssignmentNode& node) = 0;
    virtual void visit(TupleDestructureNode& node) = 0;
    virtual void visit(ExprStmtNode& node) = 0;
    virtual void visit(ReturnNode& node) = 0;
    virtual void visit(BlockNode& node) = 0;
    virtual void visit(FunctionDeclNode& node) = 0;
    virtual void visit(ForLoopNode& node) = 0;
    virtual void visit(WhileLoopNode& node) = 0;
    virtual void visit(IfStmtNode& node) = 0;
    virtual void visit(BreakNode& node) = 0;
    virtual void visit(ContinueNode& node) = 0;
    virtual void visit(UsingNode& node) = 0;
    virtual void visit(IncludeNode& node) = 0;
    virtual void visit(ThrowNode& node) = 0;
    virtual void visit(TryCatchNode& node) = 0;
    virtual void visit(MatchStmtNode& node) = 0;

    // Program
    virtual void visit(ProgramNode& node) = 0;
};

/*
 Accept Implementations (inline)
*/

inline void IntLiteralNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void FloatLiteralNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void BoolLiteralNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void StringLiteralNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void NullLiteralNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void VectorLiteralNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void MatrixLiteralNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void IdentifierNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void BinaryOpNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void UnaryOpNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void TernaryOpNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void FunctionCallNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void MemberAccessNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void ChainedMemberAccessNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void IndexAccessNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void SliceNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void LambdaNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void VarDeclNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void AssignmentNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void TupleDestructureNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void ExprStmtNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void ReturnNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void BlockNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void FunctionDeclNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void ForLoopNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void WhileLoopNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void IfStmtNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void BreakNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void ContinueNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void UsingNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void IncludeNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void ThrowNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void TryCatchNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void SetLiteralNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void TupleLiteralNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void RecordLiteralNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void MatchStmtNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void ProgramNode::accept(ASTVisitor& v) { v.visit(*this); }

} // namespace Rhodesia

#endif // RHODESIA_AST_HPP
