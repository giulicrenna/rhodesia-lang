/**
 * @file RhoValue.hpp
 * @brief Core value types for Rhodesia language
 * 
 * Defines the fundamental value type (RhoValue) as a std::variant
 * that can hold scalars, vectors, and matrices using Eigen types.
 */

#ifndef RHODESIA_RHOVALUE_HPP
#define RHODESIA_RHOVALUE_HPP

// ponytail: Eigen's SIMD alignment is unsafe when Eigen::VectorXd lives
// inside std::variant<RhoValue> — std::variant is not aware of Eigen's
// 16-byte alignment requirement, so allocations (especially on the stack)
// can be misaligned, producing intermittent segfaults in back-to-back
// stats calls. Disabling vectorisation costs a little perf but removes
// the UB. Define BEFORE any Eigen include.
#ifndef EIGEN_DONT_ALIGN
  #define EIGEN_DONT_ALIGN
#endif
#ifndef EIGEN_DONT_ALIGN_STATICALLY
  #define EIGEN_DONT_ALIGN_STATICALLY
#endif

#include <variant>
#include <string>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <Eigen/Dense>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>

namespace Rhodesia {

/**
 * @brief Enumeration of Rhodesia's type system
 */
enum class RhoType {
    Int,        // int64 (default)
    Int8,
    Int16,
    Int32,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    Byte,       // alias for uint8
    Float64,
    Complex,
    Bool,
    Vec,
    Mat,
    String,
    Map,
    Arr,
    Set,
    Tuple,
    Record,
    Enum,
    Function,
    Void,
    Null,
    Unknown
};

/**
 * @brief Convert RhoType to string representation
 */
inline std::string typeToString(RhoType type) {
    switch (type) {
        case RhoType::Int:      return "int";
        case RhoType::Int8:     return "int8";
        case RhoType::Int16:    return "int16";
        case RhoType::Int32:    return "int32";
        case RhoType::UInt8:    return "uint8";
        case RhoType::UInt16:   return "uint16";
        case RhoType::UInt32:   return "uint32";
        case RhoType::UInt64:   return "uint64";
        case RhoType::Byte:     return "byte";
        case RhoType::Float64:  return "float64";
        case RhoType::Complex:  return "complex";
        case RhoType::Bool:     return "bool";
        case RhoType::Vec:      return "vec";
        case RhoType::Mat:      return "mat";
        case RhoType::String:   return "string";
        case RhoType::Map:      return "map";
        case RhoType::Arr:      return "arr";
        case RhoType::Set:      return "set";
        case RhoType::Tuple:    return "tuple";
        case RhoType::Record:   return "record";
        case RhoType::Enum:     return "enum";
        case RhoType::Function: return "function";
        case RhoType::Void:     return "void";
        case RhoType::Null:     return "null";
        default:                return "unknown";
    }
}

/**
 * @brief Parse string to RhoType
 */
inline RhoType stringToType(const std::string& str) {
    if (str == "int")      return RhoType::Int;
    if (str == "int8")     return RhoType::Int8;
    if (str == "int16")    return RhoType::Int16;
    if (str == "int32")    return RhoType::Int32;
    if (str == "uint8")    return RhoType::UInt8;
    if (str == "uint16")   return RhoType::UInt16;
    if (str == "uint32")   return RhoType::UInt32;
    if (str == "uint64")   return RhoType::UInt64;
    if (str == "byte")     return RhoType::Byte;
    if (str == "float64")  return RhoType::Float64;
    if (str == "complex")  return RhoType::Complex;
    if (str == "bool")     return RhoType::Bool;
    if (str == "vec")      return RhoType::Vec;
    if (str == "mat")      return RhoType::Mat;
    if (str == "string")   return RhoType::String;
    if (str == "map")      return RhoType::Map;
    if (str == "arr")      return RhoType::Arr;
    if (str == "set")      return RhoType::Set;
    if (str == "tuple")    return RhoType::Tuple;
    if (str == "record")   return RhoType::Record;
    if (str == "enum")     return RhoType::Enum;
    if (str == "function") return RhoType::Function;
    if (str == "void")     return RhoType::Void;
    if (str == "null")     return RhoType::Null;
    return RhoType::Unknown;
}

// Forward declarations
class RangeGenerator;
class RhoMap;
class RhoArray;
class RhoFunction;
class RhoSet;
class RhoTuple;
class RhoRecord;
class RhoEnum;
class RhoComplex;
class RhoNull;

/**
 * @brief The core value type for Rhodesia
 *
 * RhoValue can hold:
 * - int64_t (64-bit integer, default int)
 * - int8_t, int16_t, int32_t (smaller integers)
 * - uint8_t, uint16_t, uint32_t, uint64_t (unsigned integers)
 * - double (float64)
 * - bool (boolean value)
 * - std::string (text)
 * - Eigen::VectorXd (dynamic-size vector)
 * - Eigen::MatrixXd (dynamic-size matrix)
 * - std::shared_ptr<RhoComplex> (complex numbers)
 * - std::shared_ptr<RangeGenerator> (lazy range generator)
 * - std::shared_ptr<RhoMap> (hash map container)
 * - std::shared_ptr<RhoArray> (heterogeneous array)
 * - std::shared_ptr<RhoSet> (unordered set)
 * - std::shared_ptr<RhoTuple> (immutable tuple)
 * - std::shared_ptr<RhoRecord> (named fields structure)
 * - std::shared_ptr<RhoEnum> (enumeration)
 * - std::shared_ptr<RhoFunction> (function/lambda with closures)
 * - std::shared_ptr<RhoNull> (null value)
 */
using RhoValue = std::variant<
    int64_t,
    int8_t,
    int16_t,
    int32_t,
    uint8_t,
    uint16_t,
    uint32_t,
    uint64_t,
    double,
    bool,
    std::string,
    Eigen::VectorXd,
    Eigen::MatrixXd,
    std::shared_ptr<RhoComplex>,
    std::shared_ptr<RangeGenerator>,
    std::shared_ptr<RhoMap>,
    std::shared_ptr<RhoArray>,
    std::shared_ptr<RhoSet>,
    std::shared_ptr<RhoTuple>,
    std::shared_ptr<RhoRecord>,
    std::shared_ptr<RhoEnum>,
    std::shared_ptr<RhoFunction>,
    std::shared_ptr<RhoNull>
>;

/**
 * @brief Get the RhoType of a RhoValue
 */
inline RhoType getValueType(const RhoValue& value) {
    return std::visit([](auto&& arg) -> RhoType {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int64_t>)
            return RhoType::Int;
        else if constexpr (std::is_same_v<T, int8_t>)
            return RhoType::Int8;
        else if constexpr (std::is_same_v<T, int16_t>)
            return RhoType::Int16;
        else if constexpr (std::is_same_v<T, int32_t>)
            return RhoType::Int32;
        else if constexpr (std::is_same_v<T, uint8_t>)
            return RhoType::UInt8;
        else if constexpr (std::is_same_v<T, uint16_t>)
            return RhoType::UInt16;
        else if constexpr (std::is_same_v<T, uint32_t>)
            return RhoType::UInt32;
        else if constexpr (std::is_same_v<T, uint64_t>)
            return RhoType::UInt64;
        else if constexpr (std::is_same_v<T, double>)
            return RhoType::Float64;
        else if constexpr (std::is_same_v<T, bool>)
            return RhoType::Bool;
        else if constexpr (std::is_same_v<T, std::string>)
            return RhoType::String;
        else if constexpr (std::is_same_v<T, Eigen::VectorXd>)
            return RhoType::Vec;
        else if constexpr (std::is_same_v<T, Eigen::MatrixXd>)
            return RhoType::Mat;
        else if constexpr (std::is_same_v<T, std::shared_ptr<RhoComplex>>)
            return RhoType::Complex;
        else if constexpr (std::is_same_v<T, std::shared_ptr<RhoMap>>)
            return RhoType::Map;
        else if constexpr (std::is_same_v<T, std::shared_ptr<RhoArray>>)
            return RhoType::Arr;
        else if constexpr (std::is_same_v<T, std::shared_ptr<RhoSet>>)
            return RhoType::Set;
        else if constexpr (std::is_same_v<T, std::shared_ptr<RhoTuple>>)
            return RhoType::Tuple;
        else if constexpr (std::is_same_v<T, std::shared_ptr<RhoRecord>>)
            return RhoType::Record;
        else if constexpr (std::is_same_v<T, std::shared_ptr<RhoEnum>>)
            return RhoType::Enum;
        else if constexpr (std::is_same_v<T, std::shared_ptr<RhoFunction>>)
            return RhoType::Function;
        else if constexpr (std::is_same_v<T, std::shared_ptr<RhoNull>>)
            return RhoType::Null;
        else
            return RhoType::Unknown;
    }, value);
}

