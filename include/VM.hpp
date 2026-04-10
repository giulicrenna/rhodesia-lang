/**
 * @file VM.hpp
 * @brief Bytecode virtual machine for the Rhodesia language.
 *
 * The VM executes compiled Chunk objects produced by the Compiler.  It uses a
 * value stack, a call-frame stack, a try-frame stack, and a separate iterator
 * stack for for-loop support.
 *
 * RhoFunction objects that carry compiled bytecode store a shared_ptr<Chunk>
 * via the existing bodyNode_ field (as a shared_ptr<void> that actually points
 * to a Chunk) and a vector of captured RhoValues in the closure_ map under the
 * synthetic keys "__upvalue_0", "__upvalue_1", … .  The helpers compiledChunk()
 * and upvalues() below encapsulate those conventions so the rest of the VM does
 * not need to know the encoding.
 */

#pragma once
#ifndef RHODESIA_VM_HPP
#define RHODESIA_VM_HPP

#include "Bytecode.hpp"
#include "RhoValue.hpp"
#include "Builtins.hpp"
#include "Error.hpp"

#include <Eigen/Dense>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace Rhodesia {

// ---------------------------------------------------------------------------
// VMError
// ---------------------------------------------------------------------------

class VMError : public std::runtime_error {
public:
    explicit VMError(const std::string& msg)
        : std::runtime_error("VMError: " + msg) {}
};

// ---------------------------------------------------------------------------
// Helpers to work with compiled RhoFunction objects
//
// Convention used by the Compiler (and expected by the VM):
//   - A "compiled" RhoFunction has its bodyNode_ pointing to a Chunk
//     (wrapped in a no-op-deleter shared_ptr<void>).
//   - Captured upvalues are stored as a std::vector<RhoValue> pointed to by
//     a shared_ptr stored at key "__upvalue_vec__" in the closure_ map as a
//     native-function wrapper around the vector.
//
// Because modifying RhoValue.hpp is out of scope here we expose two free
// functions that encapsulate the encoding:
//
//   compiledChunk(f)  -> shared_ptr<Chunk> or nullptr
//   upvalues(f)       -> vector<RhoValue>&  (mutable reference)
//   setUpvalues(f, v) -> store a vector of upvalues into f
//   isCompiled(f)     -> true if f carries bytecode
// ---------------------------------------------------------------------------

/**
 * @brief A small box that lets us store a mutable vector<RhoValue> as a
 *        native-function-like closure entry inside RhoFunction.
 *
 * We allocate one UpvalueBox per compiled closure and store a shared_ptr to
 * it inside a thin NativeFunc wrapper so the shared_ptr keeps it alive as long
 * as any RhoFunction referencing it exists.
 */
struct UpvalueBox {
    std::vector<RhoValue> values;
};

// ---------------------------------------------------------------------------
// Global registry for UpvalueBox objects (keyed by a monotonically
// increasing ID stored in the RhoFunction closure as "__upvalue_id__").
// ---------------------------------------------------------------------------

namespace detail {

inline std::unordered_map<int64_t, std::shared_ptr<UpvalueBox>>& upvalueRegistry() {
    static std::unordered_map<int64_t, std::shared_ptr<UpvalueBox>> reg;
    return reg;
}

inline int64_t nextUpvalueId() {
    static int64_t counter = 1;
    return counter++;
}

} // namespace detail

/**
 * @brief Create a compiled RhoFunction from a Chunk and an upvalue vector.
 *
 * This is called by MAKE_CLOSURE in the VM.
 *
 * @param chunk        The compiled chunk (function body).
 * @param upvalues     Initial captured upvalues (may be empty).
 * @return             A new RhoFunction carrying the chunk and upvalues.
 */
inline std::shared_ptr<RhoFunction> makeCompiledFunction(
    std::shared_ptr<Chunk> chunk,
    std::vector<RhoValue> upvalues)
{
    // Box the upvalues in the registry.
    int64_t uid = detail::nextUpvalueId();
    auto box = std::make_shared<UpvalueBox>();
    box->values = std::move(upvalues);
    detail::upvalueRegistry()[uid] = box;

    // Build a closure map that marks this function as compiled.
    std::unordered_map<std::string, RhoValue> cl;
    cl["__compiled__"] = int64_t(1);
    cl["__upvalue_id__"] = uid;

    // Build parameter name list from chunk arity.
    std::vector<std::string> params;
    params.reserve(static_cast<size_t>(chunk->arity));
    for (int i = 0; i < chunk->arity; ++i) {
        params.push_back("__p" + std::to_string(i));
    }

    // Wrap the Chunk pointer as a shared_ptr<void> with no-op deleter so
    // the Chunk's actual shared_ptr (owned by the constant pool) keeps it
    // alive independently.
    std::shared_ptr<void> bodyVoid(chunk, static_cast<void*>(chunk.get()));

    return std::make_shared<RhoFunction>(std::move(params), std::move(bodyVoid),
                                        false, std::move(cl));
}

/**
 * @brief Check whether a RhoFunction carries bytecode.
 */
inline bool isCompiledFunction(const RhoFunction& f) {
    return f.closure().count("__compiled__") != 0;
}

/**
 * @brief Retrieve the Chunk of a compiled RhoFunction.  Returns nullptr if
 *        the function is not compiled.
 */
inline std::shared_ptr<Chunk> compiledChunk(const RhoFunction& f) {
    if (!isCompiledFunction(f)) return nullptr;
    void* raw = f.body().get();
    if (!raw) return nullptr;
    return std::shared_ptr<Chunk>(f.body(), static_cast<Chunk*>(raw));
}

/**
 * @brief Return the UpvalueBox for a compiled RhoFunction, or nullptr.
 */
inline std::shared_ptr<UpvalueBox> upvalueBoxOf(const RhoFunction& f) {
    const auto& cl = f.closure();
    auto it = cl.find("__upvalue_id__");
    if (it == cl.end()) return nullptr;
    if (!std::holds_alternative<int64_t>(it->second)) return nullptr;
    int64_t uid = std::get<int64_t>(it->second);
    auto& reg = detail::upvalueRegistry();
    auto rit = reg.find(uid);
    if (rit == reg.end()) return nullptr;
    return rit->second;
}

// ---------------------------------------------------------------------------
// CallFrame
// ---------------------------------------------------------------------------

struct CallFrame {
    std::shared_ptr<Chunk>    chunk;    ///< Currently executing chunk
    size_t                    ip;       ///< Instruction pointer (index into chunk->code)
    size_t                    bp;       ///< Base pointer: index of arg0 on stack_
    std::shared_ptr<RhoFunction> closure; ///< Current closure (nullptr for top-level)
};

// ---------------------------------------------------------------------------
// TryFrame
// ---------------------------------------------------------------------------

struct TryFrame {
    int32_t catchTarget;  ///< Absolute instruction index of the catch handler
    size_t  stackDepth;   ///< stack_.size() to restore on exception
    size_t  frameDepth;   ///< frames_.size() to restore on exception
    std::string catchVar; ///< (informational; actual binding done by CATCH_BIND)
};

// ---------------------------------------------------------------------------
// VM
// ---------------------------------------------------------------------------

class VM {
public:
    // -----------------------------------------------------------------------
    // Public API
    // -----------------------------------------------------------------------

    /**
     * @brief Execute a top-level chunk and return the final value.
     *
     * The main chunk is treated like a function with arity 0.  After execution
     * the return value (or null if the stack is empty) is returned.
     */
    RhoValue run(std::shared_ptr<Chunk> mainChunk) {
        stack_.clear();
        frames_.clear();
        tryStack_.clear();
        iterStack_.clear();

        // Reserve locals for the main chunk.
        size_t numLocals = static_cast<size_t>(mainChunk->numLocals);
        stack_.resize(numLocals, RhoValue{std::make_shared<RhoNull>()});

        frames_.push_back(CallFrame{
            mainChunk,
            /*ip=*/0,
            /*bp=*/0,
            /*closure=*/nullptr
        });

        return execute();
    }

private:
    // -----------------------------------------------------------------------
    // VM state
    // -----------------------------------------------------------------------
    std::vector<RhoValue>                    stack_;
    std::vector<CallFrame>                   frames_;
    std::vector<TryFrame>                    tryStack_;
    std::unordered_map<std::string, RhoValue> globals_;
    std::vector<std::shared_ptr<RhoIterator>> iterStack_;

