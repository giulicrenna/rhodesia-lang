# Standard Library Functions

This page documents the built-in functions available in Rhodesia's standard library.

## Table of Contents

- [Mathematical Functions](#mathematical-functions)
- [Vector Functions](#vector-functions)
- [Matrix Functions](#matrix-functions)
- [Statistical Functions](#statistical-functions)
- [Utility Functions](#utility-functions)
- [Input/Output Functions](#inputoutput-functions)

## Mathematical Functions

### Basic Math

| Function | Description | Example |
|----------|-------------|---------|
| `sqrt(x)` | Square root | `sqrt(16)` → `4.0` |
| `exp(x)` | Exponential (e^x) | `exp(1)` → `2.718` |
| `log(x)` | Natural logarithm | `log(2.718)` → `1.0` |
| `abs(x)` | Absolute value | `abs(-5)` → `5` |

### Trigonometric Functions

| Function | Description | Example |
|----------|-------------|---------|
| `sin(x)` | Sine | `sin(3.14159/2)` → `1.0` |
| `cos(x)` | Cosine | `cos(0)` → `1.0` |
| `tan(x)` | Tangent | `tan(3.14159/4)` → `1.0` |

## Vector Functions

### Vector Creation

| Function | Description | Example |
|----------|-------------|---------|
| `zeros(n)` | Vector of n zeros | `zeros(3)` → `[0, 0, 0]` |
| `ones(n)` | Vector of n ones | `ones(3)` → `[1, 1, 1]` |
| `range(n)` | Vector `[0, 1, ..., n-1]` | `range(3)` → `[0, 1, 2]` |
| `range(start, end)` | Vector `[start, ..., end-1]` | `range(2, 5)` → `[2, 3, 4]` |

### Vector Operations

| Function | Description | Example |
|----------|-------------|---------|
| `norm(v)` | Euclidean norm | `norm([3,4])` → `5.0` |
| `sum(v)` | Sum of elements | `sum([1,2,3])` → `6.0` |
| `mean(v)` | Mean of elements | `mean([1,2,3])` → `2.0` |
| `size(v)` | Number of elements | `size([1,2,3])` → `3` |

### Vector Products

| Function | Description | Example |
|----------|-------------|---------|
| `dot(u, v)` | Dot product | `dot([1,2],[3,4])` → `11.0` |

## Matrix Functions

### Matrix Creation

| Function | Description | Example |
|----------|-------------|---------|
| `zeros(m, n)` | m×n matrix of zeros | `zeros(2,3)` → `[[0,0,0],[0,0,0]]` |
| `ones(m, n)` | m×n matrix of ones | `ones(2,3)` → `[[1,1,1],[1,1,1]]` |
| `eye(n)` | n×n identity matrix | `eye(3)` → `[[1,0,0],[0,1,0],[0,0,1]]` |

### Matrix Operations

| Function | Description | Example |
|----------|-------------|---------|
| `transpose(M)` | Matrix transpose | `transpose([[1,2],[3,4]])` → `[[1,3],[2,4]]` |
| `inv(M)` | Matrix inverse | `inv([[1,2],[3,4]])` → `[[-2,1],[1.5,-0.5]]` |
| `norm(M)` | Frobenius norm | `norm([[1,2],[3,4]])` → `5.477` |
| `rows(M)` | Number of rows | `rows([[1,2],[3,4]])` → `2` |
| `cols(M)` | Number of columns | `cols([[1,2],[3,4]])` → `2` |
| `size(M)` | Total elements | `size([[1,2],[3,4]])` → `4` |

## Statistical Functions

### Basic Statistics

```rhodesia
// Mean
fun mean(vec: data) -> float64 {
    return sum(data) / size(data)
}

// Variance
fun variance(vec: data) -> float64 {
    float64: m = mean(data)
    vec: diff = data - m
    return mean(diff * diff)
}

// Standard deviation
fun std_dev(vec: data) -> float64 {
    return sqrt(variance(data))
}

// Covariance
fun covariance(vec: x, vec: y) -> float64 {
    float64: mx = mean(x)
    float64: my = mean(y)
    vec: dx = x - mx
    vec: dy = y - my
    return mean(dx * dy)
}
```

### Regression Functions

```rhodesia
// Simple linear regression
fun linear_regression(vec: x, vec: y) -> vec {
    int: n = size(x)

    // Design matrix with intercept
    mat: X = ones(n, 2)
    for i in range(n) {
        X[i, 1] = x[i]
    }

    // OLS: β = (X'X)^(-1) X'y
    mat: Xt = transpose(X)
    mat: XtX = Xt * X
    mat: XtX_inv = inv(XtX)
    vec: Xty = Xt * y

    return XtX_inv * Xty
}

// Model evaluation
fun r_squared(vec: y_true, vec: y_pred) -> float64 {
    float64: y_mean = mean(y_true)
    vec: residuals = y_true - y_pred
    vec: total = y_true - y_mean

    float64: ss_res = dot(residuals, residuals)
    float64: ss_tot = dot(total, total)

    return 1.0 - ss_res / ss_tot
}
```

## Utility Functions

### Data Normalization

```rhodesia
// Min-max normalization
fun min_max_normalize(vec: data) -> vec {
    float64: min_val = 1e100
    float64: max_val = -1e100

    // Find min and max
    for val in data {
        if val < min_val { min_val = val }
        if val > max_val { max_val = val }
    }

    // Normalize
    vec: normalized = zeros(size(data))
    float64: range_val = max_val - min_val

    if range_val == 0 {
        return normalized
    }

    for i in range(size(data)) {
        normalized[i] = (data[i] - min_val) / range_val
    }

    return normalized
}

// Z-score normalization
fun z_score_normalize(vec: data) -> vec {
    float64: mean_val = mean(data)
    vec: diff = data - mean_val
    vec: diff_sq = diff * diff
    float64: variance = mean(diff_sq)
    float64: std_dev = sqrt(variance)

    if std_dev == 0 {
        return zeros(size(data))
    }

    return diff / std_dev
}
```

### Vector Operations

```rhodesia
// Vector normalization
fun normalize(vec: v) -> vec {
    float64: n = norm(v)
    if n == 0 {
        return v  // Avoid division by zero
    }
    return v / n
}

// Vector projection
fun project(vec: u, vec: v) -> vec {
    float64: dot_uv = dot(u, v)
    float64: dot_vv = dot(v, v)
    return (dot_uv / dot_vv) * v
}
```

## Input/Output Functions

### Basic I/O

| Function | Description | Example |
|----------|-------------|---------|
| `print(...)` | Print without newline | `print("Hello")` |
| `println(...)` | Print with newline | `println("Hello")` |

### Formatted Output

```rhodesia
// Basic usage
print("Value: ", 42)          // "Value: 42"
println("Result: ", 3.14)     // "Result: 3.14"

// Multiple values
println("x =", 10, "y =", 20) // "x = 10 y = 20"

// Vector and matrix output
vec: v = [1, 2, 3]
println("Vector:", v)         // "Vector: [1, 2, 3]"

mat: m = [[1, 2], [3, 4]]
println("Matrix:")
println(m)                   // Prints matrix with formatting
```

## Examples

### Complete Statistical Analysis

```rhodesia
// Generate sample data
vec: data = [12.5, 15.2, 18.7, 14.1, 16.8, 19.3, 13.9, 17.5, 15.8, 18.2]

println("Dataset Statistics:")
println("  Sum:     ", sum(data))
println("  Mean:    ", mean(data))
println("  Norm:    ", norm(data))
println("  Size:    ", size(data))
println("  Variance:", variance(data))
println("  Std Dev: ", std_dev(data))
println("")

// Normalization
println("Normalization:")
println("  Min-Max: ", min_max_normalize(data))
println("  Z-score: ", z_score_normalize(data))
```

### Linear Algebra Example

```rhodesia
// Matrix operations
mat: A = [[1.0, 2.0], [3.0, 4.0]]
mat: B = [[5.0, 6.0], [7.0, 8.0]]

println("Matrix A:")
println(A)

println("Matrix B:")
println(B)

println("A + B:")
println(A + B)

println("A * B:")
println(A * B)

println("Transpose of A:")
println(transpose(A))

println("Inverse of A:")
println(inv(A))
```

## Next Steps

- [Vector/Matrix Methods](vectors-matrices.md) - Learn about vector/matrix operations
- [Built-in Functions](builtins.md) - Explore core built-in functions
- [Examples](examples/basics.md) - See practical usage examples
- [Language Syntax](language/syntax.md) - Understand Rhodesia syntax
