/**
 * @file Parser.hpp
 * @brief Recursive descent parser for Rhodesia language
 * 
 * Implements a Pratt parser (precedence climbing) for expressions
 * and recursive descent for statements.
 */

#ifndef RHODESIA_PARSER_HPP
#define RHODESIA_PARSER_HPP

#include "Token.hpp"
#include "AST.hpp"
#include "Error.hpp"
#include <vector>
#include <unordered_map>

namespace Rhodesia {

/**
 * @brief Parser for Rhodesia source code
 * 
 * Converts a token stream into an Abstract Syntax Tree.
 */
class Parser {
public:
    /**
     * @brief Construct parser with token stream
     * @param tokens Vector of tokens from Lexer
     */
    explicit Parser(std::vector<Token> tokens)
        : tokens_(std::move(tokens)), current_(0) {}
    
    /**
     * @brief Parse entire program
     * @return Root ProgramNode of AST
     */
    std::unique_ptr<ProgramNode> parse() {
        std::vector<StmtPtr> statements;
        
        while (!isAtEnd()) {
            try {
                statements.push_back(parseStatement());
            } catch (const ParseError& e) {
                // Error recovery: skip to next statement
                synchronize();
                throw;
            }
        }
        
        return std::make_unique<ProgramNode>(std::move(statements));
    }

private:
    std::vector<Token> tokens_;
    size_t current_;
    
    // ========================================================================
    // Token Navigation
    // ========================================================================
    
    const Token& peek() const {
        return tokens_[current_];
    }
    
    const Token& previous() const {
        return tokens_[current_ - 1];
    }
    
    bool isAtEnd() const {
        return peek().type == TokenType::Eof;
    }
    
    Token advance() {
        if (!isAtEnd()) current_++;
        return previous();
    }
    
    bool check(TokenType type) const {
        if (isAtEnd()) return false;
        return peek().type == type;
    }
    
    bool match(TokenType type) {
        if (check(type)) {
            advance();
            return true;
        }
        return false;
    }
    
    bool match(std::initializer_list<TokenType> types) {
        for (auto type : types) {
            if (check(type)) {
                advance();
                return true;
            }
        }
        return false;
    }
    
    Token consume(TokenType type, const std::string& message) {
        if (check(type)) return advance();
        throw ParseError::unexpectedToken(peek(), message);
    }
    
    /**
     * @brief Error recovery: skip to next statement boundary
     */
    void synchronize() {
        advance();
        
        while (!isAtEnd()) {
            // Statement boundaries
            if (previous().type == TokenType::Semicolon) return;
            if (previous().type == TokenType::RBrace) return;
            
            switch (peek().type) {
                case TokenType::KwFun:
                case TokenType::KwFor:
                case TokenType::KwIf:
                case TokenType::KwWhile:
                case TokenType::KwReturn:
                    return;
                default:
                    break;
            }
            
            advance();
        }
    }
    
    // ========================================================================
    // Statement Parsing
    // ========================================================================
    
    StmtPtr parseStatement() {
        if (match(TokenType::KwInclude)) return parseInclude();
        if (match(TokenType::KwFun)) return parseFunctionDecl();
        if (match(TokenType::KwFor)) return parseForLoop();
        if (match(TokenType::KwWhile)) return parseWhileLoop();
        if (match(TokenType::KwIf)) return parseIfStmt();
        if (match(TokenType::KwUsing)) return parseUsing();
        if (match(TokenType::KwReturn)) return parseReturn();
        if (match(TokenType::KwBreak)) return parseBreak();
        if (match(TokenType::KwContinue)) return parseContinue();
        if (match(TokenType::LBrace)) return parseBlock();
        
        // Check for variable declaration: tipo: nombre = ...
        if (isTypeKeyword(peek().type)) {
            return parseVarDecl();
        }
        
        // Otherwise, expression or assignment
        return parseExprOrAssignment();
    }
    
