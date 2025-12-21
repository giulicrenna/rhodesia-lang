/**
 * @file Error.hpp
 * @brief Error handling for Rhodesia language
 */

#ifndef RHODESIA_ERROR_HPP
#define RHODESIA_ERROR_HPP

#include "Token.hpp"
#include "RhoValue.hpp"
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>

namespace Rhodesia {

/**
 * @brief Base class for all Rhodesia errors
 */
class RhoError : public std::runtime_error {
public:
    RhoError(const std::string& msg, SourceLocation loc = {})
        : std::runtime_error(formatError(msg, loc)), location_(loc) {}
    
    const SourceLocation& location() const { return location_; }

protected:
    SourceLocation location_;
    
    static std::string formatError(const std::string& msg, const SourceLocation& loc) {
        if (loc.line == 0) return msg;
        return "[" + loc.toString() + "] " + msg;
    }
};

/**
 * @brief Lexer-specific errors
 */
class LexerError : public RhoError {
public:
    LexerError(const std::string& msg, SourceLocation loc = {})
        : RhoError("Lexer error: " + msg, loc) {}
};

/**
 * @brief Parser-specific errors
 */
class ParseError : public RhoError {
public:
    ParseError(const std::string& msg, SourceLocation loc = {})
        : RhoError("Parse error: " + msg, loc) {}

    ParseError(const std::string& msg, const Token& token)
        : RhoError("Parse error: " + msg + " (got '" + token.value + "')", token.location) {}

    static ParseError unexpectedToken(const Token& got, const std::string& expected) {
        return ParseError("Expected " + expected + ", got " + tokenTypeToString(got.type), got);
    }

    static ParseError unexpectedEof(const std::string& context) {
        return ParseError("Unexpected end of file " + context);
    }

    static ParseError invalidVariableDeclaration(const std::string& reason, SourceLocation loc = {}) {
        return ParseError("Invalid variable declaration: " + reason, loc);
    }

    static ParseError invalidFunctionDeclaration(const std::string& reason, SourceLocation loc = {}) {
        return ParseError("Invalid function declaration: " + reason, loc);
    }

    static ParseError invalidExpression(const std::string& context, SourceLocation loc = {}) {
        return ParseError("Invalid expression in " + context, loc);
    }

    static ParseError invalidAssignment(const std::string& reason, SourceLocation loc = {}) {
        return ParseError("Invalid assignment: " + reason, loc);
    }

    static ParseError missingDelimiter(const std::string& delimiter, const std::string& context, SourceLocation loc = {}) {
        return ParseError("Missing '" + delimiter + "' " + context, loc);
    }

    static ParseError unexpectedDelimiter(const std::string& delimiter, const std::string& context, SourceLocation loc = {}) {
        return ParseError("Unexpected '" + delimiter + "' " + context, loc);
    }
};

/**
 * @brief Type-checking errors
 */
class TypeError : public RhoError {
public:
    TypeError(const std::string& msg, SourceLocation loc = {})
        : RhoError("Type error: " + msg, loc) {}

    static TypeError incompatibleBinaryOp(const std::string& op, RhoType left, RhoType right) {
        return TypeError("Cannot apply '" + op + "' to " +
                        typeToString(left) + " and " + typeToString(right));
    }

    static TypeError incompatibleUnaryOp(const std::string& op, RhoType type) {
        return TypeError("Cannot apply '" + op + "' to " + typeToString(type));
    }

    static TypeError expectedType(const std::string& context, RhoType expected, RhoType got) {
        return TypeError(context + ": expected " + typeToString(expected) +
                        ", got " + typeToString(got));
    }
};

/**
 * @brief Memory allocation errors
 */
class MemoryError : public RhoError {
public:
    MemoryError(const std::string& msg, SourceLocation loc = {})
        : RhoError("Memory error: " + msg, loc) {}

    static MemoryError allocationFailed(const std::string& operation, size_t requestedSize) {
        std::string sizeStr;
        if (requestedSize > 1e12) {
            sizeStr = std::to_string(requestedSize / 1e12) + " trillion";
        } else if (requestedSize > 1e9) {
            sizeStr = std::to_string(requestedSize / 1e9) + " billion";
        } else if (requestedSize > 1e6) {
            sizeStr = std::to_string(requestedSize / 1e6) + " million";
        } else if (requestedSize > 1e3) {
            sizeStr = std::to_string(requestedSize / 1e3) + " thousand";
        } else {
            sizeStr = std::to_string(requestedSize);
        }

        return MemoryError("Failed to allocate memory for " + operation +
                          " of size " + sizeStr +
                          ". Try using smaller dimensions or check available memory.");
    }

    static MemoryError sizeTooLarge(const std::string& operation, size_t size, size_t maxRecommended = 100000000) {
        return MemoryError(operation + " size " + std::to_string(size) +
                          " is too large. Maximum recommended size is " +
                          std::to_string(maxRecommended) + " elements.");
    }
};

/**
 * @brief Runtime evaluation errors
 */
class RuntimeError : public RhoError {
public:
    RuntimeError(const std::string& msg, SourceLocation loc = {})
        : RhoError("Runtime error: " + msg, loc) {}

