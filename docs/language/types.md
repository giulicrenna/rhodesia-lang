---
title: Types
layout: default
nav_order: 2
parent: Language Reference
---

# Type System

Rhodesia has a static type system with five primitive types designed for numerical computing and data science.

## Primitive Types

| Type | Description | C++ Mapping | Example |
|------|-------------|-----------|---------|
| `int` | 64-bit integer | `int64_t` | `42`, `-15` |
| `float64` | Double precision float | `double` | `3.14159`, `-2.5` |
| `vec` | Column vector | `Eigen::VectorXd` | `[1.0, 2.0, 3.0]` |
| `mat` | Matrix | `Eigen::MatrixXd` | `[[1, 2], [3, 4]]` |
| `str` | Text string | `std::string` | `"Hello World"` |
| `bool` | Boolean value | `bool` | `true`, `false` |

## Collection Types

| Type | Description | Literal Syntax | Example |
|------|-------------|----------------|---------|
| `arr` | Ordered heterogeneous array | — | `arr.create(...)` |
| `set` | Unordered, no duplicates | `{v1, v2, ...}` | `{1, 2, 3}` |
| `tuple` | Immutable ordered sequence | `(v1, v2, ...)` | `(1, "ok", true)` |
| `record` | Named fields structure | `{k: v, ...}` | `{x: 1, y: 2}` |
| `map` | Key-value store | — | `map.create(...)` |

See [Collections](collections.md) for a full reference on `set`, `tuple`, and `record`.

## Type Declarations

### Variable Declaration Syntax

```rhodesia
type: name = value
```

### Examples

```rhodesia
int: age = 25
float64: pi = 3.14159
vec: coordinates = [10.5, 20.3]
mat: transform = [[1, 0], [0, 1]]
string: name = "Rhodesia"
```

## Literals

### Numeric Literals

```rhodesia
// Integers
int: a = 42
int: b = -15

// Floats
float64: pi = 3.14159
float64: e = 2.718

// Scientific notation
float64: avogadro = 6.022e23
```

### Vector Literals

```rhodesia
vec: v1 = [1, 2, 3]
vec: v2 = [1.0, 2.5, -3.14]
vec: empty = []  // Empty vector
```

### Matrix Literals

```rhodesia
mat: m1 = [[1, 2], [3, 4]]
mat: m2 = [[1.0, 2.0, 3.0],
           [4.0, 5.0, 6.0]]
```

### String Literals

```rhodesia
string: greeting = "Hello World"
string: multiline = "Line 1\nLine 2"
string: escaped = "Quotes: \", Backslash: \\"
```

### Boolean Literals

```rhodesia
int: t = true   // 1
int: f = false  // 0
```

## Type Compatibility

### Automatic Type Conversion

Rhodesia performs automatic conversion in arithmetic operations:

- `int` ↔ `float64`: Automatic conversion
- Scalar ↔ vector/matrix: Broadcasting for operations like `2 * vector`

### Type Safety

```rhodesia
// Valid: automatic conversion
int: i = 5
float64: f = i  // i is converted to 5.0

// Valid: broadcasting
vec: v = [1, 2, 3]
vec: scaled = 2 * v  // [2, 4, 6]

// Invalid: type mismatch
string: s = "hello"
int: x = s  // TypeError at runtime
```

## Operators and Types

### Arithmetic Operators

| Operator | Applicable Types | Result Type |
|----------|------------------|-------------|
| `+`, `-`, `*`, `/` | int, float64, vec, mat | Same as operands |
| `%` | int only | int |

### Comparison Operators

| Operator | Applicable Types | Result |
|----------|------------------|--------|
| `==`, `!=`, `<`, `<=`, `>`, `>=` | All numeric types | int (0 or 1) |

### Logical Operators

| Operator | Applicable Types | Result |
|----------|------------------|--------|
| `and`, `or` | int only | int (0 or 1) |
| `not` | int only | int (0 or 1) |

## Type Inference

Rhodesia does not have type inference - all variables must be explicitly typed:

```rhodesia
// Valid
int: x = 5

// Invalid - no type inference
x = 5  // Error: must specify type
```

## Variable Rules

- Must begin with letter or underscore (`_`)
- Can contain letters, digits, underscores
- Case-sensitive
- Must be declared before use
- Must be initialized at declaration

## Examples

### Type Conversions

```rhodesia
int: i = 42
float64: f = i  // 42.0

float64: pi = 3.14159
int: rounded = pi  // 3 (truncation)

vec: v = [1, 2, 3]
vec: doubled = 2.0 * v  // [2.0, 4.0, 6.0]
```

### Matrix Operations

```rhodesia
mat: A = [[1, 2], [3, 4]]
mat: B = [[5, 6], [7, 8]]

mat: C = A + B  // Element-wise addition
mat: D = A * B  // Matrix multiplication

vec: v = [1, 2]
vec: result = A * v  // Matrix-vector multiplication
```

### Type Safety Demonstration

```rhodesia
// This works
int: num = 5
float64: converted = num

// This would cause a runtime error
// string: text = "hello"
// int: bad = text  // TypeError

// Correct way
string: text = "hello"
int: length = size(text)  // 5
```

## Next Steps

- [Language Syntax](syntax.md) - Complete syntax reference
- [Collections](collections.md) - Set, Tuple, and Record types
- [Match Statement](match.md) - Pattern matching
- [Standard Library](../standard-library/functions.md) - Built-in functions