    bool isTypeKeyword(TokenType type) const {
        return type == TokenType::KwInt ||
               type == TokenType::KwFloat64 ||
               type == TokenType::KwVec ||
               type == TokenType::KwMat ||
               type == TokenType::KwStr ||
               type == TokenType::KwMap;
    }
    
    RhoType parseType() {
        Token t = advance();
        switch (t.type) {
            case TokenType::KwInt: return RhoType::Int;
            case TokenType::KwFloat64: return RhoType::Float64;
            case TokenType::KwVec: return RhoType::Vec;
            case TokenType::KwMat: return RhoType::Mat;
            case TokenType::KwStr: return RhoType::String;
            case TokenType::KwMap: return RhoType::Map;
            case TokenType::KwVoid: return RhoType::Void;
            default:
                throw ParseError("Expected type", t);
        }
    }
    
    /**
     * @brief Parse variable declaration: tipo: nombre = valor
     */
    StmtPtr parseVarDecl() {
        SourceLocation loc = peek().location;
        RhoType type = parseType();

        consume(TokenType::Colon, "':' after type in variable declaration");

        Token nameToken = consume(TokenType::Identifier, "variable name after type");
        std::string name = nameToken.value;

        consume(TokenType::Assign, "'=' after variable name in declaration");

        ExprPtr init = parseExpression();

        return std::make_unique<VarDeclNode>(type, name, std::move(init), loc);
    }
    
    /**
     * @brief Parse expression statement or assignment
     */
    StmtPtr parseExprOrAssignment() {
        SourceLocation loc = peek().location;
        ExprPtr expr = parseExpression();
        
        // Check for assignment
        if (match(TokenType::Assign)) {
            // Must be an identifier or indexed access
            if (auto* id = dynamic_cast<IdentifierNode*>(expr.get())) {
                ExprPtr value = parseExpression();
                return std::make_unique<AssignmentNode>(id->name, std::move(value), loc);
            }
            if (auto* idx = dynamic_cast<IndexAccessNode*>(expr.get())) {
                if (auto* target = dynamic_cast<IdentifierNode*>(idx->target.get())) {
                    ExprPtr value = parseExpression();
                    return std::make_unique<AssignmentNode>(
                        target->name, std::move(idx->indices), std::move(value), loc);
                }
            }
            throw ParseError("Invalid assignment target", loc);
        }
        
        return std::make_unique<ExprStmtNode>(std::move(expr), loc);
    }
    
    /**
     * @brief Parse function declaration
     */
    StmtPtr parseFunctionDecl() {
        SourceLocation loc = previous().location;
        
        Token nameToken = consume(TokenType::Identifier, "function name");
        std::string name = nameToken.value;
        
        consume(TokenType::LParen, "'(' after function name");
        
        // Parse parameters
        std::vector<FunctionParam> params;
        if (!check(TokenType::RParen)) {
            do {
                FunctionParam param;
                param.location = peek().location;
                param.type = parseType();
                consume(TokenType::Colon, "':' after parameter type");
                Token pname = consume(TokenType::Identifier, "parameter name");
                param.name = pname.value;
                params.push_back(param);
            } while (match(TokenType::Comma));
        }
        
        consume(TokenType::RParen, "')' after parameters");
        consume(TokenType::Arrow, "'->' before return type");
        
        RhoType returnType = RhoType::Void;
        if (isTypeKeyword(peek().type) || check(TokenType::KwVoid)) {
            returnType = parseType();
        }
        
        consume(TokenType::LBrace, "'{' before function body");
        auto body = parseBlockInner();
        
        return std::make_unique<FunctionDeclNode>(
            name, std::move(params), returnType, std::move(body), loc);
    }
    