/**
 * @brief Convert RhoValue to double (for numeric operations)
 * @throws std::runtime_error if value is not numeric scalar
 */
inline double toDouble(const RhoValue& value) {
    return std::visit([](auto&& arg) -> double {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, int8_t> ||
                      std::is_same_v<T, int16_t> || std::is_same_v<T, int32_t>)
            return static_cast<double>(arg);
        else if constexpr (std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> ||
                           std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>)
            return static_cast<double>(arg);
        else if constexpr (std::is_same_v<T, double>)
            return arg;
        else
            throw std::runtime_error("Cannot convert non-scalar to double");
    }, value);
}

/**
 * @brief Convert RhoValue to int64_t
 * @throws std::runtime_error if value is not numeric scalar
 */
inline int64_t toInt(const RhoValue& value) {
    return std::visit([](auto&& arg) -> int64_t {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, int8_t> ||
                      std::is_same_v<T, int16_t> || std::is_same_v<T, int32_t>)
            return static_cast<int64_t>(arg);
        else if constexpr (std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> ||
                           std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>)
            return static_cast<int64_t>(arg);
        else if constexpr (std::is_same_v<T, double>)
            return static_cast<int64_t>(arg);
        else
            throw std::runtime_error("Cannot convert non-scalar to int");
    }, value);
}

