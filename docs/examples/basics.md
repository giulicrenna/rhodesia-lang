---
title: Basics
layout: default
nav_order: 1
parent: Examples
---

# Basic Rhodesia Examples

This page contains basic examples to help you get started with Rhodesia programming.

## Table of Contents

- [Hello World](#hello-world)
- [Variables and Types](#variables-and-types)
- [Basic Operations](#basic-operations)
- [Control Flow](#control-flow)
- [Functions](#functions)
- [Complete Example](#complete-example)

## Hello World

The classic "Hello World" program in Rhodesia:

```rhodesia
// hello.rho
println("Hello, Rhodesia!")
println("Welcome to the world of data science programming!")
```

**Output:**
```
Hello, Rhodesia!
Welcome to the world of data science programming!
```

## Variables and Types

### Basic Variables

```rhodesia
// Variable declarations
int: age = 25
float64: height = 1.75
string: name = "Alice"
bool: is_student = true

println("Name:", name)
println("Age:", age)
println("Height:", height, "meters")
println("Student:", is_student)
```

### Vector Variables

```rhodesia
// Vector operations
vec: coordinates = [10.5, 20.3, 15.7]
vec: temperatures = [22.1, 23.5, 21.8, 24.2]

println("Coordinates:", coordinates)
println("First temperature:", temperatures[0])
println("Number of temperatures:", math.size(temperatures))
```

### Matrix Variables

```rhodesia
// Matrix operations
mat: identity = [[1, 0], [0, 1]]
mat: transform = [[2, 0], [0, 2]]

println("Identity matrix:")
println(identity)
println("Transform matrix:")
println(transform)
```

## Basic Operations

### Arithmetic Operations

```rhodesia
int: a = 10
int: b = 3

println("Addition:", a + b)        // 13
println("Subtraction:", a - b)     // 7
println("Multiplication:", a * b)  // 30
println("Division:", a / b)        // 3.333...
println("Modulo:", a % b)          // 1
```

### Vector Operations

```rhodesia
vec: u = [1, 2, 3]
vec: v = [4, 5, 6]

println("Vector sum:", u + v)     // [5, 7, 9]
println("Vector difference:", u - v)  // [-3, -3, -3]
println("Scalar multiplication:", 2 * u)  // [2, 4, 6]
println("Dot product:", math.dot(u, v))  // 32
```

### Matrix Operations

```rhodesia
mat: A = [[1, 2], [3, 4]]
mat: B = [[5, 6], [7, 8]]

println("Matrix sum:")
println(A + B)

println("Matrix multiplication:")
println(A * B)

println("Transpose of A:")
println(math.transpose(A))
```

## Control Flow

### If-Else Statements

```rhodesia
int: temperature = 22

if temperature > 30 {
    println("It's hot!")
} else if temperature > 20 {
    println("It's warm.")
} else {
    println("It's cool.")
}

// Output: It's warm.
```

### For Loops

```rhodesia
// Counting loop
println("Counting to 5:")
for i in range(6) {
    println(i)
}

// Vector iteration
vec: fruits = ["apple", "banana", "cherry"]
println("Fruits:")
for fruit in fruits {
    println("  ", fruit)
}
```

### While Loops

```rhodesia
int: countdown = 5
println("Countdown:")
while countdown > 0 {
    println(countdown)
    countdown = countdown - 1
}
println("Blast off!")
```

## Functions

### Simple Function

```rhodesia
fun square(float64: x) -> float64 {
    return x * x
}

// Usage
float64: result = square(5.0)
println("Square of 5:", result)  // 25
```

### Multiple Parameters

```rhodesia
fun add_and_multiply(float64: a, float64: b, float64: c) -> float64 {
    return (a + b) * c
}

// Usage
float64: calc = add_and_multiply(2.0, 3.0, 4.0)
println("Result:", calc)  // (2 + 3) * 4 = 20
```

### Void Function

```rhodesia
fun greet(string: name) -> void {
    println("Hello,", name, "!")
}

// Usage
greet("World")  // Hello, World!
```

## Complete Example

Here's a complete example that demonstrates several Rhodesia features:

```rhodesia
// complete_example.rho
println("=== Rhodesia Complete Example ===")
println("")

// Variables
int: data_points = 10
vec: measurements = [12.5, 15.2, 18.7, 14.1, 16.8,
                     19.3, 13.9, 17.5, 15.8, 18.2]

println("Data Analysis:")
println("Number of measurements:", data_points)
println("Measurements:", measurements)
println("")

// Statistics
float64: total = math.sum(measurements)
float64: average = stats.mean(measurements)
float64: min_val = math.min(measurements)  // min/max functions
float64: max_val = math.max(measurements)  // min/max functions

println("Statistics:")
println("  Total:", total)
println("  Average:", average)
println("  Minimum:", min_val)
println("  Maximum:", max_val)
println("  Range:", max_val - min_val)
println("")

// Data processing
vec: normalized = measurements - average
vec: squared_diff = normalized * normalized
float64: variance = stats.mean(squared_diff)
float64: std_dev = math.sqrt(variance)

println("Variance Analysis:")
println("  Variance:", variance)
println("  Standard Deviation:", std_dev)
println("")

// Function example
fun classify_value(float64: value, float64: mean, float64: std) -> string {
    float64: z_score = (value - mean) / std
    if z_score > 1.0 {
        return "High"
    } else if z_score < -1.0 {
        return "Low"
    } else {
        return "Normal"
    }
}

// Classify each measurement
println("Classification:")
for i in range(data_points) {
    string: category = classify_value(measurements[i], average, std_dev)
    println("  Measurement", i, ":", measurements[i], "->", category)
}
println("")

println("Analysis complete!")
```

## Next Steps

- [Data Structures Examples](data-structures.md) - Learn about vector and matrix operations
- [Machine Learning Examples](machine-learning.md) - Explore ML algorithms
- [Language Syntax](language/syntax.md) - Understand Rhodesia syntax in depth
- [Standard Library](standard-library/functions.md) - Discover built-in functions
