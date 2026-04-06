/**
 * @file Builtins.hpp
 * @brief Built-in functions for Rhodesia language
 * 
 * Provides mathematical and utility functions that operate on
 * Rhodesia's native types using Eigen for linear algebra operations.
 */

#ifndef RHODESIA_BUILTINS_HPP
#define RHODESIA_BUILTINS_HPP

#include "RhoValue.hpp"
#include "Error.hpp"
#include <unordered_map>
#include <functional>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <sstream>

namespace Rhodesia {

/**
 * @brief File handle structure for IO operations
 */
struct FileHandle {
    std::fstream stream;
    std::string filename;
    std::ios::openmode mode;
    bool is_open;

    FileHandle() : is_open(false) {}
};

/**
 * @brief Global file handle manager
 */
class FileHandleManager {
public:
    static FileHandleManager& instance() {
        static FileHandleManager inst;
        return inst;
    }

    int64_t openFile(const std::string& filename, std::ios::openmode mode) {
        int64_t handle = nextHandle_++;
        auto& fh = handles_[handle];
        fh.filename = filename;
        fh.mode = mode;
        fh.stream.open(filename, mode);
        fh.is_open = fh.stream.is_open();
        return handle;
    }

    FileHandle* getHandle(int64_t handle) {
        auto it = handles_.find(handle);
        if (it == handles_.end()) return nullptr;
        return &it->second;
    }

    bool closeFile(int64_t handle) {
        auto it = handles_.find(handle);
        if (it == handles_.end()) return false;
        if (it->second.is_open) {
            it->second.stream.close();
            it->second.is_open = false;
        }
        handles_.erase(it);
        return true;
    }

private:
    std::unordered_map<int64_t, FileHandle> handles_;
    int64_t nextHandle_ = 1;
};

/**
 * @brief Type alias for built-in function signature
 */
using BuiltinFunc = std::function<RhoValue(const std::vector<RhoValue>&, SourceLocation)>;

/**
 * @brief Registry of all built-in functions
 */
class Builtins {
public:
    static Builtins& instance() {
        static Builtins inst;
        return inst;
    }

    /**
     * @brief Check if a function is a built-in
     */
    bool isBuiltin(const std::string& name) const {
        return functions_.find(name) != functions_.end();
    }

    /**
     * @brief Check if a module.function is a built-in
     */
    bool isModuleFunction(const std::string& module, const std::string& function) const {
        auto it = modules_.find(module);
        if (it == modules_.end()) return false;
        return it->second.find(function) != it->second.end();
    }

    /**
     * @brief Call a built-in function
     */
    RhoValue call(const std::string& name, const std::vector<RhoValue>& args,
                SourceLocation loc = {}) {
        auto it = functions_.find(name);
        if (it == functions_.end()) {
            throw RuntimeError::undefinedFunction(name, loc);
        }
        return it->second(args, loc);
    }

    /**
     * @brief Call a module function (e.g., math.zeros)
     */
    RhoValue callModule(const std::string& module, const std::string& function,
                       const std::vector<RhoValue>& args, SourceLocation loc = {}) {
        auto modIt = modules_.find(module);
        if (modIt == modules_.end()) {
            throw RuntimeError("Unknown module '" + module + "'", loc);
        }

        auto funcIt = modIt->second.find(function);
        if (funcIt == modIt->second.end()) {
            throw RuntimeError::undefinedFunction(module + "." + function, loc);
        }

        return funcIt->second(args, loc);
    }

    /**
     * @brief Check if a module has a constant
     */
    bool isModuleConstant(const std::string& module, const std::string& constant) const {
        auto it = moduleConstants_.find(module);
        if (it == moduleConstants_.end()) return false;
        return it->second.find(constant) != it->second.end();
    }

    /**
     * @brief Get a module constant value (e.g., math.PI)
     */
    RhoValue getModuleConstant(const std::string& module, const std::string& constant,
                              SourceLocation loc = {}) const {
        auto modIt = moduleConstants_.find(module);
        if (modIt == moduleConstants_.end()) {
            throw RuntimeError("Unknown module '" + module + "'", loc);
        }

        auto constIt = modIt->second.find(constant);
        if (constIt == modIt->second.end()) {
            throw RuntimeError("Unknown constant: " + module + "." + constant, loc);
        }

        return constIt->second;
    }

private:
    std::unordered_map<std::string, BuiltinFunc> functions_;
    std::unordered_map<std::string, std::unordered_map<std::string, BuiltinFunc>> modules_;
    std::unordered_map<std::string, std::unordered_map<std::string, RhoValue>> moduleConstants_;

    Builtins() {
        registerAll();
    }
    
    void registerAll() {
        // ====================================================================
        // Math Module Functions
        // ====================================================================

        auto& mathModule = modules_["math"];
        auto& mathConstants = moduleConstants_["math"];

        // Mathematical Constants (accesibles sin paréntesis)
        mathConstants["PI"] = M_PI;
        mathConstants["E"] = M_E;
        mathConstants["PHI"] = 1.618033988749895;  // Golden ratio
        mathConstants["SQRT_2"] = M_SQRT2;
        mathConstants["SQRT_3"] = 1.732050807568877;

        // norm(vec) -> float64, norm(mat) -> float64
        mathModule["norm"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("norm", 1, args.size(), loc);
            }
            
            return std::visit([&loc](auto&& arg) -> RhoValue {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, Eigen::VectorXd>) {
                    return arg.norm();
                } else if constexpr (std::is_same_v<T, Eigen::MatrixXd>) {
                    return arg.norm();  // Frobenius norm
                } else {
                    throw ArgumentError("norm", "argument must be vec or mat", loc);
                }
            }, args[0]);
        };
        
        // dot(vec, vec) -> float64
        mathModule["dot"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("dot", 2, args.size(), loc);
            }
            
            auto* v1 = std::get_if<Eigen::VectorXd>(&args[0]);
            auto* v2 = std::get_if<Eigen::VectorXd>(&args[1]);
            
            if (!v1 || !v2) {
                throw ArgumentError("dot", "both arguments must be vec", loc);
            }
            
            if (v1->size() != v2->size()) {
                throw RuntimeError::dimensionMismatch("dot",
                    std::to_string(v1->size()), std::to_string(v2->size()), loc);
            }
            