/**
 * @brief Check if value is a scalar (int, double, or bool)
 */
inline bool isScalar(const RhoValue& value) {
    return std::holds_alternative<int64_t>(value) ||
        std::holds_alternative<int8_t>(value) ||
        std::holds_alternative<int16_t>(value) ||
        std::holds_alternative<int32_t>(value) ||
        std::holds_alternative<uint8_t>(value) ||
        std::holds_alternative<uint16_t>(value) ||
        std::holds_alternative<uint32_t>(value) ||
        std::holds_alternative<uint64_t>(value) ||
        std::holds_alternative<double>(value) ||
        std::holds_alternative<bool>(value);
}

/**
 * @brief Check if value is numeric (can participate in arithmetic)
 */
inline bool isNumeric(const RhoValue& value) {
    return !std::holds_alternative<std::string>(value);
}

/**
 * @brief Format RhoValue as string for display
 */
inline std::string valueToString(const RhoValue& value, int precision = 6) {
    std::ostringstream oss;
    oss << std::setprecision(precision);

    std::visit([&oss, precision](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, int32_t> ||
                      std::is_same_v<T, int16_t> || std::is_same_v<T, int8_t>) {
            oss << arg;
        }
        else if constexpr (std::is_same_v<T, uint64_t> || std::is_same_v<T, uint32_t> ||
                           std::is_same_v<T, uint16_t> || std::is_same_v<T, uint8_t>) {
            oss << arg;
        }
        else if constexpr (std::is_same_v<T, double>) {
            oss << arg;
        }
        else if constexpr (std::is_same_v<T, bool>) {
            oss << (arg ? "true" : "false");
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            oss << arg;
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<RhoComplex>>) {
            oss << arg->toString();
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<RhoNull>>) {
            oss << "null";
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<RangeGenerator>>) {
            oss << "range(" << arg->current() << ".." << (arg->current() + static_cast<int64_t>(arg->size()) - 1) << ")";
        }
        else if constexpr (std::is_same_v<T, Eigen::VectorXd>) {
            oss << "vec[" << arg.size() << "](";
            for (Eigen::Index i = 0; i < arg.size(); ++i) {
                if (i > 0) oss << ", ";
                if (i >= 10 && arg.size() > 15) {
                    oss << "...";
                    break;
                }
                oss << arg(i);
            }
            oss << ")";
        }
        else if constexpr (std::is_same_v<T, Eigen::MatrixXd>) {
            oss << "mat[" << arg.rows() << "x" << arg.cols() << "](\n";
            for (Eigen::Index i = 0; i < arg.rows() && i < 10; ++i) {
                oss << "  [";
                for (Eigen::Index j = 0; j < arg.cols() && j < 10; ++j) {
                    if (j > 0) oss << ", ";
                    oss << arg(i, j);
                }
                if (arg.cols() > 10) oss << ", ...";
                oss << "]";
                if (i < arg.rows() - 1) oss << ",";
                oss << "\n";
            }
            if (arg.rows() > 10) oss << "  ...\n";
            oss << ")";
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<RhoMap>>) {
            oss << "map{";
            size_t count = 0;
            for (auto it = arg->begin(); it != arg->end(); ++it) {
                if (count > 0) oss << ", ";
                if (count >= 10) {
                    oss << "...";
                    break;
                }
                oss << "\"" << it->first << "\": " << valueToString(it->second, precision);
                count++;
            }
            oss << "}";
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<RhoArray>>) {
            oss << "arr[" << arg->size() << "](";
            for (size_t i = 0; i < arg->size(); ++i) {
                if (i > 0) oss << ", ";
                if (i >= 10 && arg->size() > 15) {
                    oss << "...";
                    break;
                }
                oss << valueToString(arg->get(i), precision);
            }
            oss << ")";
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<RhoSet>>) {
            oss << "set{";
            size_t count = 0;
            for (const auto& elem : *arg) {
                if (count > 0) oss << ", ";
                if (count >= 10) {
                    oss << "...";
                    break;
                }
                oss << valueToString(elem, precision);
                count++;
            }
            oss << "}";
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<RhoTuple>>) {
            oss << "tuple(";
            for (size_t i = 0; i < arg->size(); ++i) {
                if (i > 0) oss << ", ";
                oss << valueToString(arg->get(i), precision);
            }
            oss << ")";
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<RhoRecord>>) {
            oss << "record{";
            size_t count = 0;
            for (const auto& [name, val] : arg->fields()) {
                if (count > 0) oss << ", ";
                oss << name << ": " << valueToString(val, precision);
                count++;
            }
            oss << "}";
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<RhoEnum>>) {
            oss << arg->name() << "::" << arg->value();
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<RhoFunction>>) {
            oss << "<function/" << arg->arity() << ">";
        }
    }, value);

    return oss.str();
}



