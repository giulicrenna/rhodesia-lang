# Rhodesia

<p align="center">
  <img src="images/RHODESIA.png" alt="Rhodesia Logo" width="200">
</p>

<p align="center">
  <strong>A strongly-typed imperative language for Data Science</strong>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/version-0.1.0-blue.svg" alt="Version">
  <img src="https://img.shields.io/badge/C++-20-green.svg" alt="C++20">
  <img src="https://img.shields.io/badge/backend-Eigen3-orange.svg" alt="Eigen3">
  <img src="https://img.shields.io/badge/license-MIT-lightgrey.svg" alt="License">
</p>

---

## Overview

**Rhodesia** is a modern, strongly-typed programming language designed specifically for Data Science and numerical computing. It combines a clean, readable syntax with high-performance numerical operations powered by the Eigen library.

### Key Features

- **Native Vector/Matrix Types**: First-class support for `vec` and `mat` types
- **Strong Typing**: Type system with `int`, `float64`, `str`, `vec`, and `mat`
- **String Manipulation**: Complete string module with 17+ functions
- **High Performance**: Numerical backend powered by Eigen3 (SIMD, cache optimization)
- **Rich Standard Library**:
  - **Math Module**: Trigonometry, hyperbolic functions, rounding, min/max, linear algebra
  - **Statistics Module**: Descriptive statistics, correlation, moments, normalization
  - **Numerical Module**: Integration, differentiation, polynomial fitting, equation solving
  - **String Module**: Text manipulation, search, transformation, validation
- **Clean Syntax**: Intuitive declaration syntax: `type: name = value`
- **Module System**: Import and organize code with selective imports and aliases
- **User-defined Functions**: First-class functions with type annotations
- **Interactive REPL**: Explore and prototype interactively

---

## Quick Start

### Installation

#### Prerequisites

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential cmake libeigen3-dev

# Fedora
sudo dnf install gcc-c++ cmake eigen3-devel

# macOS (Homebrew)
brew install cmake eigen
```

#### Build from Source

```bash
git clone https://github.com/yourusername/rhodesia.git
cd rhodesia
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Running Rhodesia

```bash
# Interactive REPL
./rhodesia

# Execute a file
./rhodesia examples/regression.rho

# Execute inline code
./rhodesia -e "println(norm([1, 2, 3]))"
```

---

## Language Guide

### Type System

Rhodesia has five primitive types:

| Type      | Description              | C++ Mapping        |
|-----------|--------------------------|-------------------|
| `int`     | 64-bit integer           | `int64_t`         |
| `float64` | Double precision float   | `double`          |
| `str`     | String                   | `std::string`     |
| `vec`     | Column vector            | `Eigen::VectorXd` |
| `mat`     | Matrix                   | `Eigen::MatrixXd` |

### Variable Declaration

```rhodesia
// Syntax: type: name = value
int: x = 42
float64: pi = 3.14159
str: nombre = "Rhodesia"
vec: v = [1.0, 2.0, 3.0]
mat: m = [[1, 2], [3, 4]]
```

### Operators

#### Arithmetic
```rhodesia
int: a = 10 + 5      // Addition
int: b = 10 - 5      // Subtraction
int: c = 10 * 5      // Multiplication
float64: d = 10 / 3  // Division (always float)
int: e = 10 % 3      // Modulo
```

#### Matrix/Vector Operations
```rhodesia
vec: u = [1, 2, 3]
vec: v = [4, 5, 6]

vec: sum = u + v           // Element-wise addition
vec: diff = u - v          // Element-wise subtraction
vec: scaled = 2.0 * u      // Scalar multiplication
float64: d = dot(u, v)     // Dot product: 32.0

mat: A = [[1, 2], [3, 4]]
mat: B = [[5, 6], [7, 8]]

mat: prod = A * B          // Matrix multiplication
mat: At = transpose(A)     // Transpose
mat: Ainv = inv(A)         // Matrix inverse
```

#### Comparison & Logical
```rhodesia
int: eq = (5 == 5)   // 1 (true)
int: ne = (5 != 3)   // 1 (true)
int: lt = (3 < 5)    // 1 (true)
int: gt = (5 > 3)    // 1 (true)

int: and_op = (1 && 1)  // 1 (true)
int: or_op = (0 || 1)   // 1 (true)
int: not_op = !0        // 1 (true)
```

### Control Flow

#### If-Else
```rhodesia
int: x = 10

if x > 0 {
    println("positive")
} else if x < 0 {
    println("negative")
} else {
    println("zero")
}
```

#### For Loop
```rhodesia
// Iterate over a vector
for i in range(10) {
    println(i)
}

// Custom range
for val in range(5, 15) {
    println(val)
}

// Iterate over vector elements
vec: data = [1.5, 2.5, 3.5, 4.5]
for x in data {
    println(sqrt(x))
}
```

#### While Loop
```rhodesia
int: i = 0
while i < 10 {
    println(i)
    i = i + 1
}
```

