/**
 * @file Lexer.hpp
 * @brief Lexical analyzer for Rhodesia language
 */

#ifndef RHODESIA_LEXER_HPP
#define RHODESIA_LEXER_HPP

#include "Token.hpp"
#include "Error.hpp"
#include <string>
#include <vector>
#include <cctype>
#include <stdexcept>

namespace Rhodesia {

/**
 * @brief Lexer class for tokenizing Rhodesia source code
 * 
 * Converts source string into a stream of tokens for the parser.
 * Handles numbers (int and float), identifiers, keywords, operators,
 * and delimiters.
 */
class Lexer {
public:
    /**
     * @brief Construct lexer with source code
     * @param source The Rhodesia source code to tokenize
     */
    explicit Lexer(std::string source) 
        : source_(std::move(source)), pos_(0), line_(1), column_(1) {}
    
    /**
     * @brief Tokenize the entire source code
     * @return Vector of tokens
     * @throws LexerError on invalid input
     */
    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        
        while (!isAtEnd()) {
            skipWhitespaceAndComments();
            if (isAtEnd()) break;
            
            Token token = nextToken();
            if (token.type != TokenType::Invalid) {
                tokens.push_back(token);
            }
        }
        
        tokens.push_back(Token(TokenType::Eof, "", currentLocation()));
        return tokens;
    }
    
    /**
     * @brief Get the next token (streaming mode)
     * @return Next token from source
     */
    Token nextToken() {
        skipWhitespaceAndComments();
        
        if (isAtEnd()) {
            return Token(TokenType::Eof, "", currentLocation());
        }
        
        SourceLocation startLoc = currentLocation();
        char c = peek();
        
        // Numbers
        if (std::isdigit(c)) {
            return readNumber(startLoc);
        }
        
        // Identifiers and keywords
        if (std::isalpha(c) || c == '_') {
            return readIdentifier(startLoc);
        }
        
        // Operators and delimiters
        return readOperatorOrDelimiter(startLoc);
    }

private:
    std::string source_;
    size_t pos_;
    size_t line_;
    size_t column_;
    
    /**
     * @brief Check if at end of source
     */
    bool isAtEnd() const {
        return pos_ >= source_.length();
    }
    
    /**
     * @brief Peek at current character without consuming
     */
    char peek() const {
        if (isAtEnd()) return '\0';
        return source_[pos_];
    }
    
    /**
     * @brief Peek at next character without consuming
     */
    char peekNext() const {
        if (pos_ + 1 >= source_.length()) return '\0';
        return source_[pos_ + 1];
    }
    
    /**
     * @brief Consume and return current character
     */
    char advance() {
        char c = source_[pos_++];
        if (c == '\n') {
            line_++;
            column_ = 1;
        } else {
            column_++;
        }
        return c;
    }
    
    /**
     * @brief Check if current char matches expected and consume if so
     */
    bool match(char expected) {
        if (isAtEnd() || source_[pos_] != expected) return false;
        advance();
        return true;
    }
    
    /**
     * @brief Get current source location
     */
    SourceLocation currentLocation() const {
        return {line_, column_, pos_};
    }
    