/**
 * @brief RhoMap - Hash map container for key-value pairs
 *
 * This class provides a dictionary/map data structure similar to Python's dict.
 * Keys are strings, values can be any RhoValue type.
 */
class RhoMap {
public:
    using MapType = std::unordered_map<std::string, RhoValue>;

    RhoMap() = default;

    /**
     * @brief Set a key-value pair in the map
     */
    void set(const std::string& key, const RhoValue& value) {
        data_[key] = value;
    }

    /**
     * @brief Get a value by key
     * @throws std::runtime_error if key doesn't exist
     */
    RhoValue get(const std::string& key) const {
        auto it = data_.find(key);
        if (it == data_.end()) {
            throw std::runtime_error("Key '" + key + "' not found in map");
        }
        return it->second;
    }

    /**
     * @brief Check if a key exists in the map
     */
    bool has(const std::string& key) const {
        return data_.find(key) != data_.end();
    }

    /**
     * @brief Remove a key from the map
     * @return true if key was found and removed, false otherwise
     */
    bool remove(const std::string& key) {
        return data_.erase(key) > 0;
    }

    /**
     * @brief Get the number of key-value pairs
     */
    size_t size() const {
        return data_.size();
    }

    /**
     * @brief Check if map is empty
     */
    bool empty() const {
        return data_.empty();
    }

    /**
     * @brief Clear all key-value pairs
     */
    void clear() {
        data_.clear();
    }

    /**
     * @brief Get all keys
     */
    std::vector<std::string> keys() const {
        std::vector<std::string> result;
        result.reserve(data_.size());
        for (const auto& pair : data_) {
            result.push_back(pair.first);
        }
        return result;
    }

    /**
     * @brief Get all values
     */
    std::vector<RhoValue> values() const {
        std::vector<RhoValue> result;
        result.reserve(data_.size());
        for (const auto& pair : data_) {
            result.push_back(pair.second);
        }
        return result;
    }

    /**
     * @brief Get iterator to beginning
     */
    MapType::const_iterator begin() const { return data_.begin(); }
    MapType::const_iterator end() const { return data_.end(); }

private:
    MapType data_;
};

