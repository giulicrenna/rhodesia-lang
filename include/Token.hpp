/**
 * @file Token.hpp
 * @brief Token definitions for Rhodesia lexer
 */

#ifndef RHODESIA_TOKEN_HPP
#define RHODESIA_TOKEN_HPP

#include <string>
#include <unordered_map>

namespace Rhodesia {

/**
 * @brief Token types for Rhodesia language
 */
enum class TokenType {
    // Literals
    IntLiteral,      // 42
    FloatLiteral,    // 3.14
    StringLiteral,   // "hello"
    
    // Identifiers & Keywords
    Identifier,      // variable/function names
    
    // Type keywords
    KwInt,           // int
    KwInt8,          // int8
    KwInt16,         // int16
    KwInt32,         // int32
    KwUInt8,         // uint8
    KwUInt16,        // uint16
    KwUInt32,        // uint32
    KwUInt64,        // uint64
    KwByte,          // byte
    KwFloat64,       // float64
    KwComplex,       // complex
    KwBool,          // bool
    KwVec,           // vec
    KwMat,           // mat
    KwStr,           // str
    KwMap,           // map
    KwArr,           // arr
    KwSet,           // set
    KwTuple,         // tuple
    KwRecord,        // record
    KwEnum,          // enum
    KwFunction,      // function
    KwVoid,          // void
    KwNull,          // null
    
    // Control flow keywords
    KwFun,           // fun
    KwLambda,        // lambda (or fn for short lambda syntax)
    KwReturn,        // return
    KwFor,           // for
    KwIn,            // in
    KwIf,            // if
    KwElse,          // else
    KwWhile,         // while
    KwBreak,         // break
    KwContinue,      // continue
    KwUsing,         // using
    KwTry,           // try
    KwCatch,         // catch
    KwThrow,         // throw
    KwMatch,         // match

    // Module system
    KwInclude,       // include
    KwAs,            // as

    // Logical keywords
    KwAnd,           // and
    KwOr,            // or
    KwNot,           // not
    KwTrue,          // true
    KwFalse,         // false
    
    // Operators - Arithmetic
    Plus,            // +
    Minus,           // -
    Star,            // *
    Slash,           // /
    Percent,         // %
    
    // Operators - Comparison
    Equal,           // ==
    NotEqual,        // !=
    Less,            // <
    Greater,         // >
    LessEqual,       // <=
    GreaterEqual,    // >=
    
    // Operators - Assignment
    Assign,          // =

    // Operators - Ternary
    Question,        // ?

    // Delimiters
    LParen,          // (
    RParen,          // )
    LBracket,        // [
    RBracket,        // ]
    LBrace,          // {
    RBrace,          // }
    Comma,           // ,
    Colon,           // :
    Semicolon,       // ;
    Arrow,           // ->
    Dot,             // .
    
    // Special
    Newline,         // for statement separation
    Eof,             // end of file
    Invalid          // error token
};

/**
 * @brief Convert TokenType to string (for debugging)
 */
inline std::string tokenTypeToString(TokenType type) {
    static const std::unordered_map<TokenType, std::string> names = {
        {TokenType::IntLiteral, "IntLiteral"},
        {TokenType::FloatLiteral, "FloatLiteral"},
        {TokenType::Identifier, "Identifier"},
        {TokenType::KwInt, "int"},
        {TokenType::KwInt8, "int8"},
        {TokenType::KwInt16, "int16"},
        {TokenType::KwInt32, "int32"},
        {TokenType::KwUInt8, "uint8"},
        {TokenType::KwUInt16, "uint16"},
        {TokenType::KwUInt32, "uint32"},
        {TokenType::KwUInt64, "uint64"},
        {TokenType::KwByte, "byte"},
        {TokenType::KwFloat64, "float64"},
        {TokenType::KwComplex, "complex"},
        {TokenType::KwBool, "bool"},
        {TokenType::KwVec, "vec"},
        {TokenType::KwMat, "mat"},
        {TokenType::KwStr, "str"},
        {TokenType::KwMap, "map"},
        {TokenType::KwArr, "arr"},
        {TokenType::KwSet, "set"},
        {TokenType::KwTuple, "tuple"},
        {TokenType::KwRecord, "record"},
        {TokenType::KwEnum, "enum"},
        {TokenType::KwFunction, "function"},
        {TokenType::KwVoid, "void"},
        {TokenType::KwNull, "null"},
        {TokenType::KwFun, "fun"},
        {TokenType::KwLambda, "lambda"},
        {TokenType::KwReturn, "return"},
        {TokenType::KwFor, "for"},
        {TokenType::KwIn, "in"},
        {TokenType::KwIf, "if"},
        {TokenType::KwElse, "else"},
        {TokenType::KwWhile, "while"},
        {TokenType::KwBreak, "break"},
        {TokenType::KwContinue, "continue"},
        {TokenType::KwUsing, "using"},
        {TokenType::KwTry, "try"},
        {TokenType::KwCatch, "catch"},
        {TokenType::KwThrow, "throw"},
        {TokenType::KwMatch, "match"},
        {TokenType::KwInclude, "include"},
        {TokenType::KwAs, "as"},
        {TokenType::KwAnd, "and"},
        {TokenType::KwOr, "or"},
        {TokenType::KwNot, "not"},
        {TokenType::KwTrue, "true"},
        {TokenType::KwFalse, "false"},
        {TokenType::StringLiteral, "String"},
        {TokenType::Plus, "+"},
        {TokenType::Minus, "-"},
        {TokenType::Star, "*"},
        {TokenType::Slash, "/"},
        {TokenType::Percent, "%"},
        {TokenType::Equal, "=="},
        {TokenType::NotEqual, "!="},
        {TokenType::Less, "<"},
        {TokenType::Greater, ">"},
        {TokenType::LessEqual, "<="},
        {TokenType::GreaterEqual, ">="},
        {TokenType::Assign, "="},
        {TokenType::Question, "?"},
        {TokenType::LParen, "("},
        {TokenType::RParen, ")"},
        {TokenType::LBracket, "["},
        {TokenType::RBracket, "]"},
        {TokenType::LBrace, "{"},
        {TokenType::RBrace, "}"},
        {TokenType::Comma, ","},
        {TokenType::Colon, ":"},
        {TokenType::Semicolon, ";"},
        {TokenType::Arrow, "->"},
        {TokenType::Dot, "."},
        {TokenType::Newline, "Newline"},
        {TokenType::Eof, "EOF"},
        {TokenType::Invalid, "Invalid"}
    };
    
    auto it = names.find(type);
    return it != names.end() ? it->second : "Unknown";
}

/**
 * @brief Source location information
 */
struct SourceLocation {
    size_t line = 1;
    size_t column = 1;
    size_t offset = 0;  // byte offset in source
    
