---
title: Syntax
layout: default
nav_order: 1
parent: Language Reference
---

# Rhodesia Language Syntax

This page covers the basic syntax and structure of Rhodesia programs.

## Table of Contents

- [Comments](#comments)
- [Variables and Types](#variables-and-types)
- [Literals](#literals)
- [Operators](#operators)
- [Control Flow](#control-flow)
- [Functions](#functions)
- [Code Examples](#code-examples)

## Comments

Rhodesia supports both single-line and multi-line comments:

```rhodesia
// This is a single-line comment

/*
   This is a multi-line comment
   It can span multiple lines
*/
```

## Variables and Types

### Variable Declaration

```rhodesia
// Basic syntax: type: name = value
int: age = 25
float64: pi = 3.14159
vec: coordinates = [10.5, 20.3, 15.7]
mat: identity = [[1, 0], [0, 1]]
string: name = "Rhodesia"
```

### Type System

Rhodesia has five primitive types:

| Type | Description | Example |
|------|-------------|---------|
| `int` | 64-bit integer | `42`, `-15` |
| `float64` | Double-precision float | `3.14159`, `-2.5` |
| `vec` | Dynamic column vector | `[1.0, 2.0, 3.0]` |
| `mat` | Dynamic matrix | `[[1, 2], [3, 4]]` |
| `string` | Text string | `"Hello World"` |

### Variable Rules

- Must begin with letter or underscore (`_`)
- Can contain letters, digits, and underscores
- Case-sensitive
- Must be initialized explicitly

## Literals

### Numeric Literals

```rhodesia
int: a = 42
int: b = -15
float64: pi = 3.14159
float64: e = 2.718
float64: avogadro = 6.022e23  // Scientific notation
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

### Boolean and String Literals

```rhodesia
int: t = true   // 1
int: f = false  // 0
string: greeting = "Hello World"
string: multiline = "Line 1\nLine 2"
```

## Operators

### Arithmetic Operators

| Operator | Description | Example |
|----------|-------------|---------|
| `+` | Addition | `a + b` |
| `-` | Subtraction | `a - b` |
| `*` | Multiplication | `a * b` |
| `/` | Division | `a / b` |
| `%` | Modulo | `a % b` |

### Comparison Operators

| Operator | Description | Example |
|----------|-------------|---------|
| `==` | Equal | `a == b` |
| `!=` | Not equal | `a != b` |
| `<` | Less than | `a < b` |
| `<=` | Less or equal | `a <= b` |
| `>` | Greater than | `a > b` |
| `>=` | Greater or equal | `a >= b` |

### Logical Operators

| Operator | Description | Example |
|----------|-------------|---------|
| `and` | Logical AND | `a and b` |
| `or` | Logical OR | `a or b` |
| `not` | Logical NOT | `not a` |

### Operator Precedence

1. Parentheses: `()`
2. Indexing: `[]`
3. Unary: `-`, `not`
4. Multiplicative: `*`, `/`, `%`
5. Additive: `+`, `-`
6. Comparison: `<`, `<=`, `>`, `>=`
7. Equality: `==`, `!=`
8. Logical AND: `and`
9. Logical OR: `or`

## Control Flow

### If-Else Statements

```rhodesia
if condition {
    // code if true
} else if other_condition {
    // code if other condition is true
} else {
    // code if no condition is true
}
```

### For Loops

```rhodesia
// Range iteration
for i in range(5) {
    println(i)  // 0, 1, 2, 3, 4
}

// Vector iteration
vec: data = [10.5, 20.3, 15.7, 8.9]
for value in data {
    println(value)
}
```

### While Loops

```rhodesia
while condition {
    // code that executes while condition is true
}
```

### Break and Continue

```rhodesia
// Break: exit loop immediately
for i in range(100) {
    if i == 5 {
        break  // Exit loop when i == 5
    }
}

// Continue: skip to next iteration
for i in range(10) {
    if i % 2 == 0 {
        continue  // Skip even numbers
    }
    println(i)  // Prints 1, 3, 5, 7, 9
}
```

## Functions

### Function Declaration

```rhodesia
fun function_name(param_type: param_name = default, ...) -> return_type {
    // function body
    return value
}
```

A parameter can be:
- **typed + named**: `int: x`
- **with a default**: `int: x = 5`
- **variadic** (last in the list): `*int: rest` — collects remaining
  positional arguments into a `vec` (or `arr` if the type is non-numeric)

### Function Examples

```rhodesia
// Simple function
fun square(float64: x) -> float64 {
    return x * x
}

// Multiple parameters
fun distance(vec: p1, vec: p2) -> float64 {
    vec: diff = p1 - p2
    return norm(diff)
}

// Default value
fun greet(string: name, string: greeting = "Hello") -> string {
    return string.concat(greeting, ", ", name)
}

// Variadic
fun sumAll(int: first, *int: rest) -> int {
    int: total = first
    for n in rest { total = total + n }
    return total
}

// Void function
fun log(string: name) -> void {
    println("Hello,", name, "!")
}

// Recursive function
fun factorial(int: n) -> int {
    if n <= 1 {
        return 1
    }
    return n * factorial(n - 1)
}
```

### Function Calls

```rhodesia
float64: result = square(5.0)
vec: point1 = [0, 0]
vec: point2 = [3, 4]
float64: dist = distance(point1, point2)
greet("World")

// Positional + keyword mix
print(greet("World", greeting: "Hi"))
print(makeWindow(width: 1024, height: 768))
print(math.sqrt(x: 16))
```

## Code Examples

### Basic Program Structure

```rhodesia
// Import modules (when available)
import math
import vector

// Main program
println("Starting Rhodesia program")

// Variables
int: count = 10
vec: data = range(count)
mat: matrix = eye(3)

// Processing
vec: squared = data * data
float64: total = sum(squared)

// Output
println("Data:", data)
println("Squared:", squared)
println("Sum of squares:", total)
println("Identity matrix:")
println(matrix)
```

### Vector Operations

```rhodesia
vec: u = [1, 2, 3]
vec: v = [4, 5, 6]

// Vector arithmetic
vec: sum = u + v
vec: diff = u - v
vec: scaled = 2 * u

// Vector products
float64: dot_product = dot(u, v)

// Vector functions
float64: norm_u = norm(u)
float64: mean_u = mean(u)
```

### Matrix Operations

```rhodesia
mat: A = [[1, 2], [3, 4]]
mat: B = [[5, 6], [7, 8]]

// Matrix arithmetic
mat: C = A * B  // Matrix multiplication
mat: D = A + B  // Element-wise addition

// Matrix functions
mat: At = transpose(A)
mat: Ainv = inv(A)  // Matrix inverse
```

## Next Steps

- [Control Flow](control-flow.md) - Learn about control structures
- [Type System](types.md) - Understand Rhodesia's type system
- [Examples](examples/basics.md) - See practical examples