/**
 * @brief RhoArray - Heterogeneous array container
 *
 * This class provides a dynamic array data structure that can hold
 * any type of RhoValue. Similar to RhoMap but indexed by integers.
 */
class RhoArray {
public:
    using ArrayType = std::vector<RhoValue>;

    RhoArray() = default;

    /**
     * @brief Create array with initial size
     */
    explicit RhoArray(size_t size) : data_(size) {}

    /**
     * @brief Create array from initializer list
     */
    RhoArray(std::initializer_list<RhoValue> values) : data_(values) {}

    /**
     * @brief Set value at index
     * @throws std::out_of_range if index is invalid
     */
    void set(size_t index, const RhoValue& value) {
        if (index >= data_.size()) {
            throw std::out_of_range("Array index " + std::to_string(index) + " out of range (size: " + std::to_string(data_.size()) + ")");
        }
        data_[index] = value;
    }

    /**
     * @brief Get value at index
     * @throws std::out_of_range if index is invalid
     */
    RhoValue get(size_t index) const {
        if (index >= data_.size()) {
            throw std::out_of_range("Array index " + std::to_string(index) + " out of range (size: " + std::to_string(data_.size()) + ")");
        }
        return data_[index];
    }

    /**
     * @brief Append value to end of array
     */
    void push(const RhoValue& value) {
        data_.push_back(value);
    }

    /**
     * @brief Remove and return last element
     * @throws std::runtime_error if array is empty
     */
    RhoValue pop() {
        if (data_.empty()) {
            throw std::runtime_error("Cannot pop from empty array");
        }
        RhoValue value = data_.back();
        data_.pop_back();
        return value;
    }

    /**
     * @brief Get the number of elements
     */
    size_t size() const {
        return data_.size();
    }

    /**
     * @brief Check if array is empty
     */
    bool empty() const {
        return data_.empty();
    }

    /**
     * @brief Clear all elements
     */
    void clear() {
        data_.clear();
    }

    /**
     * @brief Resize array
     */
    void resize(size_t newSize) {
        data_.resize(newSize);
    }

    /**
     * @brief Get iterator to beginning
     */
    ArrayType::const_iterator begin() const { return data_.begin(); }
    ArrayType::const_iterator end() const { return data_.end(); }
    ArrayType::iterator begin() { return data_.begin(); }
    ArrayType::iterator end() { return data_.end(); }

private:
    ArrayType data_;
};

/**
 * @brief RhoFunction - Function value with closure support
 *
 * This class represents a function as a first-class value, supporting:
 * - Lambda expressions (anonymous functions)
 * - Closures (capturing variables from enclosing scope)
 * - Higher-order functions (functions that take/return functions)
 */
class RhoFunction {
public:
    using NativeFunc = std::function<RhoValue(const std::vector<RhoValue>&)>;

    /**
     * @brief Construct a user-defined lambda function with closure
     * @param params Parameter names
     * @param bodyNode Lambda body AST node (shared ownership)
     * @param isExpr True if this is an expression lambda
     * @param closure Captured variables
     */
    RhoFunction(std::vector<std::string> params,
                std::shared_ptr<void> bodyNode,
                bool isExpr,
                std::unordered_map<std::string, RhoValue> closure = {},
                std::vector<RhoType> paramTypes = {})
        : params_(std::move(params)),
          bodyNode_(std::move(bodyNode)),
          isExpression_(isExpr),
          closure_(std::move(closure)),
          paramTypes_(std::move(paramTypes)),
          isNative_(false) {}

    /**
     * @brief Construct a native C++ function
     */
    explicit RhoFunction(NativeFunc func)
        : nativeFunc_(std::move(func)), isNative_(true), isExpression_(false) {}

    /**
     * @brief Get parameter names
     */
    const std::vector<std::string>& params() const { return params_; }

    /**
     * @brief Get parameter types (may be empty for native or inferred lambdas)
     */
    const std::vector<RhoType>& paramTypes() const { return paramTypes_; }

    /**
     * @brief Get function body AST node
     */
    std::shared_ptr<void> body() const { return bodyNode_; }

    /**
     * @brief Check if this is an expression lambda
     */
    bool isExpression() const { return isExpression_; }

    /**
     * @brief Get closure environment
     */
    const std::unordered_map<std::string, RhoValue>& closure() const { return closure_; }

    /**
     * @brief Check if this is a native function
     */
    bool isNative() const { return isNative_; }

