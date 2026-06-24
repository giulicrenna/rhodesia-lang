/**
 * @file Bytecode.hpp
 * @brief Bytecode VM instruction set and chunk representation for Rhodesia language
 *
 * Defines the Opcode enum, Instruction struct, Chunk (compiled code unit),
 * and RhoIterator (runtime iterator over iterable RhoValues).
 */

#pragma once
#ifndef RHODESIA_BYTECODE_HPP
#define RHODESIA_BYTECODE_HPP

#include "RhoValue.hpp"
#include "Error.hpp"

#include <Eigen/Dense>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include <memory>

namespace Rhodesia {

/**
 * @brief Bytecode instruction opcodes for the Rhodesia VM
 *
 * Operand conventions (stored in Instruction::operand, int32_t):
 *   - index / count operands are stored directly
 *   - jump targets are absolute positions in Chunk::code
 *   - CALL_BUILTIN encodes (name_idx << 8 | argc)
 *   - CALL_MODULE: operand = name_idx, next instruction encodes argc
 *     (implemented as two consecutive instructions or a combined encoding –
 *      see CALL_MODULE note below)
 *   - BUILD_MAT encodes (rows << 16 | cols)
 *   - COERCE: operand = static_cast<int32_t>(RhoType)
 *   - SLICE_GET: operand flags: bit 0 = has_start, bit 1 = has_end
 */
enum class Opcode : uint8_t {
    LOAD_CONST,         // operand = constants index
    LOAD_NULL,          // push null
    LOAD_TRUE,          // push true
    LOAD_FALSE,         // push false

    LOAD_LOCAL,         // operand = local slot index
    STORE_LOCAL,        // operand = local slot index

    LOAD_UPVALUE,       // operand = upvalue index
    STORE_UPVALUE,      // operand = upvalue index

    LOAD_GLOBAL,        // operand = names index
    STORE_GLOBAL,       // operand = names index

    POP,                // discard top of stack
    DUP,                // duplicate top of stack

    
    // Arithmetic
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    NEGATE,             // unary minus

    
    // Comparison  (pop two, push bool)
    EQ,
    NE,
    LT,
    GT,
    LE,
    GE,

    
    // Logical
    NOT,                // unary logical NOT

    
    // Jumps  (absolute target stored in operand)
    JUMP,                   // unconditional jump
    JUMP_IF_FALSE,          // pop + jump if false
    JUMP_IF_TRUE,           // pop + jump if true
    JUMP_IF_FALSE_PEEK,     // jump if false, do NOT pop
    JUMP_IF_TRUE_PEEK,      // jump if true,  do NOT pop

    
    // Functions & closures
    CALL,               // operand = argc  (callee is on stack below args)
    CALL_BUILTIN,       // operand = (name_idx << 8) | argc
    CALL_MODULE,        // operand = name_idx  (argc follows as next instr operand,
                        //           or encoded as CALL_MODULE_ARGC separately)
    RETURN,             // return top of stack
    RETURN_NULL,        // return null
    MAKE_CLOSURE,       // operand = constants index of a Chunk constant (RhoFunction)

    
    // Collection builders  (pop N values, push collection)
    BUILD_VEC,          // operand = element count  -> Eigen::VectorXd
    BUILD_MAT,          // operand = (rows << 16) | cols  -> Eigen::MatrixXd
    BUILD_SET,          // operand = element count  -> RhoSet
    BUILD_TUPLE,        // operand = element count  -> RhoTuple
    BUILD_RECORD,       // operand = field count (expects name,value pairs on stack) -> RhoRecord
    BUILD_ARR,          // operand = element count  -> RhoArray
    BUILD_MAP,          // no operand; pops key/value pairs from stack -> RhoMap
                        //   (pair count is pushed before BUILD_MAP as an int const)

    
    // Access operations
    INDEX_GET,          // pop index, pop collection, push value
    INDEX_GET_2D,       // pop col, pop row, pop matrix, push value
    INDEX_SET,          // pop value, pop index, pop collection  (collection mutated)
    INDEX_SET_2D,       // pop value, pop col, pop row, pop matrix
    MEMBER_GET,         // operand = names index  (field/key name)
    SLICE_GET,          // operand flags: 1=has_start, 2=has_end  (values on stack)
    LOAD_MODULE_CONST,  // operand = names index  (module.member access)

    
    // Type coercion
    COERCE,             // operand = static_cast<int32_t>(RhoType)

    
    // Exception handling
    THROW,              // pop and throw as UserException
    SETUP_TRY,          // operand = absolute position of catch block
    TEAR_DOWN_TRY,      // leave the try region normally (before catch)
    CATCH_BIND,         // operand = local slot index to store caught exception value

    
    // For-loop iteration
    MAKE_ITER,          // pop iterable, push RhoIterator
    ITER_NEXT,          // operand = absolute exit position;
                        //   if iterator exhausted jump there, else push next value
    ITER_POP,           // pop iterator off stack at end of loop

    
    // Miscellaneous
    NOP,
    HALT
};

// Instruction

/**
 * @brief A single bytecode instruction
 */
struct Instruction {
    Opcode  op;
    int32_t operand = 0;