#### Break and Continue
```rhodesia
for i in range(100) {
    if i == 5 {
        continue  // Skip iteration
    }
    if i == 10 {
        break     // Exit loop
    }
    println(i)
}
```

### Functions

```rhodesia
// Function definition syntax
fun function_name(type: param1, type: param2) -> return_type {
    // body
    return value
}

// Example: Euclidean norm
fun euclidean_norm(vec: v) -> float64 {
    return sqrt(dot(v, v))
}

// Example: Linear regression coefficients (OLS)
fun ols_coefficients(mat: X, vec: y) -> vec {
    mat: xt = transpose(X)
    mat: xtx = xt * X
    mat: xtx_inv = inv(xtx)
    vec: xty = xt * y
    return xtx_inv * xty
}

// Usage
vec: v = [3, 4]
float64: n = euclidean_norm(v)  // 5.0
println(n)
```

### Built-in Functions

#### Linear Algebra
| Function | Description | Example |
|----------|-------------|---------|
| `norm(v)` | Euclidean norm (L2) | `norm([3, 4])` → `5.0` |
| `dot(u, v)` | Dot product | `dot([1,2], [3,4])` → `11.0` |
| `transpose(m)` | Matrix transpose | `transpose([[1,2],[3,4]])` |
| `inv(m)` | Matrix inverse | `inv([[1,2],[3,4]])` |
| `sum(v)` | Sum of elements | `sum([1, 2, 3])` → `6.0` |
| `mean(v)` | Arithmetic mean | `mean([1, 2, 3])` → `2.0` |

#### Creation Functions
| Function | Description | Example |
|----------|-------------|---------|
| `zeros(n)` | Zero vector | `zeros(3)` → `[0, 0, 0]` |
| `zeros(n, m)` | Zero matrix | `zeros(2, 3)` |
| `ones(n)` | Vector of ones | `ones(3)` → `[1, 1, 1]` |
| `ones(n, m)` | Matrix of ones | `ones(2, 3)` |
| `eye(n)` | Identity matrix | `eye(3)` |
| `range(n)` | Sequence [0, n) | `range(5)` → `[0,1,2,3,4]` |
| `range(a, b)` | Sequence [a, b) | `range(2, 5)` → `[2,3,4]` |

#### Math Functions
| Function | Description | Vectorized |
|----------|-------------|------------|
| `sqrt(x)` | Square root | ✓ |
| `exp(x)` | Exponential | ✓ |
| `log(x)` | Natural log | ✓ |
| `abs(x)` | Absolute value | ✓ |
| `sin(x)` | Sine | Scalar only |
| `cos(x)` | Cosine | Scalar only |
| `tan(x)` | Tangent | Scalar only |

#### Information Functions
| Function | Description | Example |
|----------|-------------|---------|
| `rows(m)` | Number of rows | `rows([[1,2],[3,4]])` → `2` |
| `cols(m)` | Number of columns | `cols([[1,2],[3,4]])` → `2` |
| `size(v)` | Total elements | `size([1,2,3])` → `3` |

#### I/O Functions
| Function | Description |
|----------|-------------|
| `print(...)` | Print without newline |
| `println(...)` | Print with newline |

#### String Functions
| Function | Description | Example |
|----------|-------------|---------|
| `string.length(s)` | Get string length | `string.length("Hello")` → `5` |
| `string.upper(s)` | Convert to uppercase | `string.upper("hello")` → `"HELLO"` |
| `string.lower(s)` | Convert to lowercase | `string.lower("HELLO")` → `"hello"` |
| `string.trim(s)` | Remove whitespace | `string.trim("  hi  ")` → `"hi"` |
| `string.contains(s, sub)` | Check if contains | `string.contains("hello", "ell")` → `1` |
| `string.replace(s, old, new)` | Replace substring | `string.replace("a b", "b", "c")` → `"a c"` |
| `string.reverse(s)` | Reverse string | `string.reverse("abc")` → `"cba"` |
| `string.repeat(s, n)` | Repeat string | `string.repeat("Ha", 3)` → `"HaHaHa"` |
| ... | *See [STANDARD_LIBRARY.md](STANDARD_LIBRARY.md) for complete list* | |

---

## Examples

### Linear Regression

```rhodesia
// data_science/regression.rho
// Simple Linear Regression using Ordinary Least Squares

// Design matrix (with intercept column)
mat: X = [
    [1, 1],
    [1, 2],
    [1, 3],
    [1, 4],
    [1, 5]
]

// Response variable
vec: y = [2.1, 3.9, 6.1, 8.0, 9.8]

// OLS estimation: β = (X'X)^(-1) X'y
fun ols(mat: X, vec: y) -> vec {
    mat: xt = transpose(X)
    mat: xtx = xt * X
    mat: xtx_inv = inv(xtx)
    vec: xty = xt * y
    return xtx_inv * xty
}

vec: beta = ols(X, y)

println("Coefficients:")
println("  Intercept:", beta[0])
println("  Slope:", beta[1])

// Predictions
vec: y_pred = X * beta

println("Predictions:")
for i in range(size(y)) {
    println("  y[", i, "] = ", y[i], " -> pred: ", y_pred[i])
}

// R-squared
float64: y_mean = mean(y)
float64: ss_tot = sum((y - y_mean) * (y - y_mean))
float64: ss_res = sum((y - y_pred) * (y - y_pred))
float64: r2 = 1 - ss_res / ss_tot

println("R-squared:", r2)
```