    /**
     * @brief Call native function
     */
    RhoValue callNative(const std::vector<RhoValue>& args) const {
        if (!isNative_) {
            throw std::runtime_error("Cannot call non-native function with callNative");
        }
        return nativeFunc_(args);
    }

    /**
     * @brief Get number of parameters
     */
    size_t arity() const { return params_.size(); }

private:
    std::vector<std::string> params_;  // Parameter names
    std::shared_ptr<void> bodyNode_;   // Lambda body AST node (ExprNode* or BlockNode*)
    bool isExpression_;                // True for expression lambdas (x -> x*2), false for block lambdas
    std::unordered_map<std::string, RhoValue> closure_;  // Captured variables
    std::vector<RhoType> paramTypes_;  // Declared parameter types (empty if unknown)
    NativeFunc nativeFunc_;            // Native C++ function
    bool isNative_;                    // Is this a native function?
};

/**
 * @brief RangeGenerator - Lazy range generator for efficient iteration
 *
 * This class generates values on-demand instead of creating a full vector,
 * significantly reducing memory usage and improving performance for large ranges.
 */
class RangeGenerator {
public:
    RangeGenerator(int64_t start, int64_t end)
        : start_(start), end_(end), current_(start) {}

    /**
     * @brief Check if there are more elements in the range
     */
    bool hasNext() const {
        return current_ < end_;
    }

    /**
     * @brief Get the next value in the range
     */
    double next() {
        if (current_ >= end_) {
            throw std::runtime_error("RangeGenerator: no more elements");
        }
        return static_cast<double>(current_++);
    }

    /**
     * @brief Get the size of the range
     */
    size_t size() const {
        if (end_ <= start_) return 0;
        return static_cast<size_t>(end_ - start_);
    }

    /**
     * @brief Reset the generator to the beginning
     */
    void reset() {
        current_ = start_;
    }

    /**
     * @brief Get the current position
     */
    int64_t current() const {
        return current_;
    }

private:
    int64_t start_;
    int64_t end_;
    int64_t current_;
};

/**
 * @brief RhoComplex - Complex number type
 *
 * Represents complex numbers with real and imaginary parts.
 */
class RhoComplex {
public:
    RhoComplex() : real_(0.0), imag_(0.0) {}
    RhoComplex(double real, double imag) : real_(real), imag_(imag) {}

    double real() const { return real_; }
    double imag() const { return imag_; }

    void setReal(double r) { real_ = r; }
    void setImag(double i) { imag_ = i; }

    double magnitude() const {
        return std::sqrt(real_ * real_ + imag_ * imag_);
    }

    double phase() const {
        return std::atan2(imag_, real_);
    }

    RhoComplex conjugate() const {
        return RhoComplex(real_, -imag_);
    }

    std::string toString() const {
        std::ostringstream oss;
        oss << real_;
        if (imag_ >= 0) oss << "+";
        oss << imag_ << "i";
        return oss.str();
    }

private:
    double real_;
    double imag_;
};

/**
 * @brief RhoSet - Unordered set container
 *
 * Provides a set data structure without duplicates.
 * Uses value hashing for efficient lookups.
 */
class RhoSet {
public:
    using SetType = std::vector<RhoValue>;

    RhoSet() = default;

    /**
     * @brief Add value to set (if not already present)
     * @return true if value was added, false if already exists
     */
    bool add(const RhoValue& value) {
        if (contains(value)) return false;
        data_.push_back(value);
        return true;
    }

    /**
     * @brief Check if value exists in set
     */
    bool contains(const RhoValue& value) const {
        for (const auto& elem : data_) {
            if (valuesEqual(elem, value)) return true;
        }
        return false;
    }

