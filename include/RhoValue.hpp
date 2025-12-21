/**
 * @file RhoValue.hpp
 * @brief Core value types for Rhodesia language
 * 
 * Defines the fundamental value type (RhoValue) as a std::variant
 * that can hold scalars, vectors, and matrices using Eigen types.
 */

#ifndef RHODESIA_RHOVALUE_HPP
#define RHODESIA_RHOVALUE_HPP

#include <variant>
#include <string>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <Eigen/Dense>
#include <memory>
#include <unordered_map>
#include <vector>

namespace Rhodesia {

/**
 * @brief Enumeration of Rhodesia's type system
 */
enum class RhoType {
    Int,
    Float64,
    Vec,
    Mat,
    String,
    Map,
    Void,
    Unknown
};

/**
 * @brief Convert RhoType to string representation
 */
inline std::string typeToString(RhoType type) {
    switch (type) {
        case RhoType::Int:     return "int";
        case RhoType::Float64: return "float64";
        case RhoType::Vec:     return "vec";
        case RhoType::Mat:     return "mat";
        case RhoType::String:  return "string";
        case RhoType::Map:     return "map";
        case RhoType::Void:    return "void";
        default:               return "unknown";
    }
}

/**
 * @brief Parse string to RhoType
 */
inline RhoType stringToType(const std::string& str) {
    if (str == "int")     return RhoType::Int;
    if (str == "float64") return RhoType::Float64;
    if (str == "vec")     return RhoType::Vec;
    if (str == "mat")     return RhoType::Mat;
    if (str == "string")  return RhoType::String;
    if (str == "map")     return RhoType::Map;
    if (str == "void")    return RhoType::Void;
    return RhoType::Unknown;
}

// Forward declarations
class RangeGenerator;
class RhoMap;

/**
 * @brief The core value type for Rhodesia
 *
 * RhoValue can hold:
 * - int (64-bit integer)
 * - double (float64)
 * - std::string (text)
 * - Eigen::VectorXd (dynamic-size vector)
 * - Eigen::MatrixXd (dynamic-size matrix)
 * - std::shared_ptr<RangeGenerator> (lazy range generator)
 */
using RhoValue = std::variant<
    int64_t,
    double,
    std::string,
    Eigen::VectorXd,
    Eigen::MatrixXd,
    std::shared_ptr<RangeGenerator>,
    std::shared_ptr<RhoMap>
>;

/**
 * @brief Get the RhoType of a RhoValue
 */
inline RhoType getValueType(const RhoValue& value) {
    return std::visit([](auto&& arg) -> RhoType {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int64_t>)
            return RhoType::Int;
        else if constexpr (std::is_same_v<T, double>)
            return RhoType::Float64;
        else if constexpr (std::is_same_v<T, std::string>)
            return RhoType::String;
        else if constexpr (std::is_same_v<T, Eigen::VectorXd>)
            return RhoType::Vec;
        else if constexpr (std::is_same_v<T, Eigen::MatrixXd>)
            return RhoType::Mat;
        else if constexpr (std::is_same_v<T, std::shared_ptr<RhoMap>>)
            return RhoType::Map;
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
        if constexpr (std::is_same_v<T, int64_t>)
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
        if constexpr (std::is_same_v<T, int64_t>)
            return arg;
        else if constexpr (std::is_same_v<T, double>)
            return static_cast<int64_t>(arg);
        else
            throw std::runtime_error("Cannot convert non-scalar to int");
    }, value);
}

/**
 * @brief Check if value is a scalar (int or double)
 */
inline bool isScalar(const RhoValue& value) {
    return std::holds_alternative<int64_t>(value) || 
        std::holds_alternative<double>(value);
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
        if constexpr (std::is_same_v<T, int64_t>) {
            oss << arg;
        }
        else if constexpr (std::is_same_v<T, double>) {
            oss << arg;
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            oss << arg;
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

} // namespace Rhodesia

#endif // RHODESIA_RHOVALUE_HPP