    Instruction() : op(Opcode::NOP), operand(0) {}
    Instruction(Opcode o, int32_t arg = 0) : op(o), operand(arg) {}
};

// Chunk

/**
 * @brief A compiled code unit (function body, module top-level, etc.)
 *
 * A Chunk owns its instruction stream, constant pool, and name table.
 * Nested functions are stored as RhoFunction constants whose body pointer
 * points to another Chunk (wrapped in shared_ptr<void>).
 */
struct Chunk {
    
    // Metadata
    
    std::string name;       ///< Human-readable name (function name, "<module>", etc.)
    int         arity    = 0;  ///< Number of formal parameters
    int         numLocals = 0; ///< Total local variable slots (includes parameters)

    
    // Code and data
    
    std::vector<Instruction> code;       ///< Instruction stream
    std::vector<RhoValue>    constants;  ///< Constant pool
    std::vector<std::string> names;      ///< String pool (globals, member names, etc.)
    std::vector<RhoType>     paramTypes; ///< Optional declared parameter types

    
    // Upvalue descriptors (for closures)
    
    struct UpvalueDesc {
        bool isLocal; ///< true  -> captured from enclosing function's local slot
                      ///< false -> captured from enclosing function's upvalue
        int  index;   ///< Slot / upvalue index in the enclosing function
    };
    std::vector<UpvalueDesc> upvalueDescs;

    
    // Builder helpers
    

    /**
     * @brief Add a constant to the pool and return its index.
     */
    int addConstant(RhoValue value) {
        constants.push_back(std::move(value));
        return static_cast<int>(constants.size()) - 1;
    }

    /**
     * @brief Add a name string to the pool and return its index.
     *        Returns existing index if the name is already present.
     */
    int addName(const std::string& name_) {
        for (int i = 0; i < static_cast<int>(names.size()); ++i) {
            if (names[i] == name_) return i;
        }
        names.push_back(name_);
        return static_cast<int>(names.size()) - 1;
    }

    /**
     * @brief Emit an instruction and return its index in code[].
     */
    int emit(Opcode op, int32_t operand = 0) {
        code.emplace_back(op, operand);
        return static_cast<int>(code.size()) - 1;
    }

    /**
     * @brief Emit a jump instruction with a placeholder target (0).
     * @return Index of the emitted instruction (use patchJump later).
     */
    int emitJump(Opcode op) {
        return emit(op, 0);
    }

    /**
     * @brief Patch a previously emitted jump so its target is the current end of code.
     * @param jumpIdx Index returned by emitJump().
     */
    void patchJump(int jumpIdx) {
        code[jumpIdx].operand = static_cast<int32_t>(code.size());
    }

    /**
     * @brief Patch a previously emitted jump to a specific absolute target.
     * @param jumpIdx  Index of the jump instruction.
     * @param target   Absolute instruction index to jump to.
     */
    void patchJumpTo(int jumpIdx, int target) {
        code[jumpIdx].operand = static_cast<int32_t>(target);
    }