    /**
     * @brief Remove value from set
     * @return true if value was found and removed
     */
    bool remove(const RhoValue& value) {
        for (auto it = data_.begin(); it != data_.end(); ++it) {
            if (valuesEqual(*it, value)) {
                data_.erase(it);
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Get the number of elements
     */
    size_t size() const { return data_.size(); }

    /**
     * @brief Check if set is empty
     */
    bool empty() const { return data_.empty(); }

    /**
     * @brief Clear all elements
     */
    void clear() { data_.clear(); }

    /**
     * @brief Get iterator to beginning
     */
    SetType::const_iterator begin() const { return data_.begin(); }
    SetType::const_iterator end() const { return data_.end(); }

private:
    SetType data_;

    /**
     * @brief Compare two RhoValues for equality
     */
    bool valuesEqual(const RhoValue& a, const RhoValue& b) const {
        if (a.index() != b.index()) return false;

        return std::visit([&b](auto&& arg_a) -> bool {
            using T = std::decay_t<decltype(arg_a)>;
            if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, int32_t> ||
                          std::is_same_v<T, int16_t> || std::is_same_v<T, int8_t> ||
                          std::is_same_v<T, uint64_t> || std::is_same_v<T, uint32_t> ||
                          std::is_same_v<T, uint16_t> || std::is_same_v<T, uint8_t> ||
                          std::is_same_v<T, double> || std::is_same_v<T, bool> ||
                          std::is_same_v<T, std::string>) {
                return arg_a == std::get<T>(b);
            }
            return false;  // Complex types need pointer comparison
        }, a);
    }
};

/**
 * @brief RhoTuple - Immutable tuple container
 *
 * Provides an immutable, fixed-size heterogeneous container.
 */
class RhoTuple {
public:
    using TupleType = std::vector<RhoValue>;

    RhoTuple() = default;

    /**
     * @brief Create tuple from initializer list
     */
    RhoTuple(std::initializer_list<RhoValue> values) : data_(values) {}

    /**
     * @brief Create tuple from vector
     */
    explicit RhoTuple(std::vector<RhoValue> values) : data_(std::move(values)) {}

    /**
     * @brief Get value at index
     * @throws std::out_of_range if index is invalid
     */
    RhoValue get(size_t index) const {
        if (index >= data_.size()) {
            throw std::out_of_range("Tuple index " + std::to_string(index) +
                                  " out of range (size: " + std::to_string(data_.size()) + ")");
        }
        return data_[index];
    }

    /**
     * @brief Get the number of elements
     */
    size_t size() const { return data_.size(); }

    /**
     * @brief Check if tuple is empty
     */
    bool empty() const { return data_.empty(); }

    /**
     * @brief Get iterator to beginning
     */
    TupleType::const_iterator begin() const { return data_.begin(); }
    TupleType::const_iterator end() const { return data_.end(); }

private:
    TupleType data_;
};

/**
 * @brief RhoRecord - Named fields structure
 *
 * Provides a structure with named fields, similar to a struct or object.
 */
class RhoRecord {
public:
    using FieldMap = std::unordered_map<std::string, RhoValue>;

    RhoRecord() = default;

    /**
     * @brief Set a field value
     */
    void setField(const std::string& name, const RhoValue& value) {
        fields_[name] = value;
    }

    /**
     * @brief Get a field value
     * @throws std::runtime_error if field doesn't exist
     */
    RhoValue getField(const std::string& name) const {
        auto it = fields_.find(name);
        if (it == fields_.end()) {
            throw std::runtime_error("Field '" + name + "' not found in record");
        }
        return it->second;
    }

    /**
     * @brief Check if field exists
     */
    bool hasField(const std::string& name) const {
        return fields_.find(name) != fields_.end();
    }

    /**
     * @brief Get all fields
     */
    const FieldMap& fields() const { return fields_; }

    /**
     * @brief Get number of fields
     */
    size_t size() const { return fields_.size(); }

private:
    FieldMap fields_;
};

/**
 * @brief RhoEnum - Enumeration type
 *
 * Represents an enumeration with a name and value.
 */
class RhoEnum {
public:
    RhoEnum(std::string name, std::string value)
        : name_(std::move(name)), value_(std::move(value)) {}

    const std::string& name() const { return name_; }
    const std::string& value() const { return value_; }

    /**
     * @brief Check if this enum equals another
     */
    bool equals(const RhoEnum& other) const {
        return name_ == other.name_ && value_ == other.value_;
    }

private:
    std::string name_;   // Enum type name
    std::string value_;  // Enum value
};

/**
 * @brief RhoNull - Explicit null value
 *
 * Represents an explicit null/none value.
 */
class RhoNull {
public:
    RhoNull() = default;

    bool operator==(const RhoNull&) const { return true; }
    bool operator!=(const RhoNull&) const { return false; }
};

} // namespace Rhodesia

#endif // RHODESIA_RHOVALUE_HPP