    /**
     * @brief Parse for loop: for var in iterable { body }
     */
    StmtPtr parseForLoop() {
        SourceLocation loc = previous().location;
        
        Token varToken = consume(TokenType::Identifier, "loop variable");
        std::string iterVar = varToken.value;
        
        consume(TokenType::KwIn, "'in' keyword in for loop");
        
        ExprPtr iterable = parseExpression();
        
        consume(TokenType::LBrace, "'{' before loop body");
        auto body = parseBlockInner();
        
        return std::make_unique<ForLoopNode>(iterVar, std::move(iterable), std::move(body), loc);
    }
    
    /**
     * @brief Parse while loop: while cond { body }
     */
    StmtPtr parseWhileLoop() {
        SourceLocation loc = previous().location;
        
        ExprPtr condition = parseExpression();
        
        consume(TokenType::LBrace, "'{' before while body");
        auto body = parseBlockInner();
        
        return std::make_unique<WhileLoopNode>(std::move(condition), std::move(body), loc);
    }
    
    /**
     * @brief Parse if statement: if cond { then } else { else }
     */
    StmtPtr parseIfStmt() {
        SourceLocation loc = previous().location;
        
        ExprPtr condition = parseExpression();
        
        consume(TokenType::LBrace, "'{' before if body");
        auto thenBranch = parseBlockInner();
        
        std::unique_ptr<BlockNode> elseBranch = nullptr;
        if (match(TokenType::KwElse)) {
            if (match(TokenType::KwIf)) {
                // else if -> wrap in block
                auto elseIf = parseIfStmt();
                std::vector<StmtPtr> stmts;
                stmts.push_back(std::move(elseIf));
                elseBranch = std::make_unique<BlockNode>(std::move(stmts));
            } else {
                consume(TokenType::LBrace, "'{' before else body");
                elseBranch = parseBlockInner();
            }
        }
        
        return std::make_unique<IfStmtNode>(
            std::move(condition), std::move(thenBranch), std::move(elseBranch), loc);
    }

    /**
     * @brief Parse using statement: using expr as var { body }
     */
    StmtPtr parseUsing() {
        SourceLocation loc = previous().location;

        // Parse the resource expression (e.g., io.open(...))
        ExprPtr resourceExpr = parseExpression();

        // Expect 'as' keyword
        consume(TokenType::KwAs, "'as' after resource expression in using statement");

        // Expect variable name
        Token varToken = consume(TokenType::Identifier, "variable name after 'as' in using statement");
        std::string varName = varToken.value;

        // Expect block
        consume(TokenType::LBrace, "'{' before using body");
        auto body = parseBlockInner();

        return std::make_unique<UsingNode>(std::move(resourceExpr), std::move(varName), std::move(body), loc);
    }

    /**
     * @brief Parse return statement
     */
    StmtPtr parseReturn() {
        SourceLocation loc = previous().location;
        
        ExprPtr value = nullptr;
        if (!check(TokenType::RBrace) && !check(TokenType::Eof)) {
            // Has return value
            value = parseExpression();
        }
        
        return std::make_unique<ReturnNode>(std::move(value), loc);
    }
    
    StmtPtr parseBreak() {
        return std::make_unique<BreakNode>(previous().location);
    }
    
    StmtPtr parseContinue() {
        return std::make_unique<ContinueNode>(previous().location);
    }

    /**
     * @brief Parse include statement: include module_name{symbol1, symbol2 as alias}
     */
    StmtPtr parseInclude() {
        SourceLocation loc = previous().location;

        Token moduleToken = consume(TokenType::Identifier, "module name after 'include'");
        std::string moduleName = moduleToken.value;

        std::vector<ImportSpec> symbols;

        // Check for selective import: include module{func1, func2 as alias}
        if (match(TokenType::LBrace)) {
            if (!check(TokenType::RBrace)) {
                do {
                    Token symbol = consume(TokenType::Identifier, "symbol name in include");
                    std::string symbolName = symbol.value;
                    std::string alias = "";

                    // Check for alias: symbol as alias_name
                    if (match(TokenType::KwAs)) {
                        Token aliasToken = consume(TokenType::Identifier, "alias name after 'as'");
                        alias = aliasToken.value;
                    }

                    symbols.emplace_back(symbolName, alias);
                } while (match(TokenType::Comma));
            }
            consume(TokenType::RBrace, "'}' after include symbols");
        }
        // If no braces, import all symbols from module (symbols will be empty)

        return std::make_unique<IncludeNode>(moduleName, std::move(symbols), loc);
    }