    /**
     * @brief Skip whitespace and comments
     */
    void skipWhitespaceAndComments() {
        while (!isAtEnd()) {
            char c = peek();
            
            // Skip whitespace (but not newlines in certain modes)
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
                advance();
                continue;
            }
            
            // Single-line comment
            if (c == '/' && peekNext() == '/') {
                advance(); // consume first /
                advance(); // consume second /
                while (!isAtEnd() && peek() != '\n') {
                    advance();
                }
                continue;
            }
            
            // Multi-line comment
            if (c == '/' && peekNext() == '*') {
                advance(); // consume /
                advance(); // consume *
                while (!isAtEnd()) {
                    if (peek() == '*' && peekNext() == '/') {
                        advance(); // consume *
                        advance(); // consume /
                        break;
                    }
                    advance();
                }
                continue;
            }
            
            break;
        }
    }
    
    /**
     * @brief Read a number literal (int or float)
     */
    Token readNumber(SourceLocation startLoc) {
        std::string value;
        bool isFloat = false;
        
        // Read integer part
        while (!isAtEnd() && std::isdigit(peek())) {
            value += advance();
        }
        
        // Check for decimal point
        if (peek() == '.' && std::isdigit(peekNext())) {
            isFloat = true;
            value += advance(); // consume '.'
            
            // Read fractional part
            while (!isAtEnd() && std::isdigit(peek())) {
                value += advance();
            }
        }
        
        // Check for exponent
        if (peek() == 'e' || peek() == 'E') {
            isFloat = true;
            value += advance(); // consume 'e' or 'E'
            
            if (peek() == '+' || peek() == '-') {
                value += advance();
            }
            
            if (!std::isdigit(peek())) {
                throw LexerError("Invalid number: expected digit after exponent", startLoc);
            }
            
            while (!isAtEnd() && std::isdigit(peek())) {
                value += advance();
            }
        }
        
        TokenType type = isFloat ? TokenType::FloatLiteral : TokenType::IntLiteral;
        return Token(type, value, startLoc);
    }
    
    /**
     * @brief Read an identifier or keyword
     */
    Token readIdentifier(SourceLocation startLoc) {
        std::string value;
        
        while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_')) {
            value += advance();
        }
        
        // Check if it's a keyword
        const auto& keywords = getKeywords();
        auto it = keywords.find(value);
        if (it != keywords.end()) {
            return Token(it->second, value, startLoc);
        }
        
        return Token(TokenType::Identifier, value, startLoc);
    }
    
    /**
     * @brief Read operator or delimiter
     */
    Token readOperatorOrDelimiter(SourceLocation startLoc) {
        char c = advance();
        
        switch (c) {
            // Single-character tokens
            case '(': return Token(TokenType::LParen, "(", startLoc);
            case ')': return Token(TokenType::RParen, ")", startLoc);
            case '[': return Token(TokenType::LBracket, "[", startLoc);
            case ']': return Token(TokenType::RBracket, "]", startLoc);
            case '{': return Token(TokenType::LBrace, "{", startLoc);
            case '}': return Token(TokenType::RBrace, "}", startLoc);
            case ',': return Token(TokenType::Comma, ",", startLoc);
            case ':': return Token(TokenType::Colon, ":", startLoc);
            case ';': return Token(TokenType::Semicolon, ";", startLoc);
            case '.': return Token(TokenType::Dot, ".", startLoc);
            case '+': return Token(TokenType::Plus, "+", startLoc);
            case '*': return Token(TokenType::Star, "*", startLoc);
            case '/': return Token(TokenType::Slash, "/", startLoc);
            case '%': return Token(TokenType::Percent, "%", startLoc);
            case '?': return Token(TokenType::Question, "?", startLoc);
            
            // Two-character tokens
            case '-':
                if (match('>')) return Token(TokenType::Arrow, "->", startLoc);
                return Token(TokenType::Minus, "-", startLoc);
                
            case '=':
                if (match('=')) return Token(TokenType::Equal, "==", startLoc);
                return Token(TokenType::Assign, "=", startLoc);
                
            case '!':
                if (match('=')) return Token(TokenType::NotEqual, "!=", startLoc);
                throw LexerError("Unexpected character '!'. Did you mean '!='?", startLoc);
                
            case '<':
                if (match('=')) return Token(TokenType::LessEqual, "<=", startLoc);
                return Token(TokenType::Less, "<", startLoc);
                
            case '>':
                if (match('=')) return Token(TokenType::GreaterEqual, ">=", startLoc);
                return Token(TokenType::Greater, ">", startLoc);
            
            // String literal
            case '"':
                return readString(startLoc);
                
            default:
                throw LexerError("Unexpected character '" + std::string(1, c) + "'", startLoc);
        }
    }
    
    /**
     * @brief Read a string literal
     */
    Token readString(SourceLocation startLoc) {
        std::string value;
        
        while (!isAtEnd() && peek() != '"') {
            if (peek() == '\n') {
                throw LexerError("Unterminated string literal", startLoc);
            }
            
            if (peek() == '\\') {
                advance(); // consume backslash
                if (isAtEnd()) {
                    throw LexerError("Unterminated string literal", startLoc);
                }
                
                char escaped = advance();
                switch (escaped) {
                    case 'n': value += '\n'; break;
                    case 't': value += '\t'; break;
                    case 'r': value += '\r'; break;
                    case '\\': value += '\\'; break;
                    case '"': value += '"'; break;
                    default:
                        throw LexerError("Invalid escape sequence '\\" + std::string(1, escaped) + "'", startLoc);
                }
            } else {
                value += advance();
            }
        }
        
        if (isAtEnd()) {
            throw LexerError("Unterminated string literal", startLoc);
        }
        
        advance(); // consume closing quote
        
        return Token(TokenType::StringLiteral, value, startLoc);
    }
};

} // namespace Rhodesia

#endif // RHODESIA_LEXER_HPP