### Matrix Operations

```rhodesia
// matrix_demo.rho
// Demonstrating matrix operations

mat: A = [[1, 2, 3],
          [4, 5, 6],
          [7, 8, 10]]

println("Matrix A:")
println(A)

println("Determinant check via inverse...")
mat: A_inv = inv(A)
println("A inverse:")
println(A_inv)

mat: should_be_identity = A * A_inv
println("A * A^(-1) (should be ~identity):")
println(should_be_identity)

println("Frobenius norm:", norm(A))
println("Dimensions:", rows(A), "x", cols(A))
```

### Vector Operations

```rhodesia
// vector_demo.rho
// Vector computations

vec: a = [1, 2, 3, 4, 5]
vec: b = [5, 4, 3, 2, 1]

println("Vector a:", a)
println("Vector b:", b)
println("a + b:", a + b)
println("a - b:", a - b)
println("a * 2:", a * 2)
println("dot(a, b):", dot(a, b))
println("norm(a):", norm(a))
println("mean(a):", mean(a))
println("sum(a):", sum(a))

// Element-wise operations
println("sqrt(a):", sqrt(a))
println("exp(a/10):", exp(a / 10))
```

### Control Flow

```rhodesia
// control_flow.rho
// Demonstrating control structures

// FizzBuzz
for i in range(1, 21) {
    if i % 15 == 0 {
        println("FizzBuzz")
    } else if i % 3 == 0 {
        println("Fizz")
    } else if i % 5 == 0 {
        println("Buzz")
    } else {
        println(i)
    }
}

// Find first number divisible by 7
int: found = 0
for n in range(1, 100) {
    if n % 7 == 0 {
        println("First multiple of 7:", n)
        found = n
        break
    }
}

// Factorial using while
int: n = 10
int: factorial = 1
int: i = 1
while i <= n {
    factorial = factorial * i
    i = i + 1
}
println("Factorial of", n, "is", factorial)
```

---

## Documentation

- **[Standard Library](STANDARD_LIBRARY.md)**: Comprehensive guide to built-in modules (math, stats, numerical, vector, matrix, string)
- **[Module System](MODULES.md)**: How to create and import custom modules
- **[String Implementation](STRING_IMPLEMENTATION.md)**: Complete guide to string type and module
- **[Roadmap](ROADMAP.md)**: Development plan and upcoming features

---

## REPL Commands

When running in interactive mode:

| Command | Description |
|---------|-------------|
| `help` | Show available commands |
| `vars` | List all defined variables |
| `exit` / `quit` | Exit the REPL |

Multi-line input is supported automatically when braces are unbalanced.

---

## Project Structure

```
rhodesia/
├── CMakeLists.txt        # Build configuration
├── README.md             # This file
├── ROADMAP.md            # Development roadmap
├── include/
│   ├── RhoValue.hpp      # Type system (variant-based)
│   ├── Token.hpp         # Token definitions
│   ├── Lexer.hpp         # Lexical analyzer
│   ├── Error.hpp         # Error handling
│   ├── AST.hpp           # Abstract Syntax Tree
│   ├── Parser.hpp        # Recursive descent parser
│   ├── SymbolTable.hpp   # Variable/function storage
│   ├── Builtins.hpp      # Built-in functions
│   └── Evaluator.hpp     # AST interpreter
├── src/
│   └── main.cpp          # Entry point (REPL + file execution)
└── examples/
    ├── regression.rho
    ├── matrix_demo.rho
    ├── vector_demo.rho
    └── control_flow.rho
```

---

## Roadmap

See [ROADMAP.md](ROADMAP.md) for the complete development plan.

### Current Status: Phase 1 (Core Language)

- [x] Lexer
- [x] Parser
- [x] AST
- [x] Evaluator
- [x] Symbol Table
- [x] Built-in Functions
- [x] REPL

### Upcoming

- [ ] Static type checker
- [ ] Slicing syntax (`v[1:5]`, `m[0:2, :]`)
- [ ] File I/O (`load_csv`, `save_csv`)
- [ ] Random number generation
- [ ] Statistical functions

---

## Contributing

Contributions are welcome! Please read the roadmap to understand the project's direction.

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## Acknowledgments

- **Eigen3**: High-performance linear algebra library
- **LLVM/Clang**: Inspiration for clean language design
- **Julia**: Inspiration for scientific computing syntax
