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
        if (match(TokenType::KwTry)) return parseTryCatch();
        if (match(TokenType::KwThrow)) return parseThrow();
        if (match(TokenType::KwMatch)) return parseMatchStmt();
        if (match(TokenType::KwReturn)) return parseReturn();
        if (match(TokenType::KwBreak)) return parseBreak();
        if (match(TokenType::KwContinue)) return parseContinue();
        if (match(TokenType::LBrace)) return parseBlock();

        // Check for variable declaration: tipo: nombre = ...
        if (isTypeKeyword(peek().type)) {
            return parseVarDecl();
        }

        // Check for tuple destructuring: (type: name, type: name, ...) = expr
        if (check(TokenType::LParen)) {
            size_t savedPos = current_;
            advance(); // consume '('
            if (isTypeKeyword(peek().type)) {
                current_ = savedPos;
                return parseTupleDestructure();
            }
            current_ = savedPos;
        }

        // Otherwise, expression or assignment
        return parseExprOrAssignment();
    }
    
    bool isTypeKeyword(TokenType type) const {
        return type == TokenType::KwInt ||
               type == TokenType::KwInt8 ||
               type == TokenType::KwInt16 ||
               type == TokenType::KwInt32 ||
               type == TokenType::KwUInt8 ||
               type == TokenType::KwUInt16 ||
               type == TokenType::KwUInt32 ||
               type == TokenType::KwUInt64 ||
               type == TokenType::KwByte ||
               type == TokenType::KwFloat64 ||
               type == TokenType::KwComplex ||
               type == TokenType::KwBool ||
               type == TokenType::KwVec ||
               type == TokenType::KwMat ||
               type == TokenType::KwStr ||
               type == TokenType::KwMap ||
               type == TokenType::KwArr ||
               type == TokenType::KwSet ||
               type == TokenType::KwTuple ||
               type == TokenType::KwRecord ||
               type == TokenType::KwEnum ||
               type == TokenType::KwFunction ||
               type == TokenType::KwNull;
    }
    
    RhoType parseType() {
        Token t = advance();
        switch (t.type) {
            case TokenType::KwInt: return RhoType::Int;
            case TokenType::KwInt8: return RhoType::Int8;
            case TokenType::KwInt16: return RhoType::Int16;
            case TokenType::KwInt32: return RhoType::Int32;
            case TokenType::KwUInt8: return RhoType::UInt8;
            case TokenType::KwUInt16: return RhoType::UInt16;
            case TokenType::KwUInt32: return RhoType::UInt32;
            case TokenType::KwUInt64: return RhoType::UInt64;
            case TokenType::KwByte: return RhoType::Byte;
            case TokenType::KwFloat64: return RhoType::Float64;
            case TokenType::KwComplex: return RhoType::Complex;
            case TokenType::KwBool: return RhoType::Bool;
            case TokenType::KwVec: return RhoType::Vec;
            case TokenType::KwMat: return RhoType::Mat;
            case TokenType::KwStr: return RhoType::String;
            case TokenType::KwMap: return RhoType::Map;
            case TokenType::KwArr: return RhoType::Arr;
            case TokenType::KwSet: return RhoType::Set;
            case TokenType::KwTuple: return RhoType::Tuple;
            case TokenType::KwRecord: return RhoType::Record;
            case TokenType::KwEnum: return RhoType::Enum;
            case TokenType::KwFunction: return RhoType::Function;
            case TokenType::KwVoid: return RhoType::Void;
            case TokenType::KwNull: return RhoType::Null;
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
     * @brief Parse tuple destructuring: (type: name, type: name, ...) = expr
     */
    StmtPtr parseTupleDestructure() {
        SourceLocation loc = peek().location;
        consume(TokenType::LParen, "'(' for tuple destructuring");

        std::vector<TupleDestructureTarget> targets;
        do {
            TupleDestructureTarget tgt;
            tgt.type = parseType();
            consume(TokenType::Colon, "':' after type in tuple destructuring");
            Token nameToken = consume(TokenType::Identifier, "variable name in tuple destructuring");
            tgt.name = nameToken.value;
            targets.push_back(std::move(tgt));
        } while (match(TokenType::Comma));

        consume(TokenType::RParen, "')' after tuple destructuring targets");
        consume(TokenType::Assign, "'=' after tuple destructuring targets");

        ExprPtr rhs = parseExpression();
        return std::make_unique<TupleDestructureNode>(std::move(targets), std::move(rhs), loc);
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
        if (check(TokenType::LParen)) {
            // Tuple return type: -> (type, type, ...)
            advance(); // consume '('
            while (!check(TokenType::RParen) && !isAtEnd()) {
                advance(); // skip type tokens and commas
            }
            consume(TokenType::RParen, "')' after tuple return type");
            returnType = RhoType::Tuple;
        } else if (isTypeKeyword(peek().type) || check(TokenType::KwVoid)) {
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
     * @brief Parse throw statement: throw expression
     */
    StmtPtr parseThrow() {
        SourceLocation loc = previous().location;
        ExprPtr expr = parseExpression();
        return std::make_unique<ThrowNode>(std::move(expr), loc);
    }

    /**
     * @brief Parse try/catch statement: try { body } catch exceptionVar { handler }
     */
    StmtPtr parseTryCatch() {
        SourceLocation loc = previous().location;

        // Parse try block
        consume(TokenType::LBrace, "'{' after 'try'");
        StmtPtr tryBody = parseBlockInner();

        // Parse catch clause
        consume(TokenType::KwCatch, "'catch' after try block");
        Token exceptionVar = consume(TokenType::Identifier, "exception variable name after 'catch'");

        consume(TokenType::LBrace, "'{' after catch variable");
        StmtPtr catchBody = parseBlockInner();

        CatchClause catchClause(exceptionVar.value, std::move(catchBody));
        return std::make_unique<TryCatchNode>(std::move(tryBody), std::move(catchClause), loc);
    }

    /**
     * @brief Parse include statement: include module_name{symbol1, symbol2 as alias}
     */
    StmtPtr parseInclude() {
        SourceLocation loc = previous().location;

        Token moduleToken = consume(TokenType::Identifier, "module name after 'include'");
        std::string moduleName = moduleToken.value;

        // Support path-based module names: core/core, statistics/descriptive, etc.
        while (check(TokenType::Slash)) {
            advance(); // consume '/'
            Token part = consume(TokenType::Identifier, "module path segment after '/'");
            moduleName += "/" + part.value;
        }

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
        return parseTernary();
    }

    /**
     * @brief Parse ternary conditional: cond ? expr1 : expr2
     * Lowest precedence, right-associative
     */
    ExprPtr parseTernary() {
        ExprPtr condition = parseOr();

        if (match(TokenType::Question)) {
            SourceLocation loc = previous().location;
            ExprPtr trueExpr = parseExpression();  // Right-associative, so parse full expression
            consume(TokenType::Colon, "':' after true expression in ternary operator");
            ExprPtr falseExpr = parseExpression();  // Right-associative
            return std::make_unique<TernaryOpNode>(std::move(condition), std::move(trueExpr), std::move(falseExpr), loc);
        }

        return condition;
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
        ExprPtr left = parseBitOr();

        while (match(TokenType::KwAnd)) {
            SourceLocation loc = previous().location;
            ExprPtr right = parseBitOr();
            left = std::make_unique<BinaryOpNode>(BinaryOp::And, std::move(left), std::move(right), loc);
        }

        return left;
    }

    ExprPtr parseBitOr() {
        ExprPtr left = parseBitXor();
        while (match(TokenType::Pipe)) {
            SourceLocation loc = previous().location;
            ExprPtr right = parseBitXor();
            left = std::make_unique<BinaryOpNode>(BinaryOp::BitOr, std::move(left), std::move(right), loc);
        }
        return left;
    }

    ExprPtr parseBitXor() {
        ExprPtr left = parseBitAnd();
        while (match(TokenType::Caret)) {
            SourceLocation loc = previous().location;
            ExprPtr right = parseBitAnd();
            left = std::make_unique<BinaryOpNode>(BinaryOp::BitXor, std::move(left), std::move(right), loc);
        }
        return left;
    }

    ExprPtr parseBitAnd() {
        ExprPtr left = parseEquality();
        while (match(TokenType::Ampersand)) {
            SourceLocation loc = previous().location;
            ExprPtr right = parseEquality();
            left = std::make_unique<BinaryOpNode>(BinaryOp::BitAnd, std::move(left), std::move(right), loc);
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
        ExprPtr left = parseShift();

        while (match({TokenType::Less, TokenType::Greater, TokenType::LessEqual, TokenType::GreaterEqual})) {
            Token op = previous();
            ExprPtr right = parseShift();
            
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

    ExprPtr parseShift() {
        ExprPtr left = parseTerm();
        while (match({TokenType::LShift, TokenType::RShift})) {
            Token op = previous();
            ExprPtr right = parseTerm();
            BinaryOp binOp = (op.type == TokenType::LShift) ? BinaryOp::Shl : BinaryOp::Shr;
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
        if (match(TokenType::Tilde)) {
            SourceLocation loc = previous().location;
            ExprPtr operand = parseUnary();
            return std::make_unique<UnaryOpNode>(UnaryOp::BitNot, std::move(operand), loc);
        }

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
            else if (check(TokenType::Less)) {
                // Need to distinguish between slice and comparison
                // Slice has form: expr<start:end> with a colon inside
                // Comparison doesn't - it's handled at a higher precedence level
                //
                // We peek ahead to see if this looks like a slice (has : or starts with :)
                // This is a simple heuristic: if we see < followed by : or <digit>+:, it's a slice
                // IMPORTANT: Must not confuse with ternary operator (? ... : ...)
                size_t savedPos = current_;
                advance(); // consume <

                bool isSlice = false;
                if (check(TokenType::Colon)) {
                    isSlice = true; // <:...>
                } else if (check(TokenType::IntLiteral) || check(TokenType::FloatLiteral) ||
                          check(TokenType::Identifier) || check(TokenType::Minus)) {
                    // Could be slice start, need to check for : after expression
                    // For now, use a simple heuristic: try to find : before > or other operators
                    // BUT: if we find ? before :, it's a ternary operator, not a slice
                    int depth = 0;
                    while (!isAtEnd() && depth < 10) { // look ahead max 10 tokens
                        if (check(TokenType::Question)) {
                            // This is a ternary operator, not a slice!
                            isSlice = false;
                            break;
                        }
                        if (check(TokenType::Colon)) {
                            isSlice = true;
                            break;
                        }
                        if (check(TokenType::Greater) && depth > 0) {
                            break;
                        }
                        // Stop if we see tokens that indicate it's not a slice
                        if (check(TokenType::RParen) || check(TokenType::RBracket) ||
                            check(TokenType::Newline) || check(TokenType::Eof) ||
                            check(TokenType::LBrace) || check(TokenType::RBrace)) {
                            break;
                        }
                        advance();
                        depth++;
                    }
                }

                // Restore position
                current_ = savedPos;

                if (isSlice) {
                    match(TokenType::Less); // consume the <
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
                } else {
                    // Not a slice, let higher precedence handle it (comparison operator)
                    break;
                }
            }
            else if (check(TokenType::LParen)) {
                // '(' after an expression result - stop here.
                // Chained calls like getMatrix()(0,0) are not supported.
                // The '(' belongs to the next statement.
                break;
            }
            else {
                break;
            }
        }

        return expr;
    }

    /**
     * @brief Parse slice specification: start:end, :end, start:, or :
     * Now supports full expressions including variables, arithmetic, and negative indices
     * NOTE: We parse up to Term level to avoid consuming comparison operators (<, >) or logical operators
     */
    SliceSpec parseSliceSpec() {
        std::optional<ExprPtr> start;
        std::optional<ExprPtr> end;

        // Check if we have a start expression (not starting with ':')
        if (!check(TokenType::Colon)) {
            start = parseTerm();  // Parse arithmetic expressions (no comparisons, no logical ops)
        }

        // Expect colon
        consume(TokenType::Colon, "':' in slice specification");

        // Check if we have an end expression (not ending with ',' or '>')
        if (!check(TokenType::Comma) && !check(TokenType::Greater)) {
            end = parseTerm();  // Parse arithmetic expressions (no comparisons, no logical ops)
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

        // Null literal
        if (match(TokenType::KwNull)) {
            return std::make_unique<NullLiteralNode>(loc);
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
        
        // Parenthesized expression or tuple: (expr) or (expr, expr, ...)
        if (match(TokenType::LParen)) {
            // Empty tuple: ()
            if (check(TokenType::RParen)) {
                advance();
                return std::make_unique<TupleLiteralNode>(std::vector<ExprPtr>{}, loc);
            }

            ExprPtr first = parseExpression();

            // Comma after first element => tuple
            if (check(TokenType::Comma)) {
                std::vector<ExprPtr> elements;
                elements.push_back(std::move(first));
                while (match(TokenType::Comma)) {
                    if (check(TokenType::RParen)) break;  // trailing comma allowed
                    elements.push_back(parseExpression());
                }
                consume(TokenType::RParen, "')' after tuple elements");
                return std::make_unique<TupleLiteralNode>(std::move(elements), loc);
            }

            // Single element: just a parenthesized expression
            consume(TokenType::RParen, "')' after expression");
            return first;
        }

        // Set or Record literal: {expr, ...} or {key: val, ...}
        if (match(TokenType::LBrace)) {
            return parseSetOrRecordLiteral(loc);
        }

        // Vector/Matrix literal
        if (match(TokenType::LBracket)) {
            return parseArrayLiteral(loc);
        }
        
        // Lambda expression
        if (match(TokenType::KwLambda)) {
            return parseLambda(loc);
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

    /**
     * @brief Parse set or record literal after '{'
     * Set:    { expr, expr, ... }
     * Record: { key: val, key: val, ... }
     */
    ExprPtr parseSetOrRecordLiteral(SourceLocation loc) {
        // Empty set: {}
        if (check(TokenType::RBrace)) {
            advance();
            return std::make_unique<SetLiteralNode>(std::vector<ExprPtr>{}, loc);
        }

        // Detect record: identifier followed immediately by ':'
        if (check(TokenType::Identifier) &&
            current_ + 1 < tokens_.size() &&
            tokens_[current_ + 1].type == TokenType::Colon) {
            return parseRecordLiteral(loc);
        }

        // Otherwise: set literal
        return parseSetLiteral(loc);
    }

    /**
     * @brief Parse set literal elements (already consumed '{')
     */
    ExprPtr parseSetLiteral(SourceLocation loc) {
        std::vector<ExprPtr> elements;
        if (!check(TokenType::RBrace)) {
            elements.push_back(parseExpression());
            while (match(TokenType::Comma)) {
                if (check(TokenType::RBrace)) break;  // trailing comma
                elements.push_back(parseExpression());
            }
        }
        consume(TokenType::RBrace, "'}' after set elements");
        return std::make_unique<SetLiteralNode>(std::move(elements), loc);
    }

    /**
     * @brief Parse record literal fields (already consumed '{')
     */
    ExprPtr parseRecordLiteral(SourceLocation loc) {
        std::vector<std::pair<std::string, ExprPtr>> fields;
        do {
            Token key = consume(TokenType::Identifier, "field name in record literal");
            consume(TokenType::Colon, "':' after field name in record literal");
            ExprPtr value = parseExpression();
            fields.emplace_back(key.value, std::move(value));
        } while (match(TokenType::Comma) && !check(TokenType::RBrace));
        consume(TokenType::RBrace, "'}' after record fields");
        return std::make_unique<RecordLiteralNode>(std::move(fields), loc);
    }

    /**
     * @brief Parse match statement: match expr { pattern -> { body } ... }
     */
    StmtPtr parseMatchStmt() {
        SourceLocation loc = previous().location;

        ExprPtr scrutinee = parseExpression();
        consume(TokenType::LBrace, "'{' after match expression");

        std::vector<MatchCase> cases;
        while (!check(TokenType::RBrace) && !isAtEnd()) {
            SourceLocation caseLoc = peek().location;

            // Wildcard: _
            ExprPtr pattern = nullptr;
            if (check(TokenType::Identifier) && peek().value == "_") {
                advance();  // consume _
            } else {
                pattern = parseExpression();
            }

            consume(TokenType::Arrow, "'->' after match pattern");
            consume(TokenType::LBrace, "'{' before match arm body");
            auto body = parseBlockInner();

            cases.push_back(MatchCase{std::move(pattern), std::move(body), caseLoc});
        }

        consume(TokenType::RBrace, "'}' after match cases");
        return std::make_unique<MatchStmtNode>(std::move(scrutinee), std::move(cases), loc);
    }

    /**
     * @brief Parse lambda expression
     * Syntax: lambda(x, y) { body } or fn(x) => expression
     */
    ExprPtr parseLambda(SourceLocation loc) {
        consume(TokenType::LParen, "'(' after lambda keyword");

        // Parse parameters
        std::vector<LambdaParam> params;
        if (!check(TokenType::RParen)) {
            do {
                // Support optional type annotations: lambda(int: x, y) or lambda(x, y)
                std::optional<RhoType> paramType;

                // Check if parameter has type annotation
                if (isTypeKeyword(peek().type)) {
                    paramType = parseType();
                    consume(TokenType::Colon, "':' after parameter type in lambda");
                }

                Token paramToken = consume(TokenType::Identifier, "parameter name");
                params.emplace_back(paramToken.value, paramType);
            } while (match(TokenType::Comma));
        }

        consume(TokenType::RParen, "')' after lambda parameters");

        // Check for arrow (expression lambda) or brace (block lambda)
        bool isExpression = false;
        std::unique_ptr<ExprNode> body;

        if (match(TokenType::Arrow)) {
            // Expression lambda: fn(x) => x * 2
            isExpression = true;
            body = parseExpression();
        } else if (match(TokenType::LBrace)) {
            // Block lambda: lambda(x) { return x * 2 }
            // Parse block as expression (will be evaluated as statement block)
            auto block = parseBlockInner();
            // Convert BlockNode to ExprNode wrapper (we'll handle this in evaluator)
            // For now, store the block pointer as expression
            body = std::unique_ptr<ExprNode>(reinterpret_cast<ExprNode*>(block.release()));
        } else {
            throw ParseError("Expected '=>' or '{' after lambda parameters", peek());
        }

        return std::make_unique<LambdaNode>(std::move(params), std::move(body), isExpression, loc);
    }
};

} // namespace Rhodesia

#endif // RHODESIA_PARSER_HPP
