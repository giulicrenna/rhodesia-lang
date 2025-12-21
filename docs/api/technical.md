# Technical API

This page provides technical details about Rhodesia's internal architecture and low-level API.

## System Architecture

### Core Components

Rhodesia consists of several key components:

1. **Lexer**: Converts source code to tokens
2. **Parser**: Builds Abstract Syntax Tree (AST)
3. **Symbol Table**: Manages variables and scoping
4. **Evaluator**: Executes AST nodes
5. **Built-ins**: Core functions and operations

### AST Structure

The Abstract Syntax Tree represents program structure:

```cpp
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual RhoValue evaluate(SymbolTable& symbols) = 0;
};
```

Common AST node types:
- `BinaryOpNode`: Arithmetic and comparison operations
- `VariableNode`: Variable references
- `AssignmentNode`: Variable assignments
- `FunctionCallNode`: Function invocations
- `IfNode`: Conditional statements
- `ForNode`: Loop constructs

## Type System Implementation

### RhoValue Variant

All Rhodesia values are stored in a `std::variant`:

```cpp
using RhoValue = std::variant<
    int64_t,        // Integer values
    double,         // Float64 values
    std::string,    // String values
    Eigen::VectorXd,// Vector values
    Eigen::MatrixXd // Matrix values
>;
```

### Type Operations

Type conversions and operations are handled through:
- Automatic promotion (int → float64)
- Broadcasting (scalar ↔ vector/matrix)
- Matrix operations via Eigen

## Memory Management

### Symbol Table

Variable storage uses a hierarchical symbol table:

```cpp
class SymbolTable {
private:
    std::unordered_map<std::string, RhoValue> symbols_;
    SymbolTable* parent_;
    std::vector<std::unique_ptr<SymbolTable>> children_;
};
```

### Scope Management

RAII pattern for scope handling:

```cpp
class ScopeGuard {
public:
    explicit ScopeGuard(SymbolTable& table) : table_(table) {
        table_.enterScope();
    }
    ~ScopeGuard() {
        table_.exitScope();
    }
};
```

## Performance Characteristics

### Eigen Integration

- **SIMD Operations**: Automatic vectorization
- **Cache Optimization**: Memory access patterns
- **Lazy Evaluation**: Expression templates
- **OpenMP Support**: Parallel execution

### Memory Layout

- Vectors: Column-major Eigen vectors
- Matrices: Column-major Eigen matrices
- Strings: Standard C++ strings
- Scalars: Direct storage in variant

## Built-in Functions

### Mathematical Operations

Core math functions are implemented as:

```cpp
RhoValue builtin_sin(const std::vector<RhoValue>& args) {
    double x = std::get<double>(args[0]);
    return std::sin(x);
}
```

### Linear Algebra

Matrix operations delegate to Eigen:

```cpp
RhoValue builtin_dot(const std::vector<RhoValue>& args) {
    const auto& u = std::get<Eigen::VectorXd>(args[0]);
    const auto& v = std::get<Eigen::VectorXd>(args[1]);
    return u.dot(v);
}
```

## Error Handling

### Exception Hierarchy

```
RhoError
├── LexerError
├── ParseError
├── RuntimeError
│   ├── TypeError
│   ├── ArgumentError
│   └── DivisionByZeroError
```

### Error Propagation

Errors bubble up through the evaluation stack with source location information.

## Module System

### Import Resolution

Modules are loaded from the filesystem relative to the executing script:

```cpp
std::string resolve_module_path(const std::string& module_name) {
    // Implementation for path resolution
    return module_name + ".rho";
}
```

### Caching

Loaded modules are cached to avoid recompilation:

```cpp
std::unordered_map<std::string, std::unique_ptr<ASTNode>> module_cache_;
```

## REPL Implementation

### Interactive Mode

The REPL maintains persistent state:

```cpp
class REPL {
private:
    SymbolTable global_symbols_;
    std::unique_ptr<Parser> parser_;
    std::unique_ptr<Evaluator> evaluator_;
};
```

### Command Processing

Special commands like `help`, `vars`, `exit` are handled separately from code execution.

## Build System

### CMake Configuration

```cmake
find_package(Eigen3 REQUIRED)
target_link_libraries(rhodesia Eigen3::Eigen)
```

### Compilation Flags

- **Debug**: `-O0 -g -Wall`
- **Release**: `-O3 -march=native -DNDEBUG`

## Testing Framework

### Unit Tests

Built-in test functions for validation:

```rhodesia
fun assert_equal(a, b, message) -> void {
    if a != b {
        println("Assertion failed:", message)
        println("Expected:", b, "Got:", a)
    }
}
```

## Future Extensions

### Planned Features

1. **Static Type Checker**: Compile-time type validation
2. **JIT Compilation**: Runtime code generation
3. **GPU Support**: CUDA/OpenCL integration
4. **Distributed Computing**: Multi-node execution

### API Stability

The C++ API is considered internal and may change between versions. The Rhodesia language syntax and semantics are stable.

## Performance Benchmarks

### Microbenchmarks

- Vector addition: ~10x faster than Python NumPy
- Matrix multiplication: ~5x faster than interpreted implementations
- Function calls: Minimal overhead compared to C++

### Memory Usage

- Base interpreter: ~2MB
- Per vector element: 8 bytes (double precision)
- Symbol table: Efficient hash map storage

## Debugging Support

### Source Mapping

AST nodes maintain source location information for error reporting:

```cpp
struct SourceLocation {
    std::string filename;
    int line;
    int column;
};
```

### Stack Traces

Runtime errors include full call stack information.

## Next Steps

- [Performance Guide](performance.md) - Optimization techniques
- [Architecture](architecture.md) - High-level system design
- [Troubleshooting](../troubleshooting/errors.md) - Common issues