    /**
     * @brief Return the index of the *next* instruction that will be emitted.
     *        Useful for recording loop-start positions before emitting the body.
     */
    int currentPos() const {
        return static_cast<int>(code.size());
    }
};


// RhoIterator


/**
 * @brief Runtime iterator over any Rhodesia iterable value.
 *
 * Supports:
 *   - RangeGenerator  -> lazy integer range (no heap copy)
 *   - int64_t         -> treated as range(0, n), lazy
 *   - Eigen::VectorXd -> lazy index-based access (no copy)
 *   - RhoArray        -> lazy index-based access via shared_ptr
 *   - RhoTuple        -> lazy index-based access via shared_ptr
 *   - RhoSet          -> lazy index-based access via shared_ptr
 *   - RhoMap          -> materialized keys (unordered_map has no index access)
 *
 * Throws RuntimeError for unsupported types.
 */
class RhoIterator {
public:
    /**
     * @brief Construct an iterator from any iterable RhoValue.
     * @throws RuntimeError if the value type is not iterable.
     */
    explicit RhoIterator(const RhoValue& iterable)
    {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            //  RangeGenerator (lazy) 
            if constexpr (std::is_same_v<T, std::shared_ptr<RangeGenerator>>) {
                if (!arg) throw RuntimeError("RhoIterator: null RangeGenerator");
                int64_t start = arg->current();
                source_ = RangeSource{ start, start + static_cast<int64_t>(arg->size()), start };
            }

            //  int64_t treated as range(0, n) (lazy) 
            else if constexpr (std::is_same_v<T, int64_t>) {
                if (arg < 0)
                    throw RuntimeError("RhoIterator: cannot iterate over negative integer " + std::to_string(arg));
                source_ = RangeSource{ 0, arg, 0 };
            }

            //  Eigen::VectorXd (lazy, index-based) 
            else if constexpr (std::is_same_v<T, Eigen::VectorXd>) {
                source_ = VecSource{ arg, 0 };
            }

            //  RhoArray (lazy, index-based) 
            else if constexpr (std::is_same_v<T, std::shared_ptr<RhoArray>>) {
                if (!arg) throw RuntimeError("RhoIterator: null RhoArray");
                source_ = ArraySource{ arg, 0 };
            }

            //  RhoTuple (lazy, index-based) 
            else if constexpr (std::is_same_v<T, std::shared_ptr<RhoTuple>>) {
                if (!arg) throw RuntimeError("RhoIterator: null RhoTuple");
                source_ = TupleSource{ arg, 0 };
            }

            //  RhoSet (lazy, index-based via internal vector) 
            else if constexpr (std::is_same_v<T, std::shared_ptr<RhoSet>>) {
                if (!arg) throw RuntimeError("RhoIterator: null RhoSet");
                source_ = SetSource{ arg, 0 };
            }

            //  RhoMap -> materialize keys (unordered_map has no index access) 
            else if constexpr (std::is_same_v<T, std::shared_ptr<RhoMap>>) {
                if (!arg) throw RuntimeError("RhoIterator: null RhoMap");
                std::vector<RhoValue> keys;
                keys.reserve(arg->size());
                for (auto it = arg->begin(); it != arg->end(); ++it)
                    keys.emplace_back(it->first);
                source_ = MaterializedSource{ std::move(keys), 0 };
            }

            //  Unsupported 
            else {
                throw RuntimeError(
                    "RhoIterator: value of type '" +
                    typeToString(getValueType(RhoValue{arg})) +
                    "' is not iterable");
            }
        }, iterable);
    }

    
    // Iterator protocol
    

    bool hasNext() const {
        return std::visit([](const auto& s) -> bool {
            using S = std::decay_t<decltype(s)>;
            if constexpr (std::is_same_v<S, RangeSource>)
                return s.current < s.end;
            else if constexpr (std::is_same_v<S, VecSource>)
                return s.idx < static_cast<size_t>(s.vec.size());
            else if constexpr (std::is_same_v<S, ArraySource>)
                return s.idx < s.arr->size();
            else if constexpr (std::is_same_v<S, TupleSource>)
                return s.idx < s.tup->size();
            else if constexpr (std::is_same_v<S, SetSource>)
                return s.idx < s.set->size();
            else // MaterializedSource
                return s.idx < s.items.size();
        }, source_);
    }

    RhoValue next() {
        if (!hasNext()) throw RuntimeError("RhoIterator: iterator exhausted");
        return std::visit([](auto& s) -> RhoValue {
            using S = std::decay_t<decltype(s)>;
            if constexpr (std::is_same_v<S, RangeSource>)
                return RhoValue{s.current++};
            else if constexpr (std::is_same_v<S, VecSource>)
                return RhoValue{s.vec(static_cast<Eigen::Index>(s.idx++))};
            else if constexpr (std::is_same_v<S, ArraySource>)
                return s.arr->get(s.idx++);
            else if constexpr (std::is_same_v<S, TupleSource>)
                return s.tup->get(s.idx++);
            else if constexpr (std::is_same_v<S, SetSource>) {
                // RhoSet wraps a vector internally; iterate via begin()+offset
                auto it = s.set->begin();
                std::advance(it, static_cast<std::ptrdiff_t>(s.idx++));
                return *it;
            } else { // MaterializedSource
                return s.items[s.idx++];
            }
        }, source_);
    }

private:
    struct RangeSource       { int64_t start; int64_t end; int64_t current; };
    struct VecSource         { Eigen::VectorXd vec; size_t idx; };
    struct ArraySource       { std::shared_ptr<RhoArray> arr; size_t idx; };
    struct TupleSource       { std::shared_ptr<RhoTuple> tup; size_t idx; };
    struct SetSource         { std::shared_ptr<RhoSet>   set; size_t idx; };
    struct MaterializedSource{ std::vector<RhoValue> items; size_t idx; };

    std::variant<RangeSource, VecSource, ArraySource, TupleSource, SetSource, MaterializedSource> source_;
};

} // namespace Rhodesia

#endif // RHODESIA_BYTECODE_HPP