    /**
     * @brief Parse block after '{'
     */
    StmtPtr parseBlock() {
        return parseBlockInner();
    }
    
    std::unique_ptr<BlockNode> parseBlockInner() {
        SourceLocation loc = previous().location;
        std::vector<StmtPtr> statements;
        
        while (!check(TokenType::RBrace) && !isAtEnd()) {
            statements.push_back(parseStatement());
        }
        
        consume(TokenType::RBrace, "'}' after block");
        
        return std::make_unique<BlockNode>(std::move(statements), loc);
    }
    
    // ========================================================================
    // Expression Parsing (Pratt Parser / Precedence Climbing)
    // ========================================================================
    
    ExprPtr parseExpression() {
        return parseOr();
    }
    
    ExprPtr parseOr() {
        ExprPtr left = parseAnd();
        
        while (match(TokenType::KwOr)) {
            SourceLocation loc = previous().location;
            ExprPtr right = parseAnd();
            left = std::make_unique<BinaryOpNode>(BinaryOp::Or, std::move(left), std::move(right), loc);
        }
        
        return left;
    }
    
    ExprPtr parseAnd() {
        ExprPtr left = parseEquality();
        
        while (match(TokenType::KwAnd)) {
            SourceLocation loc = previous().location;
            ExprPtr right = parseEquality();
            left = std::make_unique<BinaryOpNode>(BinaryOp::And, std::move(left), std::move(right), loc);
        }
        
        return left;
    }
    
    ExprPtr parseEquality() {
        ExprPtr left = parseComparison();
        
        while (match({TokenType::Equal, TokenType::NotEqual})) {
            Token op = previous();
            ExprPtr right = parseComparison();
            BinaryOp binOp = (op.type == TokenType::Equal) ? BinaryOp::Eq : BinaryOp::Ne;
            left = std::make_unique<BinaryOpNode>(binOp, std::move(left), std::move(right), op.location);
        }
        
        return left;
    }
    
    ExprPtr parseComparison() {
        ExprPtr left = parseTerm();
        
        while (match({TokenType::Less, TokenType::Greater, TokenType::LessEqual, TokenType::GreaterEqual})) {
            Token op = previous();
            ExprPtr right = parseTerm();
            
            BinaryOp binOp;
            switch (op.type) {
                case TokenType::Less: binOp = BinaryOp::Lt; break;
                case TokenType::Greater: binOp = BinaryOp::Gt; break;
                case TokenType::LessEqual: binOp = BinaryOp::Le; break;
                case TokenType::GreaterEqual: binOp = BinaryOp::Ge; break;
                default: binOp = BinaryOp::Lt; break;
            }
            
            left = std::make_unique<BinaryOpNode>(binOp, std::move(left), std::move(right), op.location);
        }
        
        return left;
    }
    
    ExprPtr parseTerm() {
        ExprPtr left = parseFactor();
        
        while (match({TokenType::Plus, TokenType::Minus})) {
            Token op = previous();
            ExprPtr right = parseFactor();
            BinaryOp binOp = (op.type == TokenType::Plus) ? BinaryOp::Add : BinaryOp::Sub;
            left = std::make_unique<BinaryOpNode>(binOp, std::move(left), std::move(right), op.location);
        }
        
        return left;
    }
    