    // -----------------------------------------------------------------------
    // Stack helpers
    // -----------------------------------------------------------------------

    void push(RhoValue v) {
        stack_.push_back(std::move(v));
    }

    RhoValue pop() {
        if (stack_.empty()) {
            throw VMError("Stack underflow");
        }
        RhoValue v = std::move(stack_.back());
        stack_.pop_back();
        return v;
    }

    RhoValue& peek(int dist = 0) {
        if (static_cast<int>(stack_.size()) <= dist) {
            throw VMError("Stack peek out of bounds");
        }
        return stack_[stack_.size() - 1 - static_cast<size_t>(dist)];
    }

    // -----------------------------------------------------------------------
    // Frame helpers
    // -----------------------------------------------------------------------

    CallFrame& frame() {
        return frames_.back();
    }

    Chunk& chunk() {
        return *frames_.back().chunk;
    }

    /** Fetch the current instruction and advance ip. */
    Instruction fetch() {
        CallFrame& cf = frames_.back();
        if (cf.ip >= cf.chunk->code.size()) {
            throw VMError("Instruction pointer out of bounds");
        }
        return cf.chunk->code[cf.ip++];
    }

    // -----------------------------------------------------------------------
    // isTruthy
    // -----------------------------------------------------------------------

    bool isTruthy(const RhoValue& value) const {
        return std::visit([](const auto& arg) -> bool {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, bool>)
                return arg;
            else if constexpr (std::is_same_v<T, int64_t>)
                return arg != int64_t(0);
            else if constexpr (std::is_same_v<T, int8_t>)
                return arg != int8_t(0);
            else if constexpr (std::is_same_v<T, int16_t>)
                return arg != int16_t(0);
            else if constexpr (std::is_same_v<T, int32_t>)
                return arg != int32_t(0);
            else if constexpr (std::is_same_v<T, uint8_t>)
                return arg != uint8_t(0);
            else if constexpr (std::is_same_v<T, uint16_t>)
                return arg != uint16_t(0);
            else if constexpr (std::is_same_v<T, uint32_t>)
                return arg != uint32_t(0);
            else if constexpr (std::is_same_v<T, uint64_t>)
                return arg != uint64_t(0);
            else if constexpr (std::is_same_v<T, double>)
                return arg != 0.0;
            else if constexpr (std::is_same_v<T, std::string>)
                return !arg.empty();
            else if constexpr (std::is_same_v<T, std::shared_ptr<RhoNull>>)
                return false;
            else
                return true; // vectors, matrices, collections, functions -> truthy
        }, value);
    }

    // -----------------------------------------------------------------------
    // applyBinaryOp
    // -----------------------------------------------------------------------

    RhoValue applyBinaryOp(Opcode op, const RhoValue& left, const RhoValue& right) {
        // ---- Comparison opcodes ----
        if (op == Opcode::EQ || op == Opcode::NE ||
            op == Opcode::LT || op == Opcode::GT ||
            op == Opcode::LE || op == Opcode::GE)
        {
            return applyComparison(op, left, right);
        }

        // ---- Arithmetic ----
        switch (op) {
        case Opcode::ADD: return applyAdd(left, right);
        case Opcode::SUB: return applySub(left, right);
        case Opcode::MUL: return applyMul(left, right);
        case Opcode::DIV: return applyDiv(left, right);
        case Opcode::MOD: return applyMod(left, right);
        default:
            throw VMError("Unknown binary opcode in applyBinaryOp");
        }
    }

    // ---- ADD ----
    RhoValue applyAdd(const RhoValue& left, const RhoValue& right) {
        // Fast path: most common numeric combinations avoid std::visit overhead
        if (const auto* l = std::get_if<int64_t>(&left)) {
            if (const auto* r = std::get_if<int64_t>(&right)) return *l + *r;
            if (const auto* r = std::get_if<double>(&right))  return static_cast<double>(*l) + *r;
        }
        if (const auto* l = std::get_if<double>(&left)) {
            if (const auto* r = std::get_if<double>(&right))  return *l + *r;
            if (const auto* r = std::get_if<int64_t>(&right)) return *l + static_cast<double>(*r);
        }
        // Slow path: handles strings, vectors, matrices, and other types
        return std::visit([](const auto& l, const auto& r) -> RhoValue {
            using L = std::decay_t<decltype(l)>;
            using R = std::decay_t<decltype(r)>;

            // int + int -> int
            if constexpr (std::is_same_v<L, int64_t> && std::is_same_v<R, int64_t>)
                return l + r;
            // mixed numeric -> double
            else if constexpr ((std::is_arithmetic_v<L> && !std::is_same_v<L, bool>) &&
                               (std::is_arithmetic_v<R> && !std::is_same_v<R, bool>))
                return static_cast<double>(l) + static_cast<double>(r);
            // string + anything -> string
            else if constexpr (std::is_same_v<L, std::string>)
                return l + valueToString(RhoValue{r});
            else if constexpr (std::is_same_v<R, std::string>)
                return valueToString(RhoValue{l}) + r;
            // vec + vec
            else if constexpr (std::is_same_v<L, Eigen::VectorXd> && std::is_same_v<R, Eigen::VectorXd>) {
                if (l.size() != r.size())
                    throw VMError("Vector size mismatch in '+'");
                return Eigen::VectorXd(l + r);
            }
            // mat + mat
            else if constexpr (std::is_same_v<L, Eigen::MatrixXd> && std::is_same_v<R, Eigen::MatrixXd>) {
                if (l.rows() != r.rows() || l.cols() != r.cols())
                    throw VMError("Matrix dimension mismatch in '+'");
                return Eigen::MatrixXd(l + r);
            }
            else
                throw VMError("Incompatible types for '+'");
        }, left, right);
    }

    // ---- SUB ----
    RhoValue applySub(const RhoValue& left, const RhoValue& right) {
        if (const auto* l = std::get_if<int64_t>(&left)) {
            if (const auto* r = std::get_if<int64_t>(&right)) return *l - *r;
            if (const auto* r = std::get_if<double>(&right))  return static_cast<double>(*l) - *r;
        }
        if (const auto* l = std::get_if<double>(&left)) {
            if (const auto* r = std::get_if<double>(&right))  return *l - *r;
            if (const auto* r = std::get_if<int64_t>(&right)) return *l - static_cast<double>(*r);
        }
        return std::visit([](const auto& l, const auto& r) -> RhoValue {
            using L = std::decay_t<decltype(l)>;
            using R = std::decay_t<decltype(r)>;

            if constexpr (std::is_same_v<L, int64_t> && std::is_same_v<R, int64_t>)
                return l - r;
            else if constexpr ((std::is_arithmetic_v<L> && !std::is_same_v<L, bool>) &&
                               (std::is_arithmetic_v<R> && !std::is_same_v<R, bool>))
                return static_cast<double>(l) - static_cast<double>(r);
            else if constexpr (std::is_same_v<L, Eigen::VectorXd> && std::is_same_v<R, Eigen::VectorXd>) {
                if (l.size() != r.size())
                    throw VMError("Vector size mismatch in '-'");
                return Eigen::VectorXd(l - r);
            }
            else if constexpr (std::is_same_v<L, Eigen::MatrixXd> && std::is_same_v<R, Eigen::MatrixXd>) {
                if (l.rows() != r.rows() || l.cols() != r.cols())
                    throw VMError("Matrix dimension mismatch in '-'");
                return Eigen::MatrixXd(l - r);
            }
            else
                throw VMError("Incompatible types for '-'");
        }, left, right);
    }

    // ---- MUL ----
    RhoValue applyMul(const RhoValue& left, const RhoValue& right) {
        if (const auto* l = std::get_if<int64_t>(&left)) {
            if (const auto* r = std::get_if<int64_t>(&right)) return *l * *r;
            if (const auto* r = std::get_if<double>(&right))  return static_cast<double>(*l) * *r;
        }
        if (const auto* l = std::get_if<double>(&left)) {
            if (const auto* r = std::get_if<double>(&right))  return *l * *r;
            if (const auto* r = std::get_if<int64_t>(&right)) return *l * static_cast<double>(*r);
        }
        return std::visit([](const auto& l, const auto& r) -> RhoValue {
            using L = std::decay_t<decltype(l)>;
            using R = std::decay_t<decltype(r)>;

            if constexpr (std::is_same_v<L, int64_t> && std::is_same_v<R, int64_t>)
                return l * r;
            else if constexpr ((std::is_arithmetic_v<L> && !std::is_same_v<L, bool>) &&
                               (std::is_arithmetic_v<R> && !std::is_same_v<R, bool>))
                return static_cast<double>(l) * static_cast<double>(r);
            // scalar * vec
            else if constexpr ((std::is_same_v<L, double> || std::is_same_v<L, int64_t>) &&
                               std::is_same_v<R, Eigen::VectorXd>)
                return Eigen::VectorXd(static_cast<double>(l) * r);
            else if constexpr (std::is_same_v<L, Eigen::VectorXd> &&
                               (std::is_same_v<R, double> || std::is_same_v<R, int64_t>))
                return Eigen::VectorXd(l * static_cast<double>(r));
            // scalar * mat
            else if constexpr ((std::is_same_v<L, double> || std::is_same_v<L, int64_t>) &&
                               std::is_same_v<R, Eigen::MatrixXd>)
                return Eigen::MatrixXd(static_cast<double>(l) * r);
            else if constexpr (std::is_same_v<L, Eigen::MatrixXd> &&
                               (std::is_same_v<R, double> || std::is_same_v<R, int64_t>))
                return Eigen::MatrixXd(l * static_cast<double>(r));
            // mat * mat
            else if constexpr (std::is_same_v<L, Eigen::MatrixXd> && std::is_same_v<R, Eigen::MatrixXd>) {
                if (l.cols() != r.rows())
                    throw VMError("Matrix dimension mismatch in '*'");
                return Eigen::MatrixXd(l * r);
            }
            // mat * vec
            else if constexpr (std::is_same_v<L, Eigen::MatrixXd> && std::is_same_v<R, Eigen::VectorXd>) {
                if (l.cols() != r.size())
                    throw VMError("Matrix/vector dimension mismatch in '*'");
                return Eigen::VectorXd(l * r);
            }
            else
                throw VMError("Incompatible types for '*'");
        }, left, right);
    }

    // ---- DIV ----
    RhoValue applyDiv(const RhoValue& left, const RhoValue& right) {
        if (const auto* l = std::get_if<int64_t>(&left)) {
            if (const auto* r = std::get_if<int64_t>(&right)) {
                if (*r == 0) throw VMError("Division by zero");
                return static_cast<double>(*l) / static_cast<double>(*r);
            }
            if (const auto* r = std::get_if<double>(&right)) {
                if (*r == 0.0) throw VMError("Division by zero");
                return static_cast<double>(*l) / *r;
            }
        }
        if (const auto* l = std::get_if<double>(&left)) {
            if (const auto* r = std::get_if<double>(&right)) {
                if (*r == 0.0) throw VMError("Division by zero");
                return *l / *r;
            }
            if (const auto* r = std::get_if<int64_t>(&right)) {
                if (*r == 0) throw VMError("Division by zero");
                return *l / static_cast<double>(*r);
            }
        }
        return std::visit([](const auto& l, const auto& r) -> RhoValue {
            using L = std::decay_t<decltype(l)>;
            using R = std::decay_t<decltype(r)>;

            if constexpr ((std::is_arithmetic_v<L> && !std::is_same_v<L, bool>) &&
                          (std::is_arithmetic_v<R> && !std::is_same_v<R, bool>)) {
                double divisor = static_cast<double>(r);
                if (divisor == 0.0) throw VMError("Division by zero");
                return static_cast<double>(l) / divisor;
            }
            else if constexpr (std::is_same_v<L, Eigen::VectorXd> &&
                               (std::is_same_v<R, double> || std::is_same_v<R, int64_t>)) {
                double divisor = static_cast<double>(r);
                if (divisor == 0.0) throw VMError("Division by zero");
                return Eigen::VectorXd(l / divisor);
            }
            else if constexpr (std::is_same_v<L, Eigen::MatrixXd> &&
                               (std::is_same_v<R, double> || std::is_same_v<R, int64_t>)) {
                double divisor = static_cast<double>(r);
                if (divisor == 0.0) throw VMError("Division by zero");
                return Eigen::MatrixXd(l / divisor);
            }
            else
                throw VMError("Incompatible types for '/'");
        }, left, right);
    }

    // ---- MOD ----
    RhoValue applyMod(const RhoValue& left, const RhoValue& right) {
        if (!isScalar(left) || !isScalar(right))
            throw VMError("Modulo requires scalar operands");
        int64_t r = toInt(right);
        if (r == 0) throw VMError("Modulo by zero");
        return toInt(left) % r;
    }

    // ---- Comparison ----
    RhoValue applyComparison(Opcode op, const RhoValue& left, const RhoValue& right) {
        // null == null
        if (std::holds_alternative<std::shared_ptr<RhoNull>>(left) &&
            std::holds_alternative<std::shared_ptr<RhoNull>>(right)) {
            switch (op) {
            case Opcode::EQ: return true;
            case Opcode::NE: return false;
            default: throw VMError("Ordered comparison not supported for null");
            }
        }

        // bool == bool
        if (std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right)) {
            bool l = std::get<bool>(left);
            bool r = std::get<bool>(right);
            switch (op) {
            case Opcode::EQ: return l == r;
            case Opcode::NE: return l != r;
            default: throw VMError("Ordered comparison not supported for bool");
            }
        }

        // string == string
        if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) {
            const auto& l = std::get<std::string>(left);
            const auto& r = std::get<std::string>(right);
            switch (op) {
            case Opcode::EQ: return l == r;
            case Opcode::NE: return l != r;
            case Opcode::LT: return l < r;
            case Opcode::GT: return l > r;
            case Opcode::LE: return l <= r;
            case Opcode::GE: return l >= r;
            default: break;
            }
        }

        // numeric fast path: int vs int (avoids toDouble + std::visit)
        if (const auto* l = std::get_if<int64_t>(&left)) {
            if (const auto* r = std::get_if<int64_t>(&right)) {
                switch (op) {
                case Opcode::EQ: return *l == *r;
                case Opcode::NE: return *l != *r;
                case Opcode::LT: return *l <  *r;
                case Opcode::GT: return *l >  *r;
                case Opcode::LE: return *l <= *r;
                case Opcode::GE: return *l >= *r;
                default: break;
                }
            }
        }
        // numeric fast path: double vs double
        if (const auto* l = std::get_if<double>(&left)) {
            if (const auto* r = std::get_if<double>(&right)) {
                switch (op) {
                case Opcode::EQ: return *l == *r;
                case Opcode::NE: return *l != *r;
                case Opcode::LT: return *l <  *r;
                case Opcode::GT: return *l >  *r;
                case Opcode::LE: return *l <= *r;
                case Opcode::GE: return *l >= *r;
                default: break;
                }
            }
        }

        // numeric slow path: mixed or other scalar types
        if (!isScalar(left) || !isScalar(right))
            throw VMError("Comparison requires scalar operands");

        double l = toDouble(left);
        double r = toDouble(right);
        switch (op) {
        case Opcode::EQ: return l == r;
        case Opcode::NE: return l != r;
        case Opcode::LT: return l < r;
        case Opcode::GT: return l > r;
        case Opcode::LE: return l <= r;
        case Opcode::GE: return l >= r;
        default: throw VMError("Unknown comparison opcode");
        }
    }

    // -----------------------------------------------------------------------
    // COERCE helper
    // -----------------------------------------------------------------------

    RhoValue coerceValue(const RhoValue& val, RhoType target) {
        RhoType src = getValueType(val);
        if (src == target) return val;

        switch (target) {
        case RhoType::Int:     return static_cast<int64_t>(toInt(val));
        case RhoType::Int8:    return static_cast<int8_t>(toInt(val));
        case RhoType::Int16:   return static_cast<int16_t>(toInt(val));
        case RhoType::Int32:   return static_cast<int32_t>(toInt(val));
        case RhoType::UInt8:
        case RhoType::Byte:    return static_cast<uint8_t>(toInt(val));
        case RhoType::UInt16:  return static_cast<uint16_t>(toInt(val));
        case RhoType::UInt32:  return static_cast<uint32_t>(toInt(val));
        case RhoType::UInt64:  return static_cast<uint64_t>(toInt(val));
        case RhoType::Float64: return toDouble(val);
        case RhoType::Bool:    return isTruthy(val);
        case RhoType::String:  return valueToString(val);
        default:
            throw VMError("Cannot coerce value to type '" + typeToString(target) + "'");
        }
    }

    // -----------------------------------------------------------------------
    // callFunction — set up a new CallFrame for a compiled RhoFunction
    // -----------------------------------------------------------------------

    /**
     * @brief Set up a new call frame.
     *
     * Before this is called the stack looks like:
     *   ... [callee] [arg0] [arg1] ... [argN-1]
     * where argc == N.
     *
     * After this call the stack looks like:
     *   ... [arg0] [arg1] ... [argN-1] [local_N] ... [local_numLocals-1]
     * and a new CallFrame has been pushed.  The return value will be pushed
     * on top of the stack by RETURN/RETURN_NULL after popping the frame.
     */
    void callFunction(const RhoValue& callee, int argc) {
        if (!std::holds_alternative<std::shared_ptr<RhoFunction>>(callee))
            throw VMError("Attempt to call a non-function value");

        auto func = std::get<std::shared_ptr<RhoFunction>>(callee);
        if (!func)
            throw VMError("Null function value");

        // ---- Native function ----
        if (func->isNative()) {
            std::vector<RhoValue> args;
            args.reserve(static_cast<size_t>(argc));
            // args are at stack[top-argc .. top-1]
            size_t base = stack_.size() - static_cast<size_t>(argc);
            for (int i = 0; i < argc; ++i)
                args.push_back(stack_[base + static_cast<size_t>(i)]);
            // Remove callee + args from stack
            // callee is just below the args
            stack_.resize(base - 1); // removes callee too
            RhoValue result = func->callNative(args);
            push(std::move(result));
            return;
        }

        // ---- Compiled function ----
        if (isCompiledFunction(*func)) {
            auto chunk = compiledChunk(*func);
            if (!chunk)
                throw VMError("Compiled function has null chunk");

            int expected = chunk->arity;
            if (argc != expected)
                throw VMError("Arity mismatch: expected " + std::to_string(expected) +
                              " args, got " + std::to_string(argc));

            // Remove the callee from the stack; args remain at the top.
            // callee is at stack_[stack_.size() - argc - 1]
            size_t calleeIdx = stack_.size() - static_cast<size_t>(argc) - 1;
            stack_.erase(stack_.begin() + static_cast<ptrdiff_t>(calleeIdx));

            // bp is now the index of arg0
            size_t bp = stack_.size() - static_cast<size_t>(argc);

            // Extend stack to hold all locals (numLocals >= argc)
            int numLocals = chunk->numLocals;
            if (numLocals < argc)
                throw VMError("numLocals < argc — malformed chunk");
            size_t extraLocals = static_cast<size_t>(numLocals) - static_cast<size_t>(argc);
            for (size_t i = 0; i < extraLocals; ++i)
                stack_.push_back(RhoValue{std::make_shared<RhoNull>()});

            frames_.push_back(CallFrame{chunk, 0, bp, func});
            return;
        }

        // ---- AST-based lambda (non-native, non-compiled) ----
        // These are constructed by the Evaluator, not the Compiler.
        // The VM can still call them by forwarding to the Builtins machinery
        // would require the Evaluator, which we cannot run here.
        // We fall back to a best-effort native call if the function happens
        // to expose a native wrapper; otherwise throw.
        throw VMError("VM cannot call AST-based lambda functions directly. "
                      "Only compiled (bytecode) or native functions are supported.");
    }

    // -----------------------------------------------------------------------
    // callBuiltinByName
    // -----------------------------------------------------------------------

    RhoValue callBuiltinByName(const std::string& name, std::vector<RhoValue>& args) {
        return Builtins::instance().call(name, args);
    }

    // -----------------------------------------------------------------------
    // throwException — handle THROW with try-frame unwinding
    // -----------------------------------------------------------------------

    /**
     * @brief Handle a user-thrown exception.
     *
     * If there is an active try frame, unwind to its catch handler and push
     * the exception value.  Otherwise re-throw as a C++ exception.
     *
     * @return true if the exception was caught (ip updated, value on stack).
     */
    bool handleException(RhoValue exVal) {
        if (tryStack_.empty()) {
            // No handler — propagate as C++ exception.
            throw UserException(exVal, {});
        }

        TryFrame tf = tryStack_.back();
        tryStack_.pop_back();

        // Unwind call frames
        while (frames_.size() > tf.frameDepth) {
            frames_.pop_back();
        }
        // Restore stack depth
        if (stack_.size() > tf.stackDepth) {
            stack_.resize(tf.stackDepth);
        }

        // Jump to catch handler
        frames_.back().ip = static_cast<size_t>(tf.catchTarget);

        // Push exception value for CATCH_BIND to consume
        push(std::move(exVal));
        return true;
    }

    // -----------------------------------------------------------------------
    // splitDot — split "module.func" into {"module","func"}
    // -----------------------------------------------------------------------

    static std::pair<std::string, std::string> splitDot(const std::string& s) {
        auto pos = s.find('.');
        if (pos == std::string::npos)
            return {s, ""};
        return {s.substr(0, pos), s.substr(pos + 1)};
    }

    // -----------------------------------------------------------------------
    // INDEX_GET helpers
    // -----------------------------------------------------------------------

    RhoValue doIndexGet(const RhoValue& container, const RhoValue& idx) {
        return std::visit([&](const auto& c) -> RhoValue {
            using C = std::decay_t<decltype(c)>;

            if constexpr (std::is_same_v<C, Eigen::VectorXd>) {
                size_t i = static_cast<size_t>(toInt(idx));
                if (i >= static_cast<size_t>(c.size()))
                    throw VMError("Vector index " + std::to_string(i) + " out of range");
                return c(static_cast<Eigen::Index>(i));
            }
            else if constexpr (std::is_same_v<C, Eigen::MatrixXd>) {
                // 1-D flat index into matrix (row-major)
                size_t i = static_cast<size_t>(toInt(idx));
                Eigen::Index total = c.rows() * c.cols();
                if (static_cast<Eigen::Index>(i) >= total)
                    throw VMError("Matrix flat index out of range");
                return c(static_cast<Eigen::Index>(i) / c.cols(),
                         static_cast<Eigen::Index>(i) % c.cols());
            }
            else if constexpr (std::is_same_v<C, std::shared_ptr<RhoArray>>) {
                if (!c) throw VMError("Null array");
                if (std::holds_alternative<std::string>(idx)) {
                    throw VMError("Array index must be integer, not string");
                }
                size_t i = static_cast<size_t>(toInt(idx));
                return c->get(i);
            }
            else if constexpr (std::is_same_v<C, std::shared_ptr<RhoMap>>) {
                if (!c) throw VMError("Null map");
                if (!std::holds_alternative<std::string>(idx))
                    throw VMError("Map key must be a string");
                return c->get(std::get<std::string>(idx));
            }
            else if constexpr (std::is_same_v<C, std::shared_ptr<RhoTuple>>) {
                if (!c) throw VMError("Null tuple");
                size_t i = static_cast<size_t>(toInt(idx));
                return c->get(i);
            }
            else if constexpr (std::is_same_v<C, std::shared_ptr<RhoRecord>>) {
                if (!c) throw VMError("Null record");
                if (!std::holds_alternative<std::string>(idx))
                    throw VMError("Record field name must be a string");
                return c->getField(std::get<std::string>(idx));
            }
            else {
                throw VMError("Type '" + typeToString(getValueType(RhoValue{c})) +
                              "' does not support indexing");
            }
        }, container);
    }

    RhoValue doIndexGet2D(const RhoValue& container, const RhoValue& idx1, const RhoValue& idx2) {
        return std::visit([&](const auto& c) -> RhoValue {
            using C = std::decay_t<decltype(c)>;

            if constexpr (std::is_same_v<C, Eigen::MatrixXd>) {
                size_t row = static_cast<size_t>(toInt(idx1));
                size_t col = static_cast<size_t>(toInt(idx2));
                if (row >= static_cast<size_t>(c.rows()) ||
                    col >= static_cast<size_t>(c.cols()))
                    throw VMError("Matrix index out of range");
                return c(static_cast<Eigen::Index>(row), static_cast<Eigen::Index>(col));
            }
            else if constexpr (std::is_same_v<C, std::shared_ptr<RhoArray>>) {
                if (!c) throw VMError("Null array");
                size_t i = static_cast<size_t>(toInt(idx1));
                RhoValue inner = c->get(i);
                return doIndexGet(inner, idx2);
            }
            else {
                throw VMError("2D indexing only supported for matrices and nested arrays");
            }
        }, container);
    }

    // -----------------------------------------------------------------------
    // INDEX_SET helpers
    // -----------------------------------------------------------------------

    /**
     * @brief Perform indexed assignment.
     *
     * For heap types (RhoArray, RhoMap, RhoRecord) the mutation is in-place.
     * For value types (VectorXd, MatrixXd) a modified copy is returned and
     * the caller is responsible for storing it back.
     *
     * @return The (possibly modified) container value.
     */
    RhoValue doIndexSet(RhoValue container, const RhoValue& idx, const RhoValue& val) {
        return std::visit([&](auto& c) -> RhoValue {
            using C = std::decay_t<decltype(c)>;

            if constexpr (std::is_same_v<C, Eigen::VectorXd>) {
                size_t i = static_cast<size_t>(toInt(idx));
                if (i >= static_cast<size_t>(c.size()))
                    throw VMError("Vector index out of range in assignment");
                c(static_cast<Eigen::Index>(i)) = toDouble(val);
                return RhoValue{c};
            }
            else if constexpr (std::is_same_v<C, Eigen::MatrixXd>) {
                size_t flat = static_cast<size_t>(toInt(idx));
                Eigen::Index total = c.rows() * c.cols();
                if (static_cast<Eigen::Index>(flat) >= total)
                    throw VMError("Matrix flat index out of range in assignment");
                c(static_cast<Eigen::Index>(flat) / c.cols(),
                  static_cast<Eigen::Index>(flat) % c.cols()) = toDouble(val);
                return RhoValue{c};
            }
            else if constexpr (std::is_same_v<C, std::shared_ptr<RhoArray>>) {
                if (!c) throw VMError("Null array in indexed assignment");
                size_t i = static_cast<size_t>(toInt(idx));
                c->set(i, val);
                return RhoValue{c};
            }
            else if constexpr (std::is_same_v<C, std::shared_ptr<RhoMap>>) {
                if (!c) throw VMError("Null map in indexed assignment");
                if (!std::holds_alternative<std::string>(idx))
                    throw VMError("Map key must be a string");
                c->set(std::get<std::string>(idx), val);
                return RhoValue{c};
            }
            else if constexpr (std::is_same_v<C, std::shared_ptr<RhoRecord>>) {
                if (!c) throw VMError("Null record in indexed assignment");
                if (!std::holds_alternative<std::string>(idx))
                    throw VMError("Record field key must be a string");
                c->setField(std::get<std::string>(idx), val);
                return RhoValue{c};
            }
            else {
                throw VMError("Type '" + typeToString(getValueType(RhoValue{c})) +
                              "' does not support indexed assignment");
            }
        }, container);
    }

    RhoValue doIndexSet2D(RhoValue container, const RhoValue& idx1, const RhoValue& idx2,
                          const RhoValue& val) {
        return std::visit([&](auto& c) -> RhoValue {
            using C = std::decay_t<decltype(c)>;

            if constexpr (std::is_same_v<C, Eigen::MatrixXd>) {
                size_t row = static_cast<size_t>(toInt(idx1));
                size_t col = static_cast<size_t>(toInt(idx2));
                if (row >= static_cast<size_t>(c.rows()) ||
                    col >= static_cast<size_t>(c.cols()))
                    throw VMError("Matrix 2D index out of range in assignment");
                c(static_cast<Eigen::Index>(row), static_cast<Eigen::Index>(col)) = toDouble(val);
                return RhoValue{c};
            }
            else {
                throw VMError("2D indexed assignment only supported for matrices");
            }
        }, container);
    }

    // -----------------------------------------------------------------------
    // SLICE_GET helper
    // -----------------------------------------------------------------------

    RhoValue doSliceGet(const RhoValue& container, bool hasStart, bool hasEnd,
                        int64_t startVal, int64_t endVal) {
        return std::visit([&](const auto& c) -> RhoValue {
            using C = std::decay_t<decltype(c)>;

            if constexpr (std::is_same_v<C, Eigen::VectorXd>) {
                size_t sz = static_cast<size_t>(c.size());
                auto normIdx = [sz](int64_t idx) -> size_t {
                    if (idx < 0) {
                        int64_t n = static_cast<int64_t>(sz) + idx;
                        return n < 0 ? 0 : static_cast<size_t>(n);
                    }
                    return static_cast<size_t>(idx);
                };
                size_t s = hasStart ? normIdx(startVal) : 0;
                size_t e = hasEnd   ? normIdx(endVal)   : sz;
                if (s > sz) s = sz;
                if (e > sz) e = sz;
                if (s >= e) return Eigen::VectorXd(0);
                return Eigen::VectorXd(c.segment(static_cast<Eigen::Index>(s),
                                                  static_cast<Eigen::Index>(e - s)));
            }
            else if constexpr (std::is_same_v<C, Eigen::MatrixXd>) {
                // Row slice: return a sub-matrix block (all columns)
                size_t nr = static_cast<size_t>(c.rows());
                auto normIdx = [nr](int64_t idx) -> size_t {
                    if (idx < 0) {
                        int64_t n = static_cast<int64_t>(nr) + idx;
                        return n < 0 ? 0 : static_cast<size_t>(n);
                    }
                    return static_cast<size_t>(idx);
                };
                size_t s = hasStart ? normIdx(startVal) : 0;
                size_t e = hasEnd   ? normIdx(endVal)   : nr;
                if (s > nr) s = nr;
                if (e > nr) e = nr;
                if (s >= e) return Eigen::MatrixXd(0, c.cols());
                return Eigen::MatrixXd(c.block(static_cast<Eigen::Index>(s), 0,
                                               static_cast<Eigen::Index>(e - s), c.cols()));
            }
            else if constexpr (std::is_same_v<C, std::shared_ptr<RhoArray>>) {
                if (!c) throw VMError("Null array in slice");
                size_t sz = c->size();
                auto normIdx = [sz](int64_t idx) -> size_t {
                    if (idx < 0) {
                        int64_t n = static_cast<int64_t>(sz) + idx;
                        return n < 0 ? 0 : static_cast<size_t>(n);
                    }
                    return static_cast<size_t>(idx);
                };
                size_t s = hasStart ? normIdx(startVal) : 0;
                size_t e = hasEnd   ? normIdx(endVal)   : sz;
                if (s > sz) s = sz;
                if (e > sz) e = sz;
                auto result = std::make_shared<RhoArray>();
                for (size_t i = s; i < e; ++i)
                    result->push(c->get(i));
                return RhoValue{result};
            }
            else {
                throw VMError("Slicing not supported for type '" +
                              typeToString(getValueType(RhoValue{c})) + "'");
            }
        }, container);
    }

    // -----------------------------------------------------------------------
    // MAKE_CLOSURE
    // -----------------------------------------------------------------------

    void doMakeClosure(int32_t constIdx) {
        // The constant at constIdx must be a RhoFunction with a compiledChunk.
        const RhoValue& cv = chunk().constants[static_cast<size_t>(constIdx)];
        if (!std::holds_alternative<std::shared_ptr<RhoFunction>>(cv))
            throw VMError("MAKE_CLOSURE: constant is not a RhoFunction");

        auto tmpl = std::get<std::shared_ptr<RhoFunction>>(cv);
        if (!tmpl || !isCompiledFunction(*tmpl))
            throw VMError("MAKE_CLOSURE: constant is not a compiled function");

        auto chunkPtr = compiledChunk(*tmpl);
        if (!chunkPtr)
            throw VMError("MAKE_CLOSURE: template function has no chunk");

        // Capture upvalues as described in the chunk's upvalueDescs.
        std::vector<RhoValue> upvals;
        upvals.reserve(chunkPtr->upvalueDescs.size());

        for (const auto& desc : chunkPtr->upvalueDescs) {
            if (desc.isLocal) {
                // Capture from the enclosing function's local slot.
                size_t slot = frame().bp + static_cast<size_t>(desc.index);
                if (slot >= stack_.size())
                    throw VMError("MAKE_CLOSURE: upvalue local slot out of range");
                upvals.push_back(stack_[slot]);
            } else {
                // Capture from the enclosing closure's upvalue vector.
                if (!frame().closure)
                    throw VMError("MAKE_CLOSURE: no enclosing closure for upvalue capture");
                auto box = upvalueBoxOf(*frame().closure);
                if (!box)
                    throw VMError("MAKE_CLOSURE: enclosing closure has no upvalue box");
                size_t idx = static_cast<size_t>(desc.index);
                if (idx >= box->values.size())
                    throw VMError("MAKE_CLOSURE: upvalue index out of range in enclosing closure");
                upvals.push_back(box->values[idx]);
            }
        }

        auto newFunc = makeCompiledFunction(chunkPtr, std::move(upvals));
        push(RhoValue{newFunc});
    }

    // -----------------------------------------------------------------------
    // Main dispatch loop
    // -----------------------------------------------------------------------

    RhoValue execute() {
        while (true) {
            Instruction instr = fetch();
            Opcode op = instr.op;
            int32_t operand = instr.operand;

            try {
                switch (op) {

                // ----------------------------------------------------------
                // Constants
                // ----------------------------------------------------------
                case Opcode::LOAD_CONST: {
                    size_t idx = static_cast<size_t>(operand);
                    if (idx >= chunk().constants.size())
                        throw VMError("LOAD_CONST: index out of range");
                    push(chunk().constants[idx]);
                    break;
                }
                case Opcode::LOAD_NULL:
                    push(RhoValue{std::make_shared<RhoNull>()});
                    break;
                case Opcode::LOAD_TRUE:
                    push(RhoValue{true});
                    break;
                case Opcode::LOAD_FALSE:
                    push(RhoValue{false});
                    break;

                // ----------------------------------------------------------
                // Locals
                // ----------------------------------------------------------
                case Opcode::LOAD_LOCAL: {
                    size_t slot = frame().bp + static_cast<size_t>(operand);
                    if (slot >= stack_.size())
                        throw VMError("LOAD_LOCAL: slot out of range");
                    push(stack_[slot]);
                    break;
                }
                case Opcode::STORE_LOCAL: {
                    size_t slot = frame().bp + static_cast<size_t>(operand);
                    if (slot >= stack_.size())
                        throw VMError("STORE_LOCAL: slot out of range");
                    stack_[slot] = pop();
                    break;
                }

                // ----------------------------------------------------------
                // Upvalues
                // ----------------------------------------------------------
                case Opcode::LOAD_UPVALUE: {
                    if (!frame().closure)
                        throw VMError("LOAD_UPVALUE: no active closure");
                    auto box = upvalueBoxOf(*frame().closure);
                    if (!box)
                        throw VMError("LOAD_UPVALUE: closure has no upvalue box");
                    size_t idx = static_cast<size_t>(operand);
                    if (idx >= box->values.size())
                        throw VMError("LOAD_UPVALUE: index out of range");
                    push(box->values[idx]);
                    break;
                }
                case Opcode::STORE_UPVALUE: {
                    if (!frame().closure)
                        throw VMError("STORE_UPVALUE: no active closure");
                    auto box = upvalueBoxOf(*frame().closure);
                    if (!box)
                        throw VMError("STORE_UPVALUE: closure has no upvalue box");
                    size_t idx = static_cast<size_t>(operand);
                    if (idx >= box->values.size())
                        throw VMError("STORE_UPVALUE: index out of range");
                    box->values[idx] = pop();
                    break;
                }

                // ----------------------------------------------------------
                // Globals
                // ----------------------------------------------------------
                case Opcode::LOAD_GLOBAL: {
                    size_t nameIdx = static_cast<size_t>(operand);
                    if (nameIdx >= chunk().names.size())
                        throw VMError("LOAD_GLOBAL: name index out of range");
                    const std::string& name = chunk().names[nameIdx];
                    auto it = globals_.find(name);
                    if (it == globals_.end())
                        throw VMError("Undefined global variable '" + name + "'");
                    push(it->second);
                    break;
                }
                case Opcode::STORE_GLOBAL: {
                    size_t nameIdx = static_cast<size_t>(operand);
                    if (nameIdx >= chunk().names.size())
                        throw VMError("STORE_GLOBAL: name index out of range");
                    globals_[chunk().names[nameIdx]] = pop();
                    break;
                }

                // ----------------------------------------------------------
                // Stack manipulation
                // ----------------------------------------------------------
                case Opcode::POP:
                    pop();
                    break;
                case Opcode::DUP:
                    push(peek(0));
                    break;

                // ----------------------------------------------------------
                // Arithmetic
                // ----------------------------------------------------------
                case Opcode::ADD:
                case Opcode::SUB:
                case Opcode::MUL:
                case Opcode::DIV:
                case Opcode::MOD: {
                    RhoValue right = pop();
                    RhoValue left  = pop();
                    push(applyBinaryOp(op, left, right));
                    break;
                }
                case Opcode::NEGATE: {
                    RhoValue v = pop();
                    RhoValue result = std::visit([](const auto& arg) -> RhoValue {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, int64_t>)   return -arg;
                        else if constexpr (std::is_same_v<T, int8_t>)  return static_cast<int8_t>(-arg);
                        else if constexpr (std::is_same_v<T, int16_t>) return static_cast<int16_t>(-arg);
                        else if constexpr (std::is_same_v<T, int32_t>) return static_cast<int32_t>(-arg);
                        else if constexpr (std::is_same_v<T, double>)  return -arg;
                        else if constexpr (std::is_same_v<T, Eigen::VectorXd>) return Eigen::VectorXd(-arg);
                        else if constexpr (std::is_same_v<T, Eigen::MatrixXd>) return Eigen::MatrixXd(-arg);
                        else throw VMError("Cannot negate non-numeric value");
                    }, v);
                    push(std::move(result));
                    break;
                }

                // ----------------------------------------------------------
                // Comparison
                // ----------------------------------------------------------
                case Opcode::EQ:
                case Opcode::NE:
                case Opcode::LT:
                case Opcode::GT:
                case Opcode::LE:
                case Opcode::GE: {
                    RhoValue right = pop();
                    RhoValue left  = pop();
                    push(applyComparison(op, left, right));
                    break;
                }

                // ----------------------------------------------------------
                // Logical NOT
                // ----------------------------------------------------------
                case Opcode::NOT: {
                    RhoValue v = pop();
                    push(RhoValue{!isTruthy(v)});
                    break;
                }

                // ----------------------------------------------------------
                // Jumps
                // ----------------------------------------------------------
                case Opcode::JUMP:
                    frame().ip = static_cast<size_t>(operand);
                    break;

                case Opcode::JUMP_IF_FALSE: {
                    RhoValue v = pop();
                    if (!isTruthy(v))
                        frame().ip = static_cast<size_t>(operand);
                    break;
                }
                case Opcode::JUMP_IF_TRUE: {
                    RhoValue v = pop();
                    if (isTruthy(v))
                        frame().ip = static_cast<size_t>(operand);
                    break;
                }
                case Opcode::JUMP_IF_FALSE_PEEK:
                    if (!isTruthy(peek(0)))
                        frame().ip = static_cast<size_t>(operand);
                    break;
                case Opcode::JUMP_IF_TRUE_PEEK:
                    if (isTruthy(peek(0)))
                        frame().ip = static_cast<size_t>(operand);
                    break;

                // ----------------------------------------------------------
                // CALL
                // ----------------------------------------------------------
                case Opcode::CALL: {
                    int argc = operand;
                    RhoValue callee = stack_[stack_.size() - static_cast<size_t>(argc) - 1];
                    callFunction(callee, argc);
                    // If callFunction pushed a result (native) we continue;
                    // for compiled functions the new frame will run on next iteration.
                    break;
                }

                // ----------------------------------------------------------
                // CALL_BUILTIN
                // ----------------------------------------------------------
                case Opcode::CALL_BUILTIN: {
                    // Encoding: (name_idx << 8) | argc
                    int argc    = operand & 0xFF;
                    int nameIdx = (operand >> 8) & 0xFFFFFF;
                    if (nameIdx >= static_cast<int>(chunk().names.size()))
                        throw VMError("CALL_BUILTIN: name index out of range");
                    const std::string& name = chunk().names[static_cast<size_t>(nameIdx)];

                    std::vector<RhoValue> args;
                    args.resize(static_cast<size_t>(argc));
                    for (int i = argc - 1; i >= 0; --i) {
                        args[static_cast<size_t>(i)] = pop();
                    }
                    RhoValue result = Builtins::instance().call(name, args);
                    push(std::move(result));
                    break;
                }

                // ----------------------------------------------------------
                // CALL_MODULE
                // ----------------------------------------------------------
                case Opcode::CALL_MODULE: {
                    // operand = name_idx.  The next instruction encodes argc.
                    int nameIdx = operand;
                    if (nameIdx >= static_cast<int>(chunk().names.size()))
                        throw VMError("CALL_MODULE: name index out of range");
                    const std::string& fullName = chunk().names[static_cast<size_t>(nameIdx)];

                    // Read the immediately following instruction for argc.
                    Instruction argcInstr = fetch();
                    int argc = argcInstr.operand;

                    auto [modName, funcName] = splitDot(fullName);
                    if (funcName.empty())
                        throw VMError("CALL_MODULE: name '" + fullName + "' has no '.'");

                    std::vector<RhoValue> args;
                    args.resize(static_cast<size_t>(argc));
                    for (int i = argc - 1; i >= 0; --i) {
                        args[static_cast<size_t>(i)] = pop();
                    }
                    RhoValue result = Builtins::instance().callModule(modName, funcName, args);
                    push(std::move(result));
                    break;
                }

                // ----------------------------------------------------------
                // LOAD_MODULE_CONST
                // ----------------------------------------------------------
                case Opcode::LOAD_MODULE_CONST: {
                    int nameIdx = operand;
                    if (nameIdx >= static_cast<int>(chunk().names.size()))
                        throw VMError("LOAD_MODULE_CONST: name index out of range");
                    const std::string& fullName = chunk().names[static_cast<size_t>(nameIdx)];
                    auto [modName, member] = splitDot(fullName);
                    if (member.empty())
                        throw VMError("LOAD_MODULE_CONST: name has no '.'");
                    RhoValue val = Builtins::instance().getModuleConstant(modName, member);
                    push(std::move(val));
                    break;
                }

                // ----------------------------------------------------------
                // RETURN
                // ----------------------------------------------------------
                case Opcode::RETURN: {
                    RhoValue retVal = pop();
                    size_t prevBp = frame().bp;
                    frames_.pop_back();

                    if (frames_.empty()) {
                        // Top-level return.
                        return retVal;
                    }
                    // callFunction already removed the callee from the stack and
                    // set bp == index of arg0.  Everything at [bp..] belongs to
                    // the now-finished call (args + locals + temporaries).
                    // Truncate back to bp so the caller's stack is clean, then
                    // push the return value.
                    stack_.resize(prevBp);
                    push(std::move(retVal));
                    break;
                }

                case Opcode::RETURN_NULL: {
                    size_t prevBp = frame().bp;
                    frames_.pop_back();

                    if (frames_.empty()) {
                        return RhoValue{std::make_shared<RhoNull>()};
                    }
                    stack_.resize(prevBp);
                    push(RhoValue{std::make_shared<RhoNull>()});
                    break;
                }

                // ----------------------------------------------------------
                // MAKE_CLOSURE
                // ----------------------------------------------------------
                case Opcode::MAKE_CLOSURE:
                    doMakeClosure(operand);
                    break;

                // ----------------------------------------------------------
                // Collection builders
                // ----------------------------------------------------------
                case Opcode::BUILD_VEC: {
                    int count = operand;
                    Eigen::VectorXd vec(count);
                    for (int i = count - 1; i >= 0; --i) {
                        vec(i) = toDouble(pop());
                    }
                    push(RhoValue{vec});
                    break;
                }
                case Opcode::BUILD_MAT: {
                    int rows = (operand >> 16) & 0xFFFF;
                    int cols =  operand        & 0xFFFF;
                    Eigen::MatrixXd mat(rows, cols);
                    // Stack has elements in row-major order: first pushed = (0,0).
                    // We pop in reverse so last element (rows-1, cols-1) comes first.
                    for (int r = rows - 1; r >= 0; --r) {
                        for (int c = cols - 1; c >= 0; --c) {
                            mat(r, c) = toDouble(pop());
                        }
                    }
                    push(RhoValue{mat});
                    break;
                }
                case Opcode::BUILD_SET: {
                    int count = operand;
                    auto set = std::make_shared<RhoSet>();
                    std::vector<RhoValue> elems(static_cast<size_t>(count));
                    for (int i = count - 1; i >= 0; --i)
                        elems[static_cast<size_t>(i)] = pop();
                    for (auto& e : elems)
                        set->add(e);
                    push(RhoValue{set});
                    break;
                }
                case Opcode::BUILD_TUPLE: {
                    int count = operand;
                    std::vector<RhoValue> elems(static_cast<size_t>(count));
                    for (int i = count - 1; i >= 0; --i)
                        elems[static_cast<size_t>(i)] = pop();
                    push(RhoValue{std::make_shared<RhoTuple>(std::move(elems))});
                    break;
                }
                case Opcode::BUILD_RECORD: {
                    // operand = number of key/value pairs
                    int count = operand;
                    auto rec = std::make_shared<RhoRecord>();
                    // Stack layout (bottom to top): key0, val0, key1, val1, ...
                    // Pop in reverse: last pair first.
                    struct KV { std::string key; RhoValue val; };
                    std::vector<KV> pairs(static_cast<size_t>(count));
                    for (int i = count - 1; i >= 0; --i) {
                        pairs[static_cast<size_t>(i)].val = pop();
                        RhoValue keyVal = pop();
                        if (!std::holds_alternative<std::string>(keyVal))
                            throw VMError("BUILD_RECORD: key must be a string");
                        pairs[static_cast<size_t>(i)].key = std::get<std::string>(keyVal);
                    }
                    for (auto& kv : pairs)
                        rec->setField(kv.key, kv.val);
                    push(RhoValue{rec});
                    break;
                }
                case Opcode::BUILD_ARR: {
                    int count = operand;
                    auto arr = std::make_shared<RhoArray>();
                    std::vector<RhoValue> elems(static_cast<size_t>(count));
                    for (int i = count - 1; i >= 0; --i)
                        elems[static_cast<size_t>(i)] = pop();
                    for (auto& e : elems)
                        arr->push(e);
                    push(RhoValue{arr});
                    break;
                }
                case Opcode::BUILD_MAP: {
                    // The count of pairs was pushed as an integer constant before BUILD_MAP.
                    // Pop it from the stack.
                    RhoValue countVal = pop();
                    int count = static_cast<int>(toInt(countVal));
                    auto map = std::make_shared<RhoMap>();
                    struct KV { std::string key; RhoValue val; };
                    std::vector<KV> pairs(static_cast<size_t>(count));
                    for (int i = count - 1; i >= 0; --i) {
                        pairs[static_cast<size_t>(i)].val = pop();
                        RhoValue keyVal = pop();
                        if (!std::holds_alternative<std::string>(keyVal))
                            throw VMError("BUILD_MAP: key must be a string");
                        pairs[static_cast<size_t>(i)].key = std::get<std::string>(keyVal);
                    }
                    for (auto& kv : pairs)
                        map->set(kv.key, kv.val);
                    push(RhoValue{map});
                    break;
                }

                // ----------------------------------------------------------
                // Access
                // ----------------------------------------------------------
                case Opcode::INDEX_GET: {
                    RhoValue idx       = pop();
                    RhoValue container = pop();
                    push(doIndexGet(container, idx));
                    break;
                }
                case Opcode::INDEX_GET_2D: {
                    RhoValue idx2      = pop();
                    RhoValue idx1      = pop();
                    RhoValue container = pop();
                    push(doIndexGet2D(container, idx1, idx2));
                    break;
                }
                case Opcode::INDEX_SET: {
                    RhoValue val       = pop();
                    RhoValue idx       = pop();
                    RhoValue container = pop();
                    RhoValue updated   = doIndexSet(std::move(container), idx, val);
                    // For value types the caller should store back; here we just push
                    // the updated container so the next instruction (e.g. STORE_LOCAL)
                    // can save it.
                    push(std::move(updated));
                    break;
                }
                case Opcode::INDEX_SET_2D: {
                    RhoValue val       = pop();
                    RhoValue idx2      = pop();
                    RhoValue idx1      = pop();
                    RhoValue container = pop();
                    RhoValue updated   = doIndexSet2D(std::move(container), idx1, idx2, val);
                    push(std::move(updated));
                    break;
                }
                case Opcode::MEMBER_GET: {
                    size_t nameIdx = static_cast<size_t>(operand);
                    if (nameIdx >= chunk().names.size())
                        throw VMError("MEMBER_GET: name index out of range");
                    const std::string& fieldName = chunk().names[nameIdx];
                    RhoValue obj = pop();
                    if (std::holds_alternative<std::shared_ptr<RhoRecord>>(obj)) {
                        auto rec = std::get<std::shared_ptr<RhoRecord>>(obj);
                        if (!rec) throw VMError("MEMBER_GET: null record");
                        push(rec->getField(fieldName));
                    } else {
                        throw VMError("MEMBER_GET: value of type '" +
                                      typeToString(getValueType(obj)) + "' has no member '" +
                                      fieldName + "'");
                    }
                    break;
                }
                case Opcode::SLICE_GET: {
                    int32_t flags   = operand;
                    bool hasEnd     = (flags & 2) != 0;
                    bool hasStart   = (flags & 1) != 0;
                    int64_t endVal   = 0;
                    int64_t startVal = 0;
                    if (hasEnd)   endVal   = toInt(pop());
                    if (hasStart) startVal = toInt(pop());
                    RhoValue container = pop();
                    push(doSliceGet(container, hasStart, hasEnd, startVal, endVal));
                    break;
                }

                // ----------------------------------------------------------
                // COERCE
                // ----------------------------------------------------------
                case Opcode::COERCE: {
                    RhoType target = static_cast<RhoType>(operand);
                    RhoValue v = pop();
                    push(coerceValue(v, target));
                    break;
                }

                // ----------------------------------------------------------
                // Exception handling
                // ----------------------------------------------------------
                case Opcode::THROW: {
                    RhoValue exVal = pop();
                    // Convert to string if not already, for display.
                    // We keep the original value for the catch handler.
                    handleException(std::move(exVal));
                    break;
                }
                case Opcode::SETUP_TRY: {
                    TryFrame tf;
                    tf.catchTarget = operand;
                    tf.stackDepth  = stack_.size();
                    tf.frameDepth  = frames_.size();
                    tryStack_.push_back(tf);
                    break;
                }
                case Opcode::TEAR_DOWN_TRY:
                    if (!tryStack_.empty())
                        tryStack_.pop_back();
                    break;
                case Opcode::CATCH_BIND: {
                    // TOS is the exception value pushed by handleException.
                    size_t slot = frame().bp + static_cast<size_t>(operand);
                    if (slot >= stack_.size())
                        throw VMError("CATCH_BIND: slot out of range");
                    stack_[slot] = pop();
                    break;
                }

                // ----------------------------------------------------------
                // Iteration
                // ----------------------------------------------------------
                case Opcode::MAKE_ITER: {
                    RhoValue iterable = pop();
                    iterStack_.push_back(std::make_shared<RhoIterator>(iterable));
                    break;
                }
                case Opcode::ITER_NEXT: {
                    if (iterStack_.empty())
                        throw VMError("ITER_NEXT: iterator stack is empty");
                    auto& iter = iterStack_.back();
                    if (!iter->hasNext()) {
                        frame().ip = static_cast<size_t>(operand);
                    } else {
                        push(iter->next());
                    }
                    break;
                }
                case Opcode::ITER_POP:
                    if (!iterStack_.empty())
                        iterStack_.pop_back();
                    break;

                // ----------------------------------------------------------
                // Miscellaneous
                // ----------------------------------------------------------
                case Opcode::NOP:
                    break;

                case Opcode::HALT: {
                    if (!stack_.empty())
                        return pop();
                    return RhoValue{int64_t(0)};
                }

                default:
                    throw VMError("Unknown opcode: " +
                                  std::to_string(static_cast<int>(op)));
                } // end switch

            } catch (UserException& e) {
                // A C++ UserException from THROW or builtins — try to catch it.
                if (!handleException(e.value())) {
                    throw;
                }
            } catch (RuntimeError& e) {
                // Convert to string and try to catch.
                RhoValue msg{std::string(e.what())};
                if (!handleException(std::move(msg))) {
                    throw;
                }
            }

        } // end while(true)
    } // end execute()

}; // class VM

} // namespace Rhodesia

#endif // RHODESIA_VM_HPP
