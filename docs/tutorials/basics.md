---
title: Basics
layout: default
nav_order: 1
parent: Tutorials
---

# Rhodesia Basics Tutorial

Welcome to the Rhodesia basics tutorial! This guide will walk you through the fundamental concepts of the Rhodesia programming language.

## What is Rhodesia?

Rhodesia is a statically-typed programming language designed for data science and numerical computing. It combines clean syntax with high-performance mathematical operations powered by the Eigen library.

## Setting Up

First, make sure you have Rhodesia installed. Follow the [Getting Started](getting-started.md) guide to build the language from source.

## Your First Program

Let's start with a simple "Hello World" program:

```rhodesia
// hello.rho
println("Hello, Rhodesia!")
```

Save this as `hello.rho` and run it:

```bash
./rhodesia hello.rho
```

## Variables and Types

Rhodesia has five primitive types:

### Basic Variables

```rhodesia
// Integer
int: age = 25

// Floating point
float64: pi = 3.14159

// String
string: name = "Rhodesia"

// Boolean (represented as int)
int: is_active = true  // 1
int: is_inactive = false  // 0
```

### Vectors and Matrices

```rhodesia
// Vector (column vector)
vec: coordinates = [10.5, 20.3, 15.7]

// Matrix
mat: transformation = [[1, 0, 0],
                       [0, 1, 0],
                       [0, 0, 1]]
```

## Basic Operations

### Arithmetic

```rhodesia
int: a = 10
int: b = 3

int: sum = a + b        // 13
int: difference = a - b // 7
int: product = a * b    // 30
float64: quotient = a / b  // 3.333...
int: remainder = a % b  // 1
```

### Vector Operations

```rhodesia
vec: u = [1, 2, 3]
vec: v = [4, 5, 6]

vec: sum_vec = u + v      // [5, 7, 9]
vec: scaled = 2 * u       // [2, 4, 6]
float64: dot_prod = dot(u, v)  // 32.0
```

### Matrix Operations

```rhodesia
mat: A = [[1, 2], [3, 4]]
mat: B = [[5, 6], [7, 8]]

mat: C = A * B  // Matrix multiplication
mat: D = A + B  // Element-wise addition
mat: At = transpose(A)  // Transpose
```

## Control Flow

### If Statements

```rhodesia
int: x = 15

if x > 20 {
    println("x is greater than 20")
} else if x > 10 {
    println("x is between 11 and 20")
} else {
    println("x is 10 or less")
}
```

### For Loops

```rhodesia
// Iterate over a range
for i in range(5) {
    println(i)  // 0, 1, 2, 3, 4
}

// Iterate over a vector
vec: data = [10, 20, 30]
for value in data {
    println(value)
}
```

### While Loops

```rhodesia
int: count = 0
while count < 5 {
    println("Count:", count)
    count = count + 1
}
```

## Functions

### Defining Functions

```rhodesia
// Simple function
fun square(float64: x) -> float64 {
    return x * x
}

// Function with multiple parameters
fun add(float64: a, float64: b) -> float64 {
    return a + b
}

// Void function (no return value)
fun greet(string: name) -> void {
    println("Hello,", name, "!")
}
```

### Calling Functions

```rhodesia
float64: result = square(5.0)  // 25.0
float64: sum = add(3.0, 4.0)   // 7.0
greet("World")                  // Prints: Hello, World!
```

## Working with Data

### Creating Data

```rhodesia
// Create a sequence
vec: numbers = range(10)  // [0, 1, 2, ..., 9]

// Create matrices
mat: zeros_mat = zeros(3, 3)  // 3x3 matrix of zeros
mat: ones_mat = ones(2, 4)    // 2x4 matrix of ones
mat: identity = eye(3)        // 3x3 identity matrix
```

### Data Analysis

```rhodesia
vec: data = [12, 15, 18, 22, 25, 30]

println("Data:", data)
println("Size:", size(data))
println("Sum:", sum(data))
println("Mean:", mean(data))
println("Norm:", norm(data))
```

## Putting It Together

Here's a complete example that demonstrates many concepts:

```rhodesia
// statistics_demo.rho
// Calculate basic statistics for a dataset

fun calculate_stats(vec: data) -> void {
    if size(data) == 0 {
        println("Empty dataset!")
        return
    }

    float64: total = sum(data)
    float64: avg = mean(data)
    float64: data_norm = norm(data)

    println("=== Dataset Statistics ===")
    println("Data:", data)
    println("Count:", size(data))
    println("Sum:", total)
    println("Mean:", avg)
    println("Norm:", data_norm)

    // Find min and max
    float64: min_val = 1e100
    float64: max_val = -1e100

    for val in data {
        if val < min_val { min_val = val }
        if val > max_val { max_val = val }
    }

    println("Min:", min_val)
    println("Max:", max_val)
    println("Range:", max_val - min_val)
}

// Test the function
vec: sample_data = [23.5, 18.2, 31.7, 25.1, 29.8, 22.4]
calculate_stats(sample_data)
```

## Next Steps

- [Data Science Tutorial](data-science.md) - Statistical analysis and visualization
- [Algorithms Tutorial](algorithms.md) - Implementing numerical algorithms
- [Examples](examples/basics.md) - More practical examples

## Practice Exercises

1. **Basic Calculator**: Create functions for basic arithmetic operations
2. **Vector Statistics**: Extend the statistics function to calculate variance and standard deviation
3. **Matrix Operations**: Implement matrix-vector multiplication manually
4. **Data Filtering**: Write a function that filters a vector based on a condition

Remember: Practice is key to learning! Try modifying the examples and experimenting with different operations.