    std::string toString() const {
        return "line " + std::to_string(line) + ", col " + std::to_string(column);
    }
};

/**
 * @brief Token structure containing type, value, and location
 */
struct Token {
    TokenType type;
    std::string value;
    SourceLocation location;
    
    Token() : type(TokenType::Invalid) {}
    
    Token(TokenType t, std::string v, SourceLocation loc)
        : type(t), value(std::move(v)), location(loc) {}
    
    bool is(TokenType t) const { return type == t; }
    
    bool isOneOf(std::initializer_list<TokenType> types) const {
        for (auto t : types) {
            if (type == t) return true;
        }
        return false;
    }
    
    bool isKeyword() const {
        return type >= TokenType::KwInt && type <= TokenType::KwFalse;
    }
    
    bool isTypeKeyword() const {
        return type >= TokenType::KwInt && type <= TokenType::KwNull;
    }
    
    bool isOperator() const {
        return type >= TokenType::Plus && type <= TokenType::GreaterEqual;
    }
    
    bool isComparisonOperator() const {
        return type >= TokenType::Equal && type <= TokenType::GreaterEqual;
    }
    
    bool isArithmeticOperator() const {
        return type >= TokenType::Plus && type <= TokenType::Percent;
    }
    
    std::string toString() const {
        return tokenTypeToString(type) + "('" + value + "') at " + location.toString();
    }
};

/**
 * @brief Keyword lookup table
 */
inline const std::unordered_map<std::string, TokenType>& getKeywords() {
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"int", TokenType::KwInt},
        {"int8", TokenType::KwInt8},
        {"int16", TokenType::KwInt16},
        {"int32", TokenType::KwInt32},
        {"uint8", TokenType::KwUInt8},
        {"uint16", TokenType::KwUInt16},
        {"uint32", TokenType::KwUInt32},
        {"uint64", TokenType::KwUInt64},
        {"byte", TokenType::KwByte},
        {"float64", TokenType::KwFloat64},
        {"complex", TokenType::KwComplex},
        {"bool", TokenType::KwBool},
        {"vec", TokenType::KwVec},
        {"mat", TokenType::KwMat},
        {"str", TokenType::KwStr},
        {"map", TokenType::KwMap},
        {"arr", TokenType::KwArr},
        {"set", TokenType::KwSet},
        {"tuple", TokenType::KwTuple},
        {"record", TokenType::KwRecord},
        {"enum", TokenType::KwEnum},
        {"function", TokenType::KwFunction},
        {"void", TokenType::KwVoid},
        {"null", TokenType::KwNull},
        {"fun", TokenType::KwFun},
        {"lambda", TokenType::KwLambda},
        {"fn", TokenType::KwLambda},  // Short alias for lambda
        {"return", TokenType::KwReturn},
        {"for", TokenType::KwFor},
        {"in", TokenType::KwIn},
        {"if", TokenType::KwIf},
        {"else", TokenType::KwElse},
        {"while", TokenType::KwWhile},
        {"break", TokenType::KwBreak},
        {"continue", TokenType::KwContinue},
        {"using", TokenType::KwUsing},
        {"try", TokenType::KwTry},
        {"catch", TokenType::KwCatch},
        {"throw", TokenType::KwThrow},
        {"match", TokenType::KwMatch},
        {"include", TokenType::KwInclude},
        {"as", TokenType::KwAs},
        {"and", TokenType::KwAnd},
        {"or", TokenType::KwOr},
        {"not", TokenType::KwNot},
        {"true", TokenType::KwTrue},
        {"false", TokenType::KwFalse}
    };
    return keywords;
}

} // namespace Rhodesia

#endif // RHODESIA_TOKEN_HPP