            return v1->dot(*v2);
        };
        
        // transpose(mat) -> mat, transpose(vec) -> mat (row vector)
        mathModule["transpose"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("transpose", 1, args.size(), loc);
            }
            
            return std::visit([&loc](auto&& arg) -> RhoValue {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, Eigen::MatrixXd>) {
                    return Eigen::MatrixXd(arg.transpose());
                } else if constexpr (std::is_same_v<T, Eigen::VectorXd>) {
                    // Convert column vector to row vector (1xN matrix)
                    return Eigen::MatrixXd(arg.transpose());
                } else {
                    throw ArgumentError("transpose", "argument must be mat or vec", loc);
                }
            }, args[0]);
        };
        
        // inv(mat) -> mat
        mathModule["inv"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("inv", 1, args.size(), loc);
            }
            
            auto* m = std::get_if<Eigen::MatrixXd>(&args[0]);
            if (!m) {
                throw ArgumentError("inv", "argument must be mat", loc);
            }
            
            if (m->rows() != m->cols()) {
                throw RuntimeError("inv(): matrix must be square", loc);
            }
            
            // Use FullPivLU for numerical stability
            Eigen::FullPivLU<Eigen::MatrixXd> lu(*m);
            if (!lu.isInvertible()) {
                throw RuntimeError::singularMatrix(loc);
            }
            
            return Eigen::MatrixXd(lu.inverse());
        };
        
        // sum(vec) -> float64, sum(mat) -> float64
        mathModule["sum"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("sum", 1, args.size(), loc);
            }
            
            return std::visit([&loc](auto&& arg) -> RhoValue {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, Eigen::VectorXd>) {
                    return arg.sum();
                } else if constexpr (std::is_same_v<T, Eigen::MatrixXd>) {
                    return arg.sum();
                } else if constexpr (std::is_same_v<T, int64_t>) {
                    return static_cast<double>(arg);
                } else if constexpr (std::is_same_v<T, double>) {
                    return arg;
                } else {
                    throw ArgumentError("sum", "argument must be numeric", loc);
                }
            }, args[0]);
        };
        
        // mean(vec) -> float64, mean(mat) -> float64
        mathModule["mean"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("mean", 1, args.size(), loc);
            }
            
            return std::visit([&loc](auto&& arg) -> RhoValue {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, Eigen::VectorXd>) {
                    return arg.mean();
                } else if constexpr (std::is_same_v<T, Eigen::MatrixXd>) {
                    return arg.mean();
                } else {
                    throw ArgumentError("mean", "argument must be vec or mat", loc);
                }
            }, args[0]);
        };
        
        // ====================================================================
        // Creation Functions
        // ====================================================================
        
        // zeros(n) -> vec, zeros(n, m) -> mat
        mathModule["zeros"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.empty() || args.size() > 2) {
                throw ArgumentError("zeros", "expected 1 or 2 arguments", loc);
            }

            int64_t n = toInt(args[0]);
            if (n <= 0) throw ArgumentError("zeros", "size must be positive", loc);

            if (args.size() == 1) {
                return Eigen::VectorXd(Eigen::VectorXd::Zero(n));
            } else {
                int64_t m = toInt(args[1]);
                if (m <= 0) throw ArgumentError("zeros", "size must be positive", loc);
                return Eigen::MatrixXd(Eigen::MatrixXd::Zero(n, m));
            }
        };

        // ones(n) -> vec, ones(n, m) -> mat
        mathModule["ones"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.empty() || args.size() > 2) {
                throw ArgumentError("ones", "expected 1 or 2 arguments", loc);
            }

            int64_t n = toInt(args[0]);
            if (n <= 0) throw ArgumentError("ones", "size must be positive", loc);

            if (args.size() == 1) {
                return Eigen::VectorXd(Eigen::VectorXd::Ones(n));
            } else {
                int64_t m = toInt(args[1]);
                if (m <= 0) throw ArgumentError("ones", "size must be positive", loc);
                return Eigen::MatrixXd(Eigen::MatrixXd::Ones(n, m));
            }
        };
        
        // eye(n) -> mat (identity matrix)
        mathModule["eye"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("eye", 1, args.size(), loc);
            }
            
            int64_t n = toInt(args[0]);
            if (n <= 0) throw ArgumentError("eye", "size must be positive", loc);
            
            return Eigen::MatrixXd(Eigen::MatrixXd::Identity(n, n));
        };
        
        // range(n) -> RangeGenerator[0..n-1], range(start, end) -> RangeGenerator[start..end-1]
        functions_["range"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.empty() || args.size() > 2) {
                throw ArgumentError("range", "expected 1 or 2 arguments", loc);
            }

            int64_t start = 0;
            int64_t end;

            if (args.size() == 1) {
                end = toInt(args[0]);
            } else {
                start = toInt(args[0]);
                end = toInt(args[1]);
            }

            if (end < start) {
                // Return empty vector for backward compatibility
                return Eigen::VectorXd(0);
            }

            // Create a lazy range generator instead of materializing the full vector
            return std::make_shared<RangeGenerator>(start, end);
        };
        
        // ====================================================================
        // Math Functions
        // ====================================================================
        
        // sqrt(x) - scalar or element-wise
        mathModule["sqrt"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("sqrt", 1, args.size(), loc);
            }
            
            return std::visit([&loc](auto&& arg) -> RhoValue {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, int64_t>) {
                    return std::sqrt(static_cast<double>(arg));
                } else if constexpr (std::is_same_v<T, double>) {
                    return std::sqrt(arg);
                } else if constexpr (std::is_same_v<T, Eigen::VectorXd>) {
                    return Eigen::VectorXd(arg.array().sqrt());
                } else if constexpr (std::is_same_v<T, Eigen::MatrixXd>) {
                    return Eigen::MatrixXd(arg.array().sqrt());
                } else {
                    throw ArgumentError("sqrt", "invalid argument type", loc);
                }
            }, args[0]);
        };
        
        // exp(x)
        mathModule["exp"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("exp", 1, args.size(), loc);
            }
            
            return std::visit([&loc](auto&& arg) -> RhoValue {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, int64_t>) {
                    return std::exp(static_cast<double>(arg));
                } else if constexpr (std::is_same_v<T, double>) {
                    return std::exp(arg);
                } else if constexpr (std::is_same_v<T, Eigen::VectorXd>) {
                    return Eigen::VectorXd(arg.array().exp());
                } else if constexpr (std::is_same_v<T, Eigen::MatrixXd>) {
                    return Eigen::MatrixXd(arg.array().exp());
                } else {
                    throw ArgumentError("exp", "invalid argument type", loc);
                }
            }, args[0]);
        };
        
        // log(x)
        mathModule["log"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("log", 1, args.size(), loc);
            }
            
            return std::visit([&loc](auto&& arg) -> RhoValue {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, int64_t>) {
                    return std::log(static_cast<double>(arg));
                } else if constexpr (std::is_same_v<T, double>) {
                    return std::log(arg);
                } else if constexpr (std::is_same_v<T, Eigen::VectorXd>) {
                    return Eigen::VectorXd(arg.array().log());
                } else if constexpr (std::is_same_v<T, Eigen::MatrixXd>) {
                    return Eigen::MatrixXd(arg.array().log());
                } else {
                    throw ArgumentError("log", "invalid argument type", loc);
                }
            }, args[0]);
        };
        
        // abs(x)
        mathModule["abs"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("abs", 1, args.size(), loc);
            }
            
            return std::visit([&loc](auto&& arg) -> RhoValue {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, int64_t>) {
                    return std::abs(arg);
                } else if constexpr (std::is_same_v<T, double>) {
                    return std::abs(arg);
                } else if constexpr (std::is_same_v<T, Eigen::VectorXd>) {
                    return Eigen::VectorXd(arg.array().abs());
                } else if constexpr (std::is_same_v<T, Eigen::MatrixXd>) {
                    return Eigen::MatrixXd(arg.array().abs());
                } else {
                    throw ArgumentError("abs", "invalid argument type", loc);
                }
            }, args[0]);
        };
        
        // sin(x), cos(x), tan(x)
        auto makeTrigFunc = [](const std::string& name, double(*func)(double)) {
            return [name, func](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
                if (args.size() != 1) {
                    throw ArgumentError::wrongCount(name, 1, args.size(), loc);
                }
                
                return std::visit([&](auto&& arg) -> RhoValue {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, int64_t>) {
                        return func(static_cast<double>(arg));
                    } else if constexpr (std::is_same_v<T, double>) {
                        return func(arg);
                    } else {
                        throw ArgumentError(name, "argument must be scalar", loc);
                    }
                }, args[0]);
            };
        };
        
        mathModule["sin"] = makeTrigFunc("sin", std::sin);
        mathModule["cos"] = makeTrigFunc("cos", std::cos);
        mathModule["tan"] = makeTrigFunc("tan", std::tan);
        mathModule["asin"] = makeTrigFunc("asin", std::asin);
        mathModule["acos"] = makeTrigFunc("acos", std::acos);
        mathModule["atan"] = makeTrigFunc("atan", std::atan);

        // atan2(y, x)
        mathModule["atan2"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("atan2", 2, args.size(), loc);
            }
            double y = toDouble(args[0]);
            double x = toDouble(args[1]);
            return std::atan2(y, x);
        };

        // Hyperbolic functions
        mathModule["sinh"] = makeTrigFunc("sinh", std::sinh);
        mathModule["cosh"] = makeTrigFunc("cosh", std::cosh);
        mathModule["tanh"] = makeTrigFunc("tanh", std::tanh);

        // Power and root functions
        mathModule["pow"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("pow", 2, args.size(), loc);
            }
            double base = toDouble(args[0]);
            double exp = toDouble(args[1]);
            return std::pow(base, exp);
        };

        mathModule["cbrt"] = makeTrigFunc("cbrt", std::cbrt);

        // Rounding functions
        mathModule["floor"] = makeTrigFunc("floor", std::floor);
        mathModule["ceil"] = makeTrigFunc("ceil", std::ceil);
        mathModule["round"] = makeTrigFunc("round", std::round);
        mathModule["trunc"] = makeTrigFunc("trunc", std::trunc);

        // Min/Max functions
        mathModule["min"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.empty()) {
                throw ArgumentError("min", "expected at least 1 argument", loc);
            }

            if (args.size() == 1) {
                // min(vec) or min(mat)
                return std::visit([&loc](auto&& arg) -> RhoValue {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, Eigen::VectorXd>) {
                        return arg.minCoeff();
                    } else if constexpr (std::is_same_v<T, Eigen::MatrixXd>) {
                        return arg.minCoeff();
                    } else if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, double>) {
                        return arg;
                    } else {
                        throw ArgumentError("min", "argument must be numeric", loc);
                    }
                }, args[0]);
            } else {
                // min(a, b, c, ...)
                double minVal = toDouble(args[0]);
                for (size_t i = 1; i < args.size(); ++i) {
                    minVal = std::min(minVal, toDouble(args[i]));
                }
                return minVal;
            }
        };

        mathModule["max"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.empty()) {
                throw ArgumentError("max", "expected at least 1 argument", loc);
            }

            if (args.size() == 1) {
                // max(vec) or max(mat)
                return std::visit([&loc](auto&& arg) -> RhoValue {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, Eigen::VectorXd>) {
                        return arg.maxCoeff();
                    } else if constexpr (std::is_same_v<T, Eigen::MatrixXd>) {
                        return arg.maxCoeff();
                    } else if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, double>) {
                        return arg;
                    } else {
                        throw ArgumentError("max", "argument must be numeric", loc);
                    }
                }, args[0]);
            } else {
                // max(a, b, c, ...)
                double maxVal = toDouble(args[0]);
                for (size_t i = 1; i < args.size(); ++i) {
                    maxVal = std::max(maxVal, toDouble(args[i]));
                }
                return maxVal;
            }
        };

        // Utility functions
        mathModule["clamp"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 3) {
                throw ArgumentError::wrongCount("clamp", 3, args.size(), loc);
            }
            double value = toDouble(args[0]);
            double minVal = toDouble(args[1]);
            double maxVal = toDouble(args[2]);
            return std::clamp(value, minVal, maxVal);
        };

        mathModule["sign"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("sign", 1, args.size(), loc);
            }
            double x = toDouble(args[0]);
            if (x > 0) return 1.0;
            if (x < 0) return -1.0;
            return 0.0;
        };

        mathModule["lerp"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 3) {
                throw ArgumentError::wrongCount("lerp", 3, args.size(), loc);
            }
            double a = toDouble(args[0]);
            double b = toDouble(args[1]);
            double t = toDouble(args[2]);
            return a + t * (b - a);
        };

        // Angle conversion
        mathModule["deg2rad"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("deg2rad", 1, args.size(), loc);
            }
            double deg = toDouble(args[0]);
            return deg * M_PI / 180.0;
        };

        mathModule["rad2deg"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("rad2deg", 1, args.size(), loc);
            }
            double rad = toDouble(args[0]);
            return rad * 180.0 / M_PI;
        };

        // Factorial
        mathModule["factorial"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("factorial", 1, args.size(), loc);
            }
            int64_t n = toInt(args[0]);
            if (n < 0) {
                throw ArgumentError("factorial", "argument must be non-negative", loc);
            }
            if (n > 20) {
                throw ArgumentError("factorial", "argument too large (max 20)", loc);
            }
            int64_t result = 1;
            for (int64_t i = 2; i <= n; ++i) {
                result *= i;
            }
            return result;
        };
        
        // ====================================================================
        // Information Functions
        // ====================================================================
        
        // rows(mat) -> int
        mathModule["rows"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("rows", 1, args.size(), loc);
            }
            
            auto* m = std::get_if<Eigen::MatrixXd>(&args[0]);
            if (!m) {
                throw ArgumentError("rows", "argument must be mat", loc);
            }
            
            return static_cast<int64_t>(m->rows());
        };
        
        // cols(mat) -> int
        mathModule["cols"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("cols", 1, args.size(), loc);
            }
            
            auto* m = std::get_if<Eigen::MatrixXd>(&args[0]);
            if (!m) {
                throw ArgumentError("cols", "argument must be mat", loc);
            }
            
            return static_cast<int64_t>(m->cols());
        };
        
        // size(vec) -> int, size(mat) -> int, size(range) -> int (total elements)
        mathModule["size"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("size", 1, args.size(), loc);
            }

            if (auto* rangeGen = std::get_if<std::shared_ptr<RangeGenerator>>(&args[0])) {
                return static_cast<int64_t>((*rangeGen)->size());
            }

            return std::visit([&loc](auto&& arg) -> RhoValue {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, Eigen::VectorXd>) {
                    return static_cast<int64_t>(arg.size());
                } else if constexpr (std::is_same_v<T, Eigen::MatrixXd>) {
                    return static_cast<int64_t>(arg.size());
                } else {
                    throw ArgumentError("size", "argument must be vec, mat, or range", loc);
                }
            }, args[0]);
        };
        
        // ====================================================================
        // I/O Functions
        // ====================================================================
        
        // print(value) - print without newline
        functions_["print"] = [](const std::vector<RhoValue>& args, SourceLocation) -> RhoValue {
            for (size_t i = 0; i < args.size(); ++i) {
                if (i > 0) std::cout << " ";
                std::cout << valueToString(args[i]);
            }
            return int64_t(0);  // void
        };
        
        // println(value) - print with newline
        functions_["println"] = [](const std::vector<RhoValue>& args, SourceLocation) -> RhoValue {
            for (size_t i = 0; i < args.size(); ++i) {
                if (i > 0) std::cout << " ";
                std::cout << valueToString(args[i]);
            }
            std::cout << std::endl;
            return int64_t(0);  // void
        };

        // ====================================================================
        // Timing Functions
        // ====================================================================

        // get_tick() -> float64 - high-precision monotonic timestamp
        functions_["get_tick"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 0) {
                throw ArgumentError::wrongCount("get_tick", 0, args.size(), loc);
            }

            // Use steady_clock (equivalent to CLOCK_MONOTONIC) for monotonic timing
            auto now = std::chrono::steady_clock::now();
            auto duration = now.time_since_epoch();
            return std::chrono::duration<double>(duration).count();
        };

        // ====================================================================
        // New Data Types Constructors
        // ====================================================================

        // make_complex(real, imag) -> complex - create complex number
        functions_["make_complex"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("make_complex", 2, args.size(), loc);
            }
            double real = toDouble(args[0]);
            double imag = toDouble(args[1]);
            return std::make_shared<RhoComplex>(real, imag);
        };

        // make_set() -> set - create empty set
        // make_set(val1, val2, ...) -> set - create set with values
        functions_["make_set"] = [](const std::vector<RhoValue>& args, SourceLocation) -> RhoValue {
            auto s = std::make_shared<RhoSet>();
            for (const auto& arg : args) {
                s->add(arg);
            }
            return s;
        };

        // make_tuple(val1, val2, ...) -> tuple - create tuple
        functions_["make_tuple"] = [](const std::vector<RhoValue>& args, SourceLocation) -> RhoValue {
            std::vector<RhoValue> values(args.begin(), args.end());
            return std::make_shared<RhoTuple>(std::move(values));
        };

        // make_record() -> record - create empty record
        functions_["make_record"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 0) {
                throw ArgumentError::wrongCount("make_record", 0, args.size(), loc);
            }
            return std::make_shared<RhoRecord>();
        };

        // ====================================================================
        // Statistics Module Functions
        // ====================================================================

        auto& statsModule = modules_["stats"];

        // std(vec) -> float64, std(mat) -> float64
        // Calcula la desviación estándar (con corrección de Bessel: N-1)
        statsModule["std"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("stats.std", 1, args.size(), loc);
            }

            return std::visit([&loc](auto&& arg) -> RhoValue {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, Eigen::VectorXd>) {
                    if (arg.size() < 2) {
                        throw ArgumentError("stats.std", "vector must have at least 2 elements", loc);
                    }
                    double mean_val = arg.mean();
                    double variance = (arg.array() - mean_val).square().sum() / (arg.size() - 1);
                    return std::sqrt(variance);
                } else if constexpr (std::is_same_v<T, Eigen::MatrixXd>) {
                    if (arg.size() < 2) {
                        throw ArgumentError("stats.std", "matrix must have at least 2 elements", loc);
                    }
                    double mean_val = arg.mean();
                    double variance = (arg.array() - mean_val).square().sum() / (arg.size() - 1);
                    return std::sqrt(variance);
                } else {
                    throw ArgumentError("stats.std", "argument must be vec or mat", loc);
                }
            }, args[0]);
        };

        // var(vec) -> float64, var(mat) -> float64
        // Calcula la varianza (con corrección de Bessel: N-1)
        statsModule["var"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("stats.var", 1, args.size(), loc);
            }

            return std::visit([&loc](auto&& arg) -> RhoValue {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, Eigen::VectorXd>) {
                    if (arg.size() < 2) {
                        throw ArgumentError("stats.var", "vector must have at least 2 elements", loc);
                    }
                    double mean_val = arg.mean();
                    double variance = (arg.array() - mean_val).square().sum() / (arg.size() - 1);
                    return variance;
                } else if constexpr (std::is_same_v<T, Eigen::MatrixXd>) {
                    if (arg.size() < 2) {
                        throw ArgumentError("stats.var", "matrix must have at least 2 elements", loc);
                    }
                    double mean_val = arg.mean();
                    double variance = (arg.array() - mean_val).square().sum() / (arg.size() - 1);
                    return variance;
                } else {
                    throw ArgumentError("stats.var", "argument must be vec or mat", loc);
                }
            }, args[0]);
        };

        // cov(vec, vec) -> float64
        // Calcula la covarianza entre dos vectores (con corrección de Bessel: N-1)
        statsModule["cov"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("stats.cov", 2, args.size(), loc);
            }

            auto* v1 = std::get_if<Eigen::VectorXd>(&args[0]);
            auto* v2 = std::get_if<Eigen::VectorXd>(&args[1]);

            if (!v1 || !v2) {
                throw ArgumentError("stats.cov", "both arguments must be vec", loc);
            }

            if (v1->size() != v2->size()) {
                throw RuntimeError::dimensionMismatch("stats.cov",
                    std::to_string(v1->size()), std::to_string(v2->size()), loc);
            }

            if (v1->size() < 2) {
                throw ArgumentError("stats.cov", "vectors must have at least 2 elements", loc);
            }

            double mean1 = v1->mean();
            double mean2 = v2->mean();

            double covariance = ((v1->array() - mean1) * (v2->array() - mean2)).sum() / (v1->size() - 1);
            return covariance;
        };

        // mean(vec) -> float64, mean(mat) -> float64
        // También agregada al módulo stats para coherencia
        statsModule["mean"] = mathModule["mean"];

        // median(vec) -> float64
        statsModule["median"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("stats.median", 1, args.size(), loc);
            }

            auto* vec = std::get_if<Eigen::VectorXd>(&args[0]);
            if (!vec) {
                throw ArgumentError("stats.median", "argument must be vec", loc);
            }

            if (vec->size() == 0) {
                throw ArgumentError("stats.median", "vector must not be empty", loc);
            }

            Eigen::VectorXd sorted = *vec;
            std::sort(sorted.data(), sorted.data() + sorted.size());

            if (sorted.size() % 2 == 0) {
                return (sorted(sorted.size() / 2 - 1) + sorted(sorted.size() / 2)) / 2.0;
            } else {
                return sorted(sorted.size() / 2);
            }
        };

        // percentile(vec, p) -> float64
        statsModule["percentile"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("stats.percentile", 2, args.size(), loc);
            }

            auto* vec = std::get_if<Eigen::VectorXd>(&args[0]);
            if (!vec) {
                throw ArgumentError("stats.percentile", "first argument must be vec", loc);
            }

            double p = toDouble(args[1]);
            if (p < 0.0 || p > 100.0) {
                throw ArgumentError("stats.percentile", "percentile must be between 0 and 100", loc);
            }

            if (vec->size() == 0) {
                throw ArgumentError("stats.percentile", "vector must not be empty", loc);
            }

            Eigen::VectorXd sorted = *vec;
            std::sort(sorted.data(), sorted.data() + sorted.size());

            double idx = (p / 100.0) * (sorted.size() - 1);
            size_t lower = static_cast<size_t>(std::floor(idx));
            size_t upper = static_cast<size_t>(std::ceil(idx));

            if (lower == upper) {
                return sorted(lower);
            } else {
                double weight = idx - lower;
                return sorted(lower) * (1.0 - weight) + sorted(upper) * weight;
            }
        };

        // min(vec) -> float64
        statsModule["min"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("stats.min", 1, args.size(), loc);
            }

            auto* vec = std::get_if<Eigen::VectorXd>(&args[0]);
            if (!vec) {
                throw ArgumentError("stats.min", "argument must be vec", loc);
            }

            return vec->minCoeff();
        };

        // max(vec) -> float64
        statsModule["max"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("stats.max", 1, args.size(), loc);
            }

            auto* vec = std::get_if<Eigen::VectorXd>(&args[0]);
            if (!vec) {
                throw ArgumentError("stats.max", "argument must be vec", loc);
            }

            return vec->maxCoeff();
        };

        // corr(vec, vec) -> float64 (Pearson correlation coefficient)
        statsModule["corr"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("stats.corr", 2, args.size(), loc);
            }

            auto* v1 = std::get_if<Eigen::VectorXd>(&args[0]);
            auto* v2 = std::get_if<Eigen::VectorXd>(&args[1]);

            if (!v1 || !v2) {
                throw ArgumentError("stats.corr", "both arguments must be vec", loc);
            }

            if (v1->size() != v2->size()) {
                throw RuntimeError::dimensionMismatch("stats.corr",
                    std::to_string(v1->size()), std::to_string(v2->size()), loc);
            }

            if (v1->size() < 2) {
                throw ArgumentError("stats.corr", "vectors must have at least 2 elements", loc);
            }

            double mean1 = v1->mean();
            double mean2 = v2->mean();

            double numerator = ((v1->array() - mean1) * (v2->array() - mean2)).sum();
            double denom1 = ((v1->array() - mean1).square()).sum();
            double denom2 = ((v2->array() - mean2).square()).sum();

            if (denom1 == 0.0 || denom2 == 0.0) {
                return 0.0; // Undefined, return 0
            }

            return numerator / std::sqrt(denom1 * denom2);
        };

        // skewness(vec) -> float64
        statsModule["skewness"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("stats.skewness", 1, args.size(), loc);
            }

            auto* vec = std::get_if<Eigen::VectorXd>(&args[0]);
            if (!vec) {
                throw ArgumentError("stats.skewness", "argument must be vec", loc);
            }

            if (vec->size() < 3) {
                throw ArgumentError("stats.skewness", "vector must have at least 3 elements", loc);
            }

            double mean_val = vec->mean();
            double std_val = std::sqrt((vec->array() - mean_val).square().sum() / (vec->size() - 1));

            if (std_val == 0.0) {
                return 0.0;
            }

            double skew = ((vec->array() - mean_val).cube()).sum() / (vec->size() * std::pow(std_val, 3));
            return skew;
        };

        // kurtosis(vec) -> float64
        statsModule["kurtosis"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("stats.kurtosis", 1, args.size(), loc);
            }

            auto* vec = std::get_if<Eigen::VectorXd>(&args[0]);
            if (!vec) {
                throw ArgumentError("stats.kurtosis", "argument must be vec", loc);
            }

            if (vec->size() < 4) {
                throw ArgumentError("stats.kurtosis", "vector must have at least 4 elements", loc);
            }

            double mean_val = vec->mean();
            double std_val = std::sqrt((vec->array() - mean_val).square().sum() / (vec->size() - 1));

            if (std_val == 0.0) {
                return 0.0;
            }

            double kurt = ((vec->array() - mean_val).pow(4)).sum() / (vec->size() * std::pow(std_val, 4)) - 3.0;
            return kurt;
        };

        // zscore(vec) -> vec
        statsModule["zscore"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("stats.zscore", 1, args.size(), loc);
            }

            auto* vec = std::get_if<Eigen::VectorXd>(&args[0]);
            if (!vec) {
                throw ArgumentError("stats.zscore", "argument must be vec", loc);
            }

            if (vec->size() < 2) {
                throw ArgumentError("stats.zscore", "vector must have at least 2 elements", loc);
            }

            double mean_val = vec->mean();
            double std_val = std::sqrt((vec->array() - mean_val).square().sum() / (vec->size() - 1));

            if (std_val == 0.0) {
                return Eigen::VectorXd(Eigen::VectorXd::Zero(vec->size()));
            }

            return Eigen::VectorXd((vec->array() - mean_val) / std_val);
        };

        // ====================================================================
        // Numerical Module Functions
        // ====================================================================

        auto& numModule = modules_["numerical"];

        // Root Finding: bisection(f, a, b, tol, max_iter) -> float64
        // Note: In Rhodesia, we can't pass functions as arguments easily yet,
        // so these numerical methods will be limited or require special handling.
        // For now, we'll implement simpler numerical utilities.

        // solve_quadratic(a, b, c) -> vec[2] (returns real roots)
        numModule["solve_quadratic"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 3) {
                throw ArgumentError::wrongCount("numerical.solve_quadratic", 3, args.size(), loc);
            }

            double a = toDouble(args[0]);
            double b = toDouble(args[1]);
            double c = toDouble(args[2]);

            if (std::abs(a) < 1e-10) {
                // Linear equation: bx + c = 0
                if (std::abs(b) < 1e-10) {
                    throw RuntimeError("numerical.solve_quadratic: not a valid equation", loc);
                }
                Eigen::VectorXd result(1);
                result(0) = -c / b;
                return result;
            }

            double discriminant = b * b - 4 * a * c;

            if (discriminant < 0) {
                // No real roots
                return Eigen::VectorXd(0);
            } else if (std::abs(discriminant) < 1e-10) {
                // One root
                Eigen::VectorXd result(1);
                result(0) = -b / (2 * a);
                return result;
            } else {
                // Two roots
                Eigen::VectorXd result(2);
                result(0) = (-b - std::sqrt(discriminant)) / (2 * a);
                result(1) = (-b + std::sqrt(discriminant)) / (2 * a);
                return result;
            }
        };

        // linspace(start, end, n) -> vec (n evenly spaced values)
        numModule["linspace"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 3) {
                throw ArgumentError::wrongCount("numerical.linspace", 3, args.size(), loc);
            }

            double start = toDouble(args[0]);
            double end = toDouble(args[1]);
            int64_t n = toInt(args[2]);

            if (n < 2) {
                throw ArgumentError("numerical.linspace", "n must be at least 2", loc);
            }

            Eigen::VectorXd result(n);
            double step = (end - start) / (n - 1);

            for (int64_t i = 0; i < n; ++i) {
                result(i) = start + i * step;
            }

            return result;
        };

        // logspace(start, end, n) -> vec (n logarithmically spaced values)
        numModule["logspace"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 3) {
                throw ArgumentError::wrongCount("numerical.logspace", 3, args.size(), loc);
            }

            double start = toDouble(args[0]);
            double end = toDouble(args[1]);
            int64_t n = toInt(args[2]);

            if (n < 2) {
                throw ArgumentError("numerical.logspace", "n must be at least 2", loc);
            }

            Eigen::VectorXd result(n);
            double step = (end - start) / (n - 1);

            for (int64_t i = 0; i < n; ++i) {
                result(i) = std::pow(10.0, start + i * step);
            }

            return result;
        };

        // cumsum(vec) -> vec (cumulative sum)
        numModule["cumsum"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("numerical.cumsum", 1, args.size(), loc);
            }

            auto* vec = std::get_if<Eigen::VectorXd>(&args[0]);
            if (!vec) {
                throw ArgumentError("numerical.cumsum", "argument must be vec", loc);
            }

            Eigen::VectorXd result(vec->size());
            double sum = 0.0;

            for (int i = 0; i < vec->size(); ++i) {
                sum += (*vec)(i);
                result(i) = sum;
            }

            return result;
        };

        // cumprod(vec) -> vec (cumulative product)
        numModule["cumprod"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("numerical.cumprod", 1, args.size(), loc);
            }

            auto* vec = std::get_if<Eigen::VectorXd>(&args[0]);
            if (!vec) {
                throw ArgumentError("numerical.cumprod", "argument must be vec", loc);
            }

            Eigen::VectorXd result(vec->size());
            double prod = 1.0;

            for (int i = 0; i < vec->size(); ++i) {
                prod *= (*vec)(i);
                result(i) = prod;
            }

            return result;
        };

        // diff(vec) -> vec (first-order differences)
        numModule["diff"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("numerical.diff", 1, args.size(), loc);
            }

            auto* vec = std::get_if<Eigen::VectorXd>(&args[0]);
            if (!vec) {
                throw ArgumentError("numerical.diff", "argument must be vec", loc);
            }

            if (vec->size() < 2) {
                throw ArgumentError("numerical.diff", "vector must have at least 2 elements", loc);
            }

            Eigen::VectorXd result(vec->size() - 1);

            for (int i = 0; i < result.size(); ++i) {
                result(i) = (*vec)(i + 1) - (*vec)(i);
            }

            return result;
        };

        // gradient(vec, h) -> vec (numerical gradient with step h)
        numModule["gradient"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("numerical.gradient", 2, args.size(), loc);
            }

            auto* vec = std::get_if<Eigen::VectorXd>(&args[0]);
            if (!vec) {
                throw ArgumentError("numerical.gradient", "first argument must be vec", loc);
            }

            double h = toDouble(args[1]);
            if (h <= 0.0) {
                throw ArgumentError("numerical.gradient", "step size must be positive", loc);
            }

            if (vec->size() < 2) {
                throw ArgumentError("numerical.gradient", "vector must have at least 2 elements", loc);
            }

            Eigen::VectorXd result(vec->size());

            // Forward difference for first point
            result(0) = ((*vec)(1) - (*vec)(0)) / h;

            // Central difference for middle points
            for (int i = 1; i < vec->size() - 1; ++i) {
                result(i) = ((*vec)(i + 1) - (*vec)(i - 1)) / (2.0 * h);
            }

            // Backward difference for last point
            result(vec->size() - 1) = ((*vec)(vec->size() - 1) - (*vec)(vec->size() - 2)) / h;

            return result;
        };

        // trapz(y, x) -> float64 (trapezoidal integration)
        numModule["trapz"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("numerical.trapz", 2, args.size(), loc);
            }

            auto* y = std::get_if<Eigen::VectorXd>(&args[0]);
            auto* x = std::get_if<Eigen::VectorXd>(&args[1]);

            if (!y || !x) {
                throw ArgumentError("numerical.trapz", "both arguments must be vec", loc);
            }

            if (x->size() != y->size()) {
                throw RuntimeError::dimensionMismatch("numerical.trapz",
                    std::to_string(x->size()), std::to_string(y->size()), loc);
            }

            if (x->size() < 2) {
                throw ArgumentError("numerical.trapz", "vectors must have at least 2 elements", loc);
            }

            double integral = 0.0;
            for (int i = 0; i < x->size() - 1; ++i) {
                integral += 0.5 * ((*x)(i + 1) - (*x)(i)) * ((*y)(i) + (*y)(i + 1));
            }

            return integral;
        };

        // simps(y, x) -> float64 (Simpson's rule integration)
        numModule["simps"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("numerical.simps", 2, args.size(), loc);
            }

            auto* y = std::get_if<Eigen::VectorXd>(&args[0]);
            auto* x = std::get_if<Eigen::VectorXd>(&args[1]);

            if (!y || !x) {
                throw ArgumentError("numerical.simps", "both arguments must be vec", loc);
            }

            if (x->size() != y->size()) {
                throw RuntimeError::dimensionMismatch("numerical.simps",
                    std::to_string(x->size()), std::to_string(y->size()), loc);
            }

            if (x->size() < 3) {
                throw ArgumentError("numerical.simps", "vectors must have at least 3 elements", loc);
            }

            int n = x->size();
            double integral = 0.0;

            if (n % 2 == 0) {
                // If even number of points, use trapezoidal for last interval
                for (int i = 0; i < n - 2; i += 2) {
                    double h = (*x)(i + 1) - (*x)(i);
                    integral += h / 3.0 * ((*y)(i) + 4.0 * (*y)(i + 1) + (*y)(i + 2));
                }
                // Trapezoidal for last segment
                integral += 0.5 * ((*x)(n - 1) - (*x)(n - 2)) * ((*y)(n - 2) + (*y)(n - 1));
            } else {
                // Odd number of points
                for (int i = 0; i < n - 1; i += 2) {
                    double h = (*x)(i + 1) - (*x)(i);
                    integral += h / 3.0 * ((*y)(i) + 4.0 * (*y)(i + 1) + (*y)(i + 2));
                }
            }

            return integral;
        };

        // interp1d(x, y, x_new) -> float64 (linear interpolation)
        numModule["interp1d"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 3) {
                throw ArgumentError::wrongCount("numerical.interp1d", 3, args.size(), loc);
            }

            auto* x = std::get_if<Eigen::VectorXd>(&args[0]);
            auto* y = std::get_if<Eigen::VectorXd>(&args[1]);

            if (!x || !y) {
                throw ArgumentError("numerical.interp1d", "first two arguments must be vec", loc);
            }

            if (x->size() != y->size()) {
                throw RuntimeError::dimensionMismatch("numerical.interp1d",
                    std::to_string(x->size()), std::to_string(y->size()), loc);
            }

            if (x->size() < 2) {
                throw ArgumentError("numerical.interp1d", "vectors must have at least 2 elements", loc);
            }

            double x_new = toDouble(args[2]);

            // Find the interval containing x_new
            for (int i = 0; i < x->size() - 1; ++i) {
                if (x_new >= (*x)(i) && x_new <= (*x)(i + 1)) {
                    double t = (x_new - (*x)(i)) / ((*x)(i + 1) - (*x)(i));
                    return (*y)(i) + t * ((*y)(i + 1) - (*y)(i));
                }
            }

            // Extrapolation (use nearest endpoint)
            if (x_new < (*x)(0)) {
                return (*y)(0);
            } else {
                return (*y)(x->size() - 1);
            }
        };

        // polyval(coeffs, x) -> float64 (evaluate polynomial)
        // coeffs = [a0, a1, a2, ...] for a0 + a1*x + a2*x^2 + ...
        numModule["polyval"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("numerical.polyval", 2, args.size(), loc);
            }

            auto* coeffs = std::get_if<Eigen::VectorXd>(&args[0]);
            if (!coeffs) {
                throw ArgumentError("numerical.polyval", "first argument must be vec", loc);
            }

            double x = toDouble(args[1]);
            double result = 0.0;
            double x_power = 1.0;

            for (int i = 0; i < coeffs->size(); ++i) {
                result += (*coeffs)(i) * x_power;
                x_power *= x;
            }

            return result;
        };

        // polyfit(x, y, degree) -> vec (polynomial coefficients)
        numModule["polyfit"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 3) {
                throw ArgumentError::wrongCount("numerical.polyfit", 3, args.size(), loc);
            }

            auto* x = std::get_if<Eigen::VectorXd>(&args[0]);
            auto* y = std::get_if<Eigen::VectorXd>(&args[1]);

            if (!x || !y) {
                throw ArgumentError("numerical.polyfit", "first two arguments must be vec", loc);
            }

            if (x->size() != y->size()) {
                throw RuntimeError::dimensionMismatch("numerical.polyfit",
                    std::to_string(x->size()), std::to_string(y->size()), loc);
            }

            int64_t degree = toInt(args[2]);
            if (degree < 0 || degree >= x->size()) {
                throw ArgumentError("numerical.polyfit", "invalid degree", loc);
            }

            // Build Vandermonde matrix
            int n = x->size();
            int m = degree + 1;
            Eigen::MatrixXd A(n, m);

            for (int i = 0; i < n; ++i) {
                double x_power = 1.0;
                for (int j = 0; j < m; ++j) {
                    A(i, j) = x_power;
                    x_power *= (*x)(i);
                }
            }

            // Solve least squares: A * coeffs = y
            Eigen::VectorXd coeffs = A.colPivHouseholderQr().solve(*y);

            return coeffs;
        };

        // ====================================================================
        // Vector Module Functions
        // ====================================================================

        auto& vecModule = modules_["vector"];

        // vec.append(vector, value) -> vec
        // Retorna un nuevo vector con el valor agregado al final
        vecModule["append"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("vec.append", 2, args.size(), loc);
            }

            auto* vec = std::get_if<Eigen::VectorXd>(&args[0]);
            if (!vec) {
                throw ArgumentError("vec.append", "first argument must be vec", loc);
            }

            // Verificar que el segundo argumento sea un escalar
            if (!std::holds_alternative<int64_t>(args[1]) &&
                !std::holds_alternative<double>(args[1])) {
                throw ArgumentError("vec.append", "second argument must be a scalar (int or float64)", loc);
            }

            double value = std::visit([](auto&& arg) -> double {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, int64_t>) {
                    return static_cast<double>(arg);
                } else if constexpr (std::is_same_v<T, double>) {
                    return arg;
                }
                return 0.0;
            }, args[1]);

            // Crear nuevo vector con un elemento más
            Eigen::VectorXd result(vec->size() + 1);
            result.head(vec->size()) = *vec;
            result(vec->size()) = value;

            return result;
        };

        // vec.remove(vector, index) -> vec
        // Retorna un nuevo vector sin el elemento en la posición especificada
        vecModule["remove"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("vec.remove", 2, args.size(), loc);
            }

            auto* vec = std::get_if<Eigen::VectorXd>(&args[0]);
            if (!vec) {
                throw ArgumentError("vec.remove", "first argument must be vec", loc);
            }

            auto* idx_ptr = std::get_if<int64_t>(&args[1]);
            if (!idx_ptr) {
                throw ArgumentError("vec.remove", "second argument must be int", loc);
            }

            size_t idx = static_cast<size_t>(*idx_ptr);
            if (idx >= static_cast<size_t>(vec->size())) {
                throw RuntimeError::indexOutOfBounds(idx, vec->size(), loc);
            }

            if (vec->size() == 1) {
                throw RuntimeError("Cannot remove element from vector of size 1", loc);
            }

            // Crear nuevo vector sin el elemento en idx
            Eigen::VectorXd result(vec->size() - 1);
            if (idx > 0) {
                result.head(idx) = vec->head(idx);
            }
            if (idx < static_cast<size_t>(vec->size() - 1)) {
                result.tail(vec->size() - idx - 1) = vec->tail(vec->size() - idx - 1);
            }

            return result;
        };

        // vec.reverse(vector) -> vec
        // Retorna un nuevo vector con los elementos en orden inverso
        vecModule["reverse"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("vec.reverse", 1, args.size(), loc);
            }

            auto* vec = std::get_if<Eigen::VectorXd>(&args[0]);
            if (!vec) {
                throw ArgumentError("vec.reverse", "argument must be vec", loc);
            }

            Eigen::VectorXd result = vec->reverse();
            return result;
        };

        // vec.insert(vector, index, value) -> vec
        // Retorna un nuevo vector con el valor insertado en la posición especificada
        vecModule["insert"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 3) {
                throw ArgumentError::wrongCount("vec.insert", 3, args.size(), loc);
            }

            auto* vec = std::get_if<Eigen::VectorXd>(&args[0]);
            if (!vec) {
                throw ArgumentError("vec.insert", "first argument must be vec", loc);
            }

            auto* idx_ptr = std::get_if<int64_t>(&args[1]);
            if (!idx_ptr) {
                throw ArgumentError("vec.insert", "second argument must be int", loc);
            }

            size_t idx = static_cast<size_t>(*idx_ptr);
            if (idx > static_cast<size_t>(vec->size())) {
                throw RuntimeError("Insert index out of bounds (must be <= vector size)", loc);
            }

            // Verificar que el tercer argumento sea un escalar
            if (!std::holds_alternative<int64_t>(args[2]) &&
                !std::holds_alternative<double>(args[2])) {
                throw ArgumentError("vec.insert", "third argument must be a scalar (int or float64)", loc);
            }

            double value = std::visit([](auto&& arg) -> double {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, int64_t>) {
                    return static_cast<double>(arg);
                } else if constexpr (std::is_same_v<T, double>) {
                    return arg;
                }
                return 0.0;
            }, args[2]);

            // Crear nuevo vector con el elemento insertado
            Eigen::VectorXd result(vec->size() + 1);
            if (idx > 0) {
                result.head(idx) = vec->head(idx);
            }
            result(idx) = value;
            if (idx < static_cast<size_t>(vec->size())) {
                result.tail(vec->size() - idx) = vec->tail(vec->size() - idx);
            }

            return result;
        };

        // vec.size(vector) -> int
        // Alias de math.size para vectores (conveniencia)
        vecModule["size"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("vec.size", 1, args.size(), loc);
            }

            auto* vec = std::get_if<Eigen::VectorXd>(&args[0]);
            if (!vec) {
                throw ArgumentError("vec.size", "argument must be vec", loc);
            }

            return static_cast<int64_t>(vec->size());
        };

        // ====================================================================
        // Matrix Module Functions
        // ====================================================================

        auto& matModule = modules_["matrix"];

        // mat.append_row(matrix, row_vector) -> mat
        // Retorna una nueva matriz con la fila agregada al final
        matModule["append_row"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("mat.append_row", 2, args.size(), loc);
            }

            auto* mat = std::get_if<Eigen::MatrixXd>(&args[0]);
            if (!mat) {
                throw ArgumentError("mat.append_row", "first argument must be mat", loc);
            }

            auto* row = std::get_if<Eigen::VectorXd>(&args[1]);
            if (!row) {
                throw ArgumentError("mat.append_row", "second argument must be vec", loc);
            }

            if (row->size() != mat->cols()) {
                throw RuntimeError("Row vector size must match matrix column count", loc);
            }

            // Crear nueva matriz con una fila más
            Eigen::MatrixXd result(mat->rows() + 1, mat->cols());
            result.topRows(mat->rows()) = *mat;
            result.bottomRows(1) = row->transpose();

            return result;
        };

        // mat.append_col(matrix, col_vector) -> mat
        // Retorna una nueva matriz con la columna agregada al final
        matModule["append_col"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("mat.append_col", 2, args.size(), loc);
            }

            auto* mat = std::get_if<Eigen::MatrixXd>(&args[0]);
            if (!mat) {
                throw ArgumentError("mat.append_col", "first argument must be mat", loc);
            }

            auto* col = std::get_if<Eigen::VectorXd>(&args[1]);
            if (!col) {
                throw ArgumentError("mat.append_col", "second argument must be vec", loc);
            }

            if (col->size() != mat->rows()) {
                throw RuntimeError("Column vector size must match matrix row count", loc);
            }

            // Crear nueva matriz con una columna más
            Eigen::MatrixXd result(mat->rows(), mat->cols() + 1);
            result.leftCols(mat->cols()) = *mat;
            result.rightCols(1) = *col;

            return result;
        };

        // mat.remove_row(matrix, index) -> mat
        // Retorna una nueva matriz sin la fila en la posición especificada
        matModule["remove_row"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("mat.remove_row", 2, args.size(), loc);
            }

            auto* mat = std::get_if<Eigen::MatrixXd>(&args[0]);
            if (!mat) {
                throw ArgumentError("mat.remove_row", "first argument must be mat", loc);
            }

            auto* idx_ptr = std::get_if<int64_t>(&args[1]);
            if (!idx_ptr) {
                throw ArgumentError("mat.remove_row", "second argument must be int", loc);
            }

            size_t idx = static_cast<size_t>(*idx_ptr);
            if (idx >= static_cast<size_t>(mat->rows())) {
                throw RuntimeError::indexOutOfBounds(idx, mat->rows(), loc);
            }

            if (mat->rows() == 1) {
                throw RuntimeError("Cannot remove row from matrix with only 1 row", loc);
            }

            // Crear nueva matriz sin la fila en idx
            Eigen::MatrixXd result(mat->rows() - 1, mat->cols());
            if (idx > 0) {
                result.topRows(idx) = mat->topRows(idx);
            }
            if (idx < static_cast<size_t>(mat->rows() - 1)) {
                result.bottomRows(mat->rows() - idx - 1) = mat->bottomRows(mat->rows() - idx - 1);
            }

            return result;
        };

        // mat.remove_col(matrix, index) -> mat
        // Retorna una nueva matriz sin la columna en la posición especificada
        matModule["remove_col"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("mat.remove_col", 2, args.size(), loc);
            }

            auto* mat = std::get_if<Eigen::MatrixXd>(&args[0]);
            if (!mat) {
                throw ArgumentError("mat.remove_col", "first argument must be mat", loc);
            }

            auto* idx_ptr = std::get_if<int64_t>(&args[1]);
            if (!idx_ptr) {
                throw ArgumentError("mat.remove_col", "second argument must be int", loc);
            }

            size_t idx = static_cast<size_t>(*idx_ptr);
            if (idx >= static_cast<size_t>(mat->cols())) {
                throw RuntimeError::indexOutOfBounds(idx, mat->cols(), loc);
            }

            if (mat->cols() == 1) {
                throw RuntimeError("Cannot remove column from matrix with only 1 column", loc);
            }

            // Crear nueva matriz sin la columna en idx
            Eigen::MatrixXd result(mat->rows(), mat->cols() - 1);
            if (idx > 0) {
                result.leftCols(idx) = mat->leftCols(idx);
            }
            if (idx < static_cast<size_t>(mat->cols() - 1)) {
                result.rightCols(mat->cols() - idx - 1) = mat->rightCols(mat->cols() - idx - 1);
            }

            return result;
        };

        // mat.rows(matrix) -> int
        // Alias de math.rows (conveniencia)
        matModule["rows"] = mathModule["rows"];

        // mat.cols(matrix) -> int
        // Alias de math.cols (conveniencia)
        matModule["cols"] = mathModule["cols"];

        // mat.size(matrix) -> int
        // Alias de math.size (conveniencia)
        matModule["size"] = mathModule["size"];

        // ====================================================================
        // String Module Functions
        // ====================================================================

        auto& stringModule = modules_["string"];

        // string.length(str) -> int
        stringModule["length"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("string.length", 1, args.size(), loc);
            }

            auto* str = std::get_if<std::string>(&args[0]);
            if (!str) {
                throw ArgumentError("string.length", "argument must be string", loc);
            }

            return static_cast<int64_t>(str->length());
        };

        // string.lower(str) -> str
        stringModule["lower"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("string.lower", 1, args.size(), loc);
            }

            auto* str = std::get_if<std::string>(&args[0]);
            if (!str) {
                throw ArgumentError("string.lower", "argument must be string", loc);
            }

            std::string result = *str;
            for (char& c : result) {
                c = std::tolower(static_cast<unsigned char>(c));
            }
            return result;
        };

        // string.upper(str) -> str
        stringModule["upper"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("string.upper", 1, args.size(), loc);
            }

            auto* str = std::get_if<std::string>(&args[0]);
            if (!str) {
                throw ArgumentError("string.upper", "argument must be string", loc);
            }

            std::string result = *str;
            for (char& c : result) {
                c = std::toupper(static_cast<unsigned char>(c));
            }
            return result;
        };

        // string.substr(str, start, length) -> str
        stringModule["substr"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 3) {
                throw ArgumentError::wrongCount("string.substr", 3, args.size(), loc);
            }

            auto* str = std::get_if<std::string>(&args[0]);
            if (!str) {
                throw ArgumentError("string.substr", "first argument must be string", loc);
            }

            int64_t start = toInt(args[1]);
            int64_t length = toInt(args[2]);

            if (start < 0 || start >= static_cast<int64_t>(str->length())) {
                throw ArgumentError("string.substr", "start index out of bounds", loc);
            }

            if (length < 0) {
                throw ArgumentError("string.substr", "length must be non-negative", loc);
            }

            return str->substr(start, length);
        };

        // string.find(str, substr) -> int (returns -1 if not found)
        stringModule["find"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("string.find", 2, args.size(), loc);
            }

            auto* str = std::get_if<std::string>(&args[0]);
            auto* substr = std::get_if<std::string>(&args[1]);

            if (!str || !substr) {
                throw ArgumentError("string.find", "both arguments must be string", loc);
            }

            size_t pos = str->find(*substr);
            if (pos == std::string::npos) {
                return int64_t(-1);
            }
            return static_cast<int64_t>(pos);
        };

        // string.replace(str, old, new) -> str
        stringModule["replace"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 3) {
                throw ArgumentError::wrongCount("string.replace", 3, args.size(), loc);
            }

            auto* str = std::get_if<std::string>(&args[0]);
            auto* oldStr = std::get_if<std::string>(&args[1]);
            auto* newStr = std::get_if<std::string>(&args[2]);

            if (!str || !oldStr || !newStr) {
                throw ArgumentError("string.replace", "all arguments must be string", loc);
            }

            std::string result = *str;
            if (oldStr->empty()) return result;

            size_t pos = 0;
            while ((pos = result.find(*oldStr, pos)) != std::string::npos) {
                result.replace(pos, oldStr->length(), *newStr);
                pos += newStr->length();
            }
            return result;
        };

        // string.trim(str) -> str (removes leading and trailing whitespace)
        stringModule["trim"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("string.trim", 1, args.size(), loc);
            }

            auto* str = std::get_if<std::string>(&args[0]);
            if (!str) {
                throw ArgumentError("string.trim", "argument must be string", loc);
            }

            std::string result = *str;
            // Trim leading whitespace
            size_t start = result.find_first_not_of(" \t\n\r\f\v");
            if (start == std::string::npos) {
                return std::string("");
            }
            result = result.substr(start);

            // Trim trailing whitespace
            size_t end = result.find_last_not_of(" \t\n\r\f\v");
            result = result.substr(0, end + 1);

            return result;
        };

        // string.startswith(str, prefix) -> int (1 for true, 0 for false)
        stringModule["startswith"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("string.startswith", 2, args.size(), loc);
            }

            auto* str = std::get_if<std::string>(&args[0]);
            auto* prefix = std::get_if<std::string>(&args[1]);

            if (!str || !prefix) {
                throw ArgumentError("string.startswith", "both arguments must be string", loc);
            }

            if (str->length() >= prefix->length() &&
                str->substr(0, prefix->length()) == *prefix) {
                return int64_t(1);
            }
            return int64_t(0);
        };

        // string.endswith(str, suffix) -> int (1 for true, 0 for false)
        stringModule["endswith"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("string.endswith", 2, args.size(), loc);
            }

            auto* str = std::get_if<std::string>(&args[0]);
            auto* suffix = std::get_if<std::string>(&args[1]);

            if (!str || !suffix) {
                throw ArgumentError("string.endswith", "both arguments must be string", loc);
            }

            if (str->length() >= suffix->length() &&
                str->substr(str->length() - suffix->length()) == *suffix) {
                return int64_t(1);
            }
            return int64_t(0);
        };

        // string.contains(str, substr) -> int (1 for true, 0 for false)
        stringModule["contains"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("string.contains", 2, args.size(), loc);
            }

            auto* str = std::get_if<std::string>(&args[0]);
            auto* substr = std::get_if<std::string>(&args[1]);

            if (!str || !substr) {
                throw ArgumentError("string.contains", "both arguments must be string", loc);
            }

            return str->find(*substr) != std::string::npos ? int64_t(1) : int64_t(0);
        };

        // string.split(str, delimiter) -> vec (returns vector of strings as a special encoding)
        // Note: This is a simplified version. A proper implementation would need a string array type.
        stringModule["split"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("string.split", 2, args.size(), loc);
            }

            auto* str = std::get_if<std::string>(&args[0]);
            auto* delim = std::get_if<std::string>(&args[1]);

            if (!str || !delim) {
                throw ArgumentError("string.split", "both arguments must be string", loc);
            }

            if (delim->empty()) {
                throw ArgumentError("string.split", "delimiter cannot be empty", loc);
            }

            // For now, we'll return the count of splits as this needs array support
            // This is a placeholder implementation
            size_t count = 0;
            size_t pos = 0;
            std::string temp = *str;

            while ((pos = temp.find(*delim)) != std::string::npos) {
                count++;
                temp = temp.substr(pos + delim->length());
            }
            count++; // Last segment

            return static_cast<int64_t>(count);
        };

        // string.join(vec_of_strings, separator) -> str
        // Note: Simplified - would need proper string array support
        stringModule["join"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("string.join", 2, args.size(), loc);
            }

            auto* separator = std::get_if<std::string>(&args[1]);
            if (!separator) {
                throw ArgumentError("string.join", "second argument must be string", loc);
            }

            // Placeholder implementation
            return std::string("join_placeholder");
        };

        // string.reverse(str) -> str
        stringModule["reverse"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("string.reverse", 1, args.size(), loc);
            }

            auto* str = std::get_if<std::string>(&args[0]);
            if (!str) {
                throw ArgumentError("string.reverse", "argument must be string", loc);
            }

            std::string result = *str;
            std::reverse(result.begin(), result.end());
            return result;
        };

        // string.repeat(str, count) -> str
        stringModule["repeat"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("string.repeat", 2, args.size(), loc);
            }

            auto* str = std::get_if<std::string>(&args[0]);
            if (!str) {
                throw ArgumentError("string.repeat", "first argument must be string", loc);
            }

            int64_t count = toInt(args[1]);
            if (count < 0) {
                throw ArgumentError("string.repeat", "count must be non-negative", loc);
            }

            std::string result;
            result.reserve(str->length() * count);
            for (int64_t i = 0; i < count; ++i) {
                result += *str;
            }
            return result;
        };

        // string.at(str, index) -> str (single character as string)
        stringModule["at"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("string.at", 2, args.size(), loc);
            }

            auto* str = std::get_if<std::string>(&args[0]);
            if (!str) {
                throw ArgumentError("string.at", "first argument must be string", loc);
            }

            int64_t index = toInt(args[1]);
            if (index < 0 || index >= static_cast<int64_t>(str->length())) {
                throw ArgumentError("string.at", "index out of bounds", loc);
            }

            return std::string(1, (*str)[index]);
        };

        // string.concat(str1, str2, ...) -> str
        stringModule["concat"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.empty()) {
                throw ArgumentError("string.concat", "expected at least 1 argument", loc);
            }

            std::string result;
            for (const auto& arg : args) {
                auto* str = std::get_if<std::string>(&arg);
                if (!str) {
                    throw ArgumentError("string.concat", "all arguments must be string", loc);
                }
                result += *str;
            }
            return result;
        };

        // string.isempty(str) -> int (1 for true, 0 for false)
        stringModule["isempty"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("string.isempty", 1, args.size(), loc);
            }

            auto* str = std::get_if<std::string>(&args[0]);
            if (!str) {
                throw ArgumentError("string.isempty", "argument must be string", loc);
            }

            return str->empty() ? int64_t(1) : int64_t(0);
        };

        // string.count(str, substr) -> int
        stringModule["count"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("string.count", 2, args.size(), loc);
            }

            auto* str = std::get_if<std::string>(&args[0]);
            auto* substr = std::get_if<std::string>(&args[1]);

            if (!str || !substr) {
                throw ArgumentError("string.count", "both arguments must be string", loc);
            }

            if (substr->empty()) {
                return static_cast<int64_t>(str->length() + 1);
            }

            int64_t count = 0;
            size_t pos = 0;
            while ((pos = str->find(*substr, pos)) != std::string::npos) {
                count++;
                pos += substr->length();
            }
            return count;
        };

        // ====================================================================
        // Mapping Module Functions
        // ====================================================================

        auto& mappingModule = modules_["mapping"];

        // mapping.create() -> map (create empty map)
        mappingModule["create"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 0) {
                throw ArgumentError::wrongCount("mapping.create", 0, args.size(), loc);
            }
            return std::make_shared<RhoMap>();
        };

        // mapping.set(map, key, value) -> map (returns the map for chaining)
        mappingModule["set"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 3) {
                throw ArgumentError::wrongCount("mapping.set", 3, args.size(), loc);
            }

            auto* mapPtr = std::get_if<std::shared_ptr<RhoMap>>(&args[0]);
            if (!mapPtr) {
                throw ArgumentError("mapping.set", "first argument must be map", loc);
            }

            auto* key = std::get_if<std::string>(&args[1]);
            if (!key) {
                throw ArgumentError("mapping.set", "second argument (key) must be string", loc);
            }

            (*mapPtr)->set(*key, args[2]);
            return *mapPtr;
        };

        // mapping.get(map, key) -> value
        mappingModule["get"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("mapping.get", 2, args.size(), loc);
            }

            auto* mapPtr = std::get_if<std::shared_ptr<RhoMap>>(&args[0]);
            if (!mapPtr) {
                throw ArgumentError("mapping.get", "first argument must be map", loc);
            }

            auto* key = std::get_if<std::string>(&args[1]);
            if (!key) {
                throw ArgumentError("mapping.get", "second argument (key) must be string", loc);
            }

            return (*mapPtr)->get(*key);
        };

        // mapping.has(map, key) -> int (1 if exists, 0 otherwise)
        mappingModule["has"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("mapping.has", 2, args.size(), loc);
            }

            auto* mapPtr = std::get_if<std::shared_ptr<RhoMap>>(&args[0]);
            if (!mapPtr) {
                throw ArgumentError("mapping.has", "first argument must be map", loc);
            }

            auto* key = std::get_if<std::string>(&args[1]);
            if (!key) {
                throw ArgumentError("mapping.has", "second argument (key) must be string", loc);
            }

            return (*mapPtr)->has(*key) ? int64_t(1) : int64_t(0);
        };

        // mapping.delete(map, key) -> int (1 if deleted, 0 if not found)
        mappingModule["delete"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("mapping.delete", 2, args.size(), loc);
            }

            auto* mapPtr = std::get_if<std::shared_ptr<RhoMap>>(&args[0]);
            if (!mapPtr) {
                throw ArgumentError("mapping.delete", "first argument must be map", loc);
            }

            auto* key = std::get_if<std::string>(&args[1]);
            if (!key) {
                throw ArgumentError("mapping.delete", "second argument (key) must be string", loc);
            }

            return (*mapPtr)->remove(*key) ? int64_t(1) : int64_t(0);
        };

        // mapping.size(map) -> int
        mappingModule["size"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("mapping.size", 1, args.size(), loc);
            }

            auto* mapPtr = std::get_if<std::shared_ptr<RhoMap>>(&args[0]);
            if (!mapPtr) {
                throw ArgumentError("mapping.size", "argument must be map", loc);
            }

            return static_cast<int64_t>((*mapPtr)->size());
        };

        // mapping.isempty(map) -> int (1 if empty, 0 otherwise)
        mappingModule["isempty"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("mapping.isempty", 1, args.size(), loc);
            }

            auto* mapPtr = std::get_if<std::shared_ptr<RhoMap>>(&args[0]);
            if (!mapPtr) {
                throw ArgumentError("mapping.isempty", "argument must be map", loc);
            }

            return (*mapPtr)->empty() ? int64_t(1) : int64_t(0);
        };

        // mapping.clear(map) -> map (returns the map)
        mappingModule["clear"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("mapping.clear", 1, args.size(), loc);
            }

            auto* mapPtr = std::get_if<std::shared_ptr<RhoMap>>(&args[0]);
            if (!mapPtr) {
                throw ArgumentError("mapping.clear", "argument must be map", loc);
            }

            (*mapPtr)->clear();
            return *mapPtr;
        };

        // mapping.keys(map) -> vec (vector of key indices/count - simplified)
        // Note: This returns the count of keys since we don't have string arrays yet
        mappingModule["keys"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("mapping.keys", 1, args.size(), loc);
            }

            auto* mapPtr = std::get_if<std::shared_ptr<RhoMap>>(&args[0]);
            if (!mapPtr) {
                throw ArgumentError("mapping.keys", "argument must be map", loc);
            }

            // For now, return count as we need string array support
            return static_cast<int64_t>((*mapPtr)->size());
        };

        // mapping.values(map) -> vec (vector of values - simplified)
        // Note: This returns the count of values since we don't have proper array support yet
        mappingModule["values"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("mapping.values", 1, args.size(), loc);
            }

            auto* mapPtr = std::get_if<std::shared_ptr<RhoMap>>(&args[0]);
            if (!mapPtr) {
                throw ArgumentError("mapping.values", "argument must be map", loc);
            }

            // For now, return count as we need mixed-type array support
            return static_cast<int64_t>((*mapPtr)->size());
        };

        // mapping.update(map1, map2) -> map1 (merge map2 into map1)
        mappingModule["update"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("mapping.update", 2, args.size(), loc);
            }

            auto* map1Ptr = std::get_if<std::shared_ptr<RhoMap>>(&args[0]);
            auto* map2Ptr = std::get_if<std::shared_ptr<RhoMap>>(&args[1]);

            if (!map1Ptr || !map2Ptr) {
                throw ArgumentError("mapping.update", "both arguments must be map", loc);
            }

            // Copy all entries from map2 to map1
            for (auto it = (*map2Ptr)->begin(); it != (*map2Ptr)->end(); ++it) {
                (*map1Ptr)->set(it->first, it->second);
            }

            return *map1Ptr;
        };

        // mapping.copy(map) -> map (shallow copy)
        mappingModule["copy"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("mapping.copy", 1, args.size(), loc);
            }

            auto* mapPtr = std::get_if<std::shared_ptr<RhoMap>>(&args[0]);
            if (!mapPtr) {
                throw ArgumentError("mapping.copy", "argument must be map", loc);
            }

            auto newMap = std::make_shared<RhoMap>();
            for (auto it = (*mapPtr)->begin(); it != (*mapPtr)->end(); ++it) {
                newMap->set(it->first, it->second);
            }

            return newMap;
        };

        // ====================================================================
        // Array Module Functions
        // ====================================================================

        auto& arrayModule = modules_["array"];

        // array.create(size) -> arr (create array with initial size)
        arrayModule["create"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("array.create", 1, args.size(), loc);
            }

            size_t size = static_cast<size_t>(toInt(args[0]));
            return std::make_shared<RhoArray>(size);
        };

        // array.empty() -> arr (create empty array)
        arrayModule["empty"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 0) {
                throw ArgumentError::wrongCount("array.empty", 0, args.size(), loc);
            }
            return std::make_shared<RhoArray>();
        };

        // array.push(arr, value) -> arr (append value to end)
        arrayModule["push"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("array.push", 2, args.size(), loc);
            }

            auto* arrPtr = std::get_if<std::shared_ptr<RhoArray>>(&args[0]);
            if (!arrPtr) {
                throw ArgumentError("array.push", "first argument must be arr", loc);
            }

            (*arrPtr)->push(args[1]);
            return *arrPtr;
        };

        // array.pop(arr) -> value (remove and return last element)
        arrayModule["pop"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("array.pop", 1, args.size(), loc);
            }

            auto* arrPtr = std::get_if<std::shared_ptr<RhoArray>>(&args[0]);
            if (!arrPtr) {
                throw ArgumentError("array.pop", "argument must be arr", loc);
            }

            return (*arrPtr)->pop();
        };

        // array.size(arr) -> int
        arrayModule["size"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("array.size", 1, args.size(), loc);
            }

            auto* arrPtr = std::get_if<std::shared_ptr<RhoArray>>(&args[0]);
            if (!arrPtr) {
                throw ArgumentError("array.size", "argument must be arr", loc);
            }

            return static_cast<int64_t>((*arrPtr)->size());
        };

        // array.isempty(arr) -> bool
        arrayModule["isempty"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("array.isempty", 1, args.size(), loc);
            }

            auto* arrPtr = std::get_if<std::shared_ptr<RhoArray>>(&args[0]);
            if (!arrPtr) {
                throw ArgumentError("array.isempty", "argument must be arr", loc);
            }

            return (*arrPtr)->empty();
        };

        // array.clear(arr) -> arr
        arrayModule["clear"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("array.clear", 1, args.size(), loc);
            }

            auto* arrPtr = std::get_if<std::shared_ptr<RhoArray>>(&args[0]);
            if (!arrPtr) {
                throw ArgumentError("array.clear", "argument must be arr", loc);
            }

            (*arrPtr)->clear();
            return *arrPtr;
        };

        // array.resize(arr, newSize) -> arr
        arrayModule["resize"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("array.resize", 2, args.size(), loc);
            }

            auto* arrPtr = std::get_if<std::shared_ptr<RhoArray>>(&args[0]);
            if (!arrPtr) {
                throw ArgumentError("array.resize", "first argument must be arr", loc);
            }

            size_t newSize = static_cast<size_t>(toInt(args[1]));
            (*arrPtr)->resize(newSize);
            return *arrPtr;
        };

        // ====================================================================
        // IO Module Functions
        // ====================================================================

        auto& ioModule = modules_["io"];

        // io.input(prompt) -> str
        // Read a line from standard input with optional prompt
        ioModule["input"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            std::string prompt = "";

            if (args.size() > 1) {
                throw ArgumentError("io.input", "expected 0 or 1 arguments", loc);
            }

            if (args.size() == 1) {
                auto* str = std::get_if<std::string>(&args[0]);
                if (!str) {
                    throw ArgumentError("io.input", "prompt must be string", loc);
                }
                prompt = *str;
            }

            // Print prompt if provided
            if (!prompt.empty()) {
                std::cout << prompt;
                std::cout.flush();
            }

            std::string line;
            if (!std::getline(std::cin, line)) {
                // EOF reached
                return std::string("");
            }

            return line;
        };

        // io.open(filename, mode) -> int (file handle)
        // Open a file and return handle
        // Modes: "r" (read), "w" (write), "a" (append), "r+" (read/write), "w+" (write/read), "a+" (append/read)
        ioModule["open"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("io.open", 2, args.size(), loc);
            }

            auto* filename = std::get_if<std::string>(&args[0]);
            auto* mode_str = std::get_if<std::string>(&args[1]);

            if (!filename) {
                throw ArgumentError("io.open", "filename must be string", loc);
            }
            if (!mode_str) {
                throw ArgumentError("io.open", "mode must be string", loc);
            }

            // Parse mode string
            std::ios::openmode mode = std::ios::binary; // Default to binary mode
            if (*mode_str == "r") {
                mode = std::ios::in;
            } else if (*mode_str == "w") {
                mode = std::ios::out | std::ios::trunc;
            } else if (*mode_str == "a") {
                mode = std::ios::out | std::ios::app;
            } else if (*mode_str == "r+") {
                mode = std::ios::in | std::ios::out;
            } else if (*mode_str == "w+") {
                mode = std::ios::in | std::ios::out | std::ios::trunc;
            } else if (*mode_str == "a+") {
                mode = std::ios::in | std::ios::out | std::ios::app;
            } else {
                throw ArgumentError("io.open", "invalid mode '" + *mode_str + "' (use r, w, a, r+, w+, a+)", loc);
            }

            int64_t handle = FileHandleManager::instance().openFile(*filename, mode);
            FileHandle* fh = FileHandleManager::instance().getHandle(handle);

            if (!fh || !fh->is_open) {
                throw RuntimeError("io.open: Cannot open file '" + *filename + "'", loc);
            }

            return handle;
        };

        // io.read(handle) -> str (read entire file)
        // io.read(handle, size) -> str (read size bytes)
        ioModule["read"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.empty() || args.size() > 2) {
                throw ArgumentError("io.read", "expected 1 or 2 arguments", loc);
            }

            int64_t handle = toInt(args[0]);
            FileHandle* fh = FileHandleManager::instance().getHandle(handle);

            if (!fh || !fh->is_open) {
                throw RuntimeError("io.read: invalid file handle", loc);
            }

            if (args.size() == 1) {
                // Read entire file
                std::stringstream buffer;
                buffer << fh->stream.rdbuf();
                return buffer.str();
            } else {
                // Read specified number of bytes
                int64_t size = toInt(args[1]);
                if (size < 0) {
                    throw ArgumentError("io.read", "size must be non-negative", loc);
                }

                std::string result(size, '\0');
                fh->stream.read(&result[0], size);
                std::streamsize bytesRead = fh->stream.gcount();
                result.resize(bytesRead);
                return result;
            }
        };

        // io.write(handle, content) -> int (bytes written)
        ioModule["write"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 2) {
                throw ArgumentError::wrongCount("io.write", 2, args.size(), loc);
            }

            int64_t handle = toInt(args[0]);
            auto* content = std::get_if<std::string>(&args[1]);

            if (!content) {
                throw ArgumentError("io.write", "content must be string", loc);
            }

            FileHandle* fh = FileHandleManager::instance().getHandle(handle);
            if (!fh || !fh->is_open) {
                throw RuntimeError("io.write: invalid file handle", loc);
            }

            fh->stream << *content;
            fh->stream.flush();

            if (fh->stream.fail()) {
                throw RuntimeError("io.write: error writing to file", loc);
            }

            return static_cast<int64_t>(content->length());
        };

        // io.readline(handle) -> str
        // Read a single line from file
        ioModule["readline"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("io.readline", 1, args.size(), loc);
            }

            int64_t handle = toInt(args[0]);
            FileHandle* fh = FileHandleManager::instance().getHandle(handle);

            if (!fh || !fh->is_open) {
                throw RuntimeError("io.readline: invalid file handle", loc);
            }

            std::string line;
            if (std::getline(fh->stream, line)) {
                return line;
            }
            return std::string("");
        };

        // io.seek(handle, offset, whence) -> int (new position)
        // whence: 0=begin, 1=current, 2=end
        ioModule["seek"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() < 2 || args.size() > 3) {
                throw ArgumentError("io.seek", "expected 2 or 3 arguments", loc);
            }

            int64_t handle = toInt(args[0]);
            int64_t offset = toInt(args[1]);
            int64_t whence = args.size() == 3 ? toInt(args[2]) : 0;

            FileHandle* fh = FileHandleManager::instance().getHandle(handle);
            if (!fh || !fh->is_open) {
                throw RuntimeError("io.seek: invalid file handle", loc);
            }

            std::ios::seekdir dir;
            if (whence == 0) dir = std::ios::beg;
            else if (whence == 1) dir = std::ios::cur;
            else if (whence == 2) dir = std::ios::end;
            else {
                throw ArgumentError("io.seek", "whence must be 0 (begin), 1 (current), or 2 (end)", loc);
            }

            fh->stream.seekg(offset, dir);
            fh->stream.seekp(offset, dir);

            return static_cast<int64_t>(fh->stream.tellg());
        };

        // io.tell(handle) -> int (current position)
        ioModule["tell"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("io.tell", 1, args.size(), loc);
            }

            int64_t handle = toInt(args[0]);
            FileHandle* fh = FileHandleManager::instance().getHandle(handle);

            if (!fh || !fh->is_open) {
                throw RuntimeError("io.tell: invalid file handle", loc);
            }

            return static_cast<int64_t>(fh->stream.tellg());
        };

        // io.close(handle) -> int (1 if success, 0 if failed)
        ioModule["close"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("io.close", 1, args.size(), loc);
            }

            int64_t handle = toInt(args[0]);
            bool success = FileHandleManager::instance().closeFile(handle);
            return success ? int64_t(1) : int64_t(0);
        };

        // io.flush(handle) -> int (1 if success)
        ioModule["flush"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("io.flush", 1, args.size(), loc);
            }

            int64_t handle = toInt(args[0]);
            FileHandle* fh = FileHandleManager::instance().getHandle(handle);

            if (!fh || !fh->is_open) {
                throw RuntimeError("io.flush: invalid file handle", loc);
            }

            fh->stream.flush();
            return int64_t(1);
        };

        // Utility functions (work without handles for convenience)

        // io.exists(filename) -> int (1 if exists, 0 otherwise)
        ioModule["exists"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("io.exists", 1, args.size(), loc);
            }

            auto* filename = std::get_if<std::string>(&args[0]);
            if (!filename) {
                throw ArgumentError("io.exists", "filename must be string", loc);
            }

            std::ifstream file(*filename);
            return file.good() ? int64_t(1) : int64_t(0);
        };

        // io.remove(filename) -> int (1 if successful, 0 otherwise)
        ioModule["remove"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
            if (args.size() != 1) {
                throw ArgumentError::wrongCount("io.remove", 1, args.size(), loc);
            }

            auto* filename = std::get_if<std::string>(&args[0]);
            if (!filename) {
                throw ArgumentError("io.remove", "filename must be string", loc);
            }

            int result = std::remove(filename->c_str());
            return result == 0 ? int64_t(1) : int64_t(0);
        };

        // Expose math module functions as globals so they can be called without 'math.' prefix
        for (auto& [name, fn] : modules_["math"]) {
            if (functions_.find(name) == functions_.end()) {
                functions_[name] = fn;
            }
        }
        // Also expose vector, matrix module functions as globals
        for (auto& [name, fn] : modules_["vector"]) {
            if (functions_.find(name) == functions_.end()) {
                functions_[name] = fn;
            }
        }
        for (auto& [name, fn] : modules_["matrix"]) {
            if (functions_.find(name) == functions_.end()) {
                functions_[name] = fn;
            }
        }
    }
};

} // namespace Rhodesia

#endif // RHODESIA_BUILTINS_HPP