    ExprPtr parseFactor() {
        ExprPtr left = parseUnary();
        
        while (match({TokenType::Star, TokenType::Slash, TokenType::Percent})) {
            Token op = previous();
            ExprPtr right = parseUnary();
            
            BinaryOp binOp;
            switch (op.type) {
                case TokenType::Star: binOp = BinaryOp::Mul; break;
                case TokenType::Slash: binOp = BinaryOp::Div; break;
                case TokenType::Percent: binOp = BinaryOp::Mod; break;
                default: binOp = BinaryOp::Mul; break;
            }
            
            left = std::make_unique<BinaryOpNode>(binOp, std::move(left), std::move(right), op.location);
        }
        
        return left;
    }
    
    ExprPtr parseUnary() {
        if (match(TokenType::Minus)) {
            SourceLocation loc = previous().location;
            ExprPtr operand = parseUnary();
            return std::make_unique<UnaryOpNode>(UnaryOp::Neg, std::move(operand), loc);
        }
        
        if (match(TokenType::KwNot)) {
            SourceLocation loc = previous().location;
            ExprPtr operand = parseUnary();
            return std::make_unique<UnaryOpNode>(UnaryOp::Not, std::move(operand), loc);
        }
        
        return parsePostfix();
    }
    
    ExprPtr parsePostfix() {
        ExprPtr expr = parsePrimary();

        while (true) {
            if (match(TokenType::LBracket)) {
                // Index access: expr[i] or expr[i, j]
                SourceLocation loc = previous().location;
                std::vector<ExprPtr> indices;

                indices.push_back(parseExpression());
                while (match(TokenType::Comma)) {
                    indices.push_back(parseExpression());
                }

                consume(TokenType::RBracket, "']' after index");
                expr = std::make_unique<IndexAccessNode>(std::move(expr), std::move(indices), loc);
            }
            else if (match(TokenType::Dot)) {
                // Member access: expr.member or module.function(args)
                // For now, we only support simple identifiers before the dot
                SourceLocation loc = previous().location;

                // Get the object/module name from expr
                auto* ident = dynamic_cast<IdentifierNode*>(expr.get());
                if (!ident) {
                    throw ParseError("Member access requires an identifier before '.'", loc);
                }

                // Parse member name
                Token memberToken = consume(TokenType::Identifier, "member name after '.'");
                std::string member = memberToken.value;

                // Check if it's a function call (has parentheses)
                if (match(TokenType::LParen)) {
                    std::vector<ExprPtr> args;
                    if (!check(TokenType::RParen)) {
                        args.push_back(parseExpression());
                        while (match(TokenType::Comma)) {
                            args.push_back(parseExpression());
                        }
                    }
                    consume(TokenType::RParen, "')' after arguments");
                    expr = std::make_unique<MemberAccessNode>(ident->name, member, std::move(args), loc);
                } else {
                    // Member access without call (e.g., math.PI for constants)
                    expr = std::make_unique<MemberAccessNode>(ident->name, member, std::vector<ExprPtr>{}, loc);
                }
            }
            else if (match(TokenType::Less)) {
                // Slice access: expr<start:end> or expr<row_start:row_end, col_start:col_end>
                SourceLocation loc = previous().location;
                std::vector<SliceSpec> slices;

                // Parse first slice (start:end)
                slices.push_back(parseSliceSpec());

                // Check for second slice (for matrices)
                while (match(TokenType::Comma)) {
                    slices.push_back(parseSliceSpec());
                }

                consume(TokenType::Greater, "'>' after slice");
                expr = std::make_unique<SliceNode>(std::move(expr), std::move(slices), loc);
            }
            else if (match(TokenType::LParen)) {
                // Function call on result (chained)
                // This handles cases like: getMatrix()(0, 0)
                SourceLocation loc = previous().location;
                // For now, disallow - functions are called by name only
                throw ParseError("Cannot call expression result as function", loc);
            }
            else {
                break;
            }
        }

        return expr;
    }

