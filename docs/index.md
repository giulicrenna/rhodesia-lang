# Rhodesia Language Documentation

Welcome to the official documentation for Rhodesia, a high-performance programming language designed for data science and numerical computing.

## About Rhodesia

Rhodesia is an imperative, statically-typed language that combines clean syntax with the power of Eigen for high-performance numerical computations. It's designed for:

- **Data Science**: Built-in support for vectors, matrices, and statistical operations
- **Numerical Computing**: Optimized linear algebra operations
- **Machine Learning**: Easy implementation of algorithms
- **Scientific Computing**: High-performance mathematical operations

## Getting Started

### Installation

```bash
# Clone the repository
git clone https://github.com/giulicrenna/rhodesia-lang.git
cd rhodesia-lang

# Build the language
make
```

### Run Your First Program

```bash
# Run the REPL
./rhodesia

# Run a script file
./rhodesia examples/hello.rho
```

## Documentation Sections

### Language Reference
Learn the syntax, types, and control structures of Rhodesia.

[**Language Basics**](language/syntax.md) - Variables, types, and expressions
[**Control Flow**](language/control-flow.md) - If statements, loops, and functions
[**Type System**](language/types.md) - Understanding Rhodesia's type system
[**Collections**](language/collections.md) - Set, Tuple, and Record types with literal syntax
[**Match Statement**](language/match.md) - Pattern matching over a single value

### Standard Library
Explore the built-in functions and data structures.

[**Functions**](standard-library/functions.md) - Built-in mathematical and utility functions
[**Vectors & Matrices**](standard-library/vectors-matrices.md) - Vector and matrix operations
[**Built-ins**](standard-library/builtins.md) - Core built-in functions
[**Net Module**](standard-library/net.md) - TCP sockets and HTTP/1.1 client

### Examples
Practical code examples to learn by doing.

[**Basic Examples**](examples/basics.md) - Hello World and basic operations
[**Data Structures**](examples/data-structures.md) - Working with vectors and matrices
[**Collections**](examples/collections.md) - Set, Tuple, Record, and Match examples
[**Input/Output**](examples/io.md) - File operations and user input
[**Machine Learning**](examples/machine-learning.md) - Regression, clustering, and more
[**Modules**](examples/modules.md) - Code organization with modules

### Tutorials
Step-by-step guides for common tasks.

[**Basic Tutorial**](tutorials/basics.md) - Getting started with Rhodesia
[**Data Science**](tutorials/data-science.md) - Statistical analysis and visualization
[**Algorithms**](tutorials/algorithms.md) - Implementing numerical algorithms

### API & Architecture
Technical details and performance optimization.

[**Architecture**](api/architecture.md) - System architecture and design
[**Technical API**](api/technical.md) - Low-level API reference
[**Performance Guide**](api/performance.md) - Optimization techniques

### Troubleshooting
Debugging and error handling.

[**Error Reference**](troubleshooting/errors.md) - Common errors and solutions
[**Debugging Guide**](troubleshooting/debugging.md) - Debugging techniques

## Quick Example

```rhodesia
// Basic Rhodesia program
println("Hello, Rhodesia!")

// Vector operations
vec: u = [1, 2, 3]
vec: v = [4, 5, 6]
float64: dot_product = math.dot(u, v)
println("Dot product:", dot_product)

// Matrix operations
mat: A = [[1, 2], [3, 4]]
mat: B = [[5, 6], [7, 8]]
mat: C = A * B
println("Matrix multiplication:")
println(C)
```

## Features

- **High Performance**: Built on Eigen for optimized numerical operations
- **Clean Syntax**: Easy to read and write
- **Type Safety**: Static typing with automatic type inference
- **Vectorized Operations**: Efficient array operations
- **Networking**: Built-in `net` module — TCP sockets and HTTP/1.1 client, zero extra dependencies
- **Extensible**: Easy to add new functions and modules

## Community

- **GitHub**: [https://github.com/giulicrenna/rhodesia-lang](https://github.com/giulicrenna/rhodesia-lang)
- **Issues**: Report bugs and request features
- **Contributions**: Pull requests welcome!

---

© 2025 Rhodesia Language Project