    static RuntimeError undefinedVariable(const std::string& name, SourceLocation loc = {}) {
        return RuntimeError("Undefined variable '" + name + "'. Did you mean to declare it first?", loc);
    }

    static RuntimeError undefinedFunction(const std::string& name, SourceLocation loc = {}) {
        return RuntimeError("Undefined function '" + name + "'. Check function name spelling or define it.", loc);
    }

    static RuntimeError divisionByZero(SourceLocation loc = {}) {
        return RuntimeError("Division by zero. Check your divisor before dividing.", loc);
    }

    static RuntimeError indexOutOfBounds(size_t index, size_t size, SourceLocation loc = {}) {
        return RuntimeError("Index " + std::to_string(index) +
                          " out of bounds for size " + std::to_string(size) +
                          ". Valid indices are 0 to " + std::to_string(size - 1), loc);
    }

    static RuntimeError singularMatrix(SourceLocation loc = {}) {
        return RuntimeError("Cannot invert singular matrix (determinant is zero). "
                          "Check matrix condition or use a different matrix.", loc);
    }

    static RuntimeError dimensionMismatch(const std::string& op,
                                          const std::string& dims1,
                                          const std::string& dims2,
                                          SourceLocation loc = {}) {
        return RuntimeError("Dimension mismatch in " + op + ": " + dims1 + " vs " + dims2 +
                          ". Ensure operands have compatible dimensions.", loc);
    }

    static RuntimeError invalidForLoopIterable(const std::string& typeName, SourceLocation loc = {}) {
        return RuntimeError("For loop requires a vector to iterate over, got " + typeName +
                          ". Use a vector or range() function.", loc);
    }

    static RuntimeError breakOutsideLoop(SourceLocation loc = {}) {
        return RuntimeError("Break statement outside of loop. Break can only be used inside for or while loops.", loc);
    }

    static RuntimeError continueOutsideLoop(SourceLocation loc = {}) {
        return RuntimeError("Continue statement outside of loop. Continue can only be used inside for or while loops.", loc);
    }

    static RuntimeError matrixNotSquare(size_t rows, size_t cols, SourceLocation loc = {}) {
        return RuntimeError("Matrix must be square for this operation. Got " +
                          std::to_string(rows) + "x" + std::to_string(cols) +
                          ". Ensure rows equal columns.", loc);
    }

    static RuntimeError invalidMatrixIndex(size_t row, size_t col, size_t maxRow, size_t maxCol, SourceLocation loc = {}) {
        return RuntimeError("Matrix index [" + std::to_string(row) + "," + std::to_string(col) +
                          "] out of bounds for " + std::to_string(maxRow) + "x" + std::to_string(maxCol) +
                          " matrix. Valid indices: [0.." + std::to_string(maxRow-1) + ", 0.." +
                          std::to_string(maxCol-1) + "]", loc);
    }
};

/**
 * @brief Argument validation error for built-in functions
 */
class ArgumentError : public RuntimeError {
public:
    ArgumentError(const std::string& func, const std::string& msg, SourceLocation loc = {})
        : RuntimeError(func + "(): " + msg, loc) {}
    
    static ArgumentError wrongCount(const std::string& func, size_t expected, size_t got, SourceLocation loc = {}) {
        return ArgumentError(func, "expected " + std::to_string(expected) + 
                            " arguments, got " + std::to_string(got), loc);
    }
    
    static ArgumentError wrongType(const std::string& func, size_t argNum, 
                                   const std::string& expected, const std::string& got,
                                   SourceLocation loc = {}) {
        return ArgumentError(func, "argument " + std::to_string(argNum) + 
                            " must be " + expected + ", got " + got, loc);
    }
};

// ReturnValue is defined in Evaluator.hpp to avoid circular dependency

/**
 * @brief Control flow exception for break statements
 */
class BreakException : public std::exception {};

/**
 * @brief Control flow exception for continue statements
 */
class ContinueException : public std::exception {};

/**
 * @brief Error reporter for collecting multiple errors
 */
class ErrorReporter {
public:
    void report(const RhoError& error) {
        errors_.push_back(error.what());
    }
    
    void report(const std::string& msg, SourceLocation loc = {}) {
        errors_.push_back("[" + loc.toString() + "] " + msg);
    }
    
    bool hasErrors() const { return !errors_.empty(); }
    
    const std::vector<std::string>& errors() const { return errors_; }
    
    void clear() { errors_.clear(); }
    
    std::string summary() const {
        std::ostringstream oss;
        oss << errors_.size() << " error(s):\n";
        for (const auto& err : errors_) {
            oss << "  " << err << "\n";
        }
        return oss.str();
    }

private:
    std::vector<std::string> errors_;
};

} // namespace Rhodesia

#endif // RHODESIA_ERROR_HPP