    /**
     * @brief Parse slice specification: start:end, :end, start:, or :
     */
    SliceSpec parseSliceSpec() {
        std::optional<ExprPtr> start;
        std::optional<ExprPtr> end;

        // Check if we have a start expression (not starting with ':')
        if (!check(TokenType::Colon)) {
            start = parseTerm();  // Parse arithmetic expressions only
        }

        // Expect colon
        consume(TokenType::Colon, "':' in slice specification");

        // Check if we have an end expression (not ending with ',' or '>')
        if (!check(TokenType::Comma) && !check(TokenType::Greater)) {
            end = parseTerm();  // Parse arithmetic expressions only
        }

        return SliceSpec(std::move(start), std::move(end));
    }
    
    ExprPtr parsePrimary() {
        SourceLocation loc = peek().location;
        
        // Boolean literals
        if (match(TokenType::KwTrue)) {
            return std::make_unique<BoolLiteralNode>(true, loc);
        }
        if (match(TokenType::KwFalse)) {
            return std::make_unique<BoolLiteralNode>(false, loc);
        }
        
        // Number literals
        if (match(TokenType::IntLiteral)) {
            int64_t value = std::stoll(previous().value);
            return std::make_unique<IntLiteralNode>(value, loc);
        }
        if (match(TokenType::FloatLiteral)) {
            double value = std::stod(previous().value);
            return std::make_unique<FloatLiteralNode>(value, loc);
        }
        
        // String literal
        if (match(TokenType::StringLiteral)) {
            return std::make_unique<StringLiteralNode>(previous().value, loc);
        }
        
        // Parenthesized expression
        if (match(TokenType::LParen)) {
            ExprPtr expr = parseExpression();
            consume(TokenType::RParen, "')' after expression");
            return expr;
        }
        
        // Vector/Matrix literal
        if (match(TokenType::LBracket)) {
            return parseArrayLiteral(loc);
        }
        
        // Identifier or function call
        if (match(TokenType::Identifier)) {
            std::string name = previous().value;
            
            // Check for function call
            if (match(TokenType::LParen)) {
                return parseFunctionCall(name, loc);
            }
            
            return std::make_unique<IdentifierNode>(name, loc);
        }
        
        throw ParseError("Expected expression", peek());
    }
    
    /**
     * @brief Parse array literal [1, 2, 3] or [[1, 2], [3, 4]]
     */
    ExprPtr parseArrayLiteral(SourceLocation loc) {
        // Check if first element is also a bracket (matrix)
        if (check(TokenType::LBracket)) {
            // Matrix literal
            std::vector<std::vector<ExprPtr>> rows;
            
            do {
                consume(TokenType::LBracket, "'[' for matrix row");
                std::vector<ExprPtr> row;
                
                if (!check(TokenType::RBracket)) {
                    row.push_back(parseExpression());
                    while (match(TokenType::Comma)) {
                        row.push_back(parseExpression());
                    }
                }
                
                consume(TokenType::RBracket, "']' after matrix row");
                rows.push_back(std::move(row));
            } while (match(TokenType::Comma) && check(TokenType::LBracket));
            
            consume(TokenType::RBracket, "']' after matrix");
            return std::make_unique<MatrixLiteralNode>(std::move(rows), loc);
        }
        
        // Vector literal
        std::vector<ExprPtr> elements;
        
        if (!check(TokenType::RBracket)) {
            elements.push_back(parseExpression());
            while (match(TokenType::Comma)) {
                elements.push_back(parseExpression());
            }
        }
        
        consume(TokenType::RBracket, "']' after vector");
        return std::make_unique<VectorLiteralNode>(std::move(elements), loc);
    }
    
    /**
     * @brief Parse function call arguments
     */
    ExprPtr parseFunctionCall(const std::string& name, SourceLocation loc) {
        std::vector<ExprPtr> args;
        
        if (!check(TokenType::RParen)) {
            args.push_back(parseExpression());
            while (match(TokenType::Comma)) {
                args.push_back(parseExpression());
            }
        }
        
        consume(TokenType::RParen, "')' after arguments");
        return std::make_unique<FunctionCallNode>(name, std::move(args), loc);
    }
};

} // namespace Rhodesia

#endif // RHODESIA_PARSER_HPP
