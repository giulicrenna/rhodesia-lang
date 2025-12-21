# Standard Library Functions

This page documents the functions available in Rhodesia's standard library modules. Rhodesia organizes functions into logical modules that must be imported or accessed with module prefixes.

## Table of Contents

- [Mathematical Functions](#mathematical-functions)
- [Vector Functions](#vector-functions)
- [Matrix Functions](#matrix-functions)
- [Statistical Functions](#statistical-functions)
- [Utility Functions](#utility-functions)
- [Input/Output Functions](#inputoutput-functions)

## Mathematical Functions (math module)

All mathematical functions are available through the `math` module.

### Basic Math

| Function | Description | Example |
|----------|-------------|---------|
| `math.sqrt(x)` | Square root | `math.sqrt(16)` → `4.0` |
| `math.exp(x)` | Exponential (e^x) | `math.exp(1)` → `2.718` |
| `math.log(x)` | Natural logarithm | `math.log(2.718)` → `1.0` |
| `math.abs(x)` | Absolute value | `math.abs(-5)` → `5` |

### Trigonometric Functions

| Function | Description | Example |
|----------|-------------|---------|
| `math.sin(x)` | Sine | `math.sin(3.14159/2)` → `1.0` |
| `math.cos(x)` | Cosine | `math.cos(0)` → `1.0` |
| `math.tan(x)` | Tangent | `math.tan(3.14159/4)` → `1.0` |

## Vector Functions (math module)

Vector operations are available through the `math` module.

### Vector Creation

| Function | Description | Example |
|----------|-------------|---------|
| `math.zeros(n)` | Vector of n zeros | `math.zeros(3)` → `[0, 0, 0]` |
| `math.ones(n)` | Vector of n ones | `math.ones(3)` → `[1, 1, 1]` |
| `math.range(n)` | Vector `[0, 1, ..., n-1]` | `math.range(3)` → `[0, 1, 2]` |
| `math.range(start, end)` | Vector `[start, ..., end-1]` | `math.range(2, 5)` → `[2, 3, 4]` |

### Vector Operations

| Function | Description | Example |
|----------|-------------|---------|
| `math.norm(v)` | Euclidean norm | `math.norm([3,4])` → `5.0` |
| `math.sum(v)` | Sum of elements | `math.sum([1,2,3])` → `6.0` |
| `math.size(v)` | Number of elements | `math.size([1,2,3])` → `3` |

### Vector Products

| Function | Description | Example |
|----------|-------------|---------|
| `math.dot(u, v)` | Dot product | `math.dot([1,2],[3,4])` → `11.0` |

## Matrix Functions (math module)

Matrix operations are available through the `math` module.

### Matrix Creation

| Function | Description | Example |
|----------|-------------|---------|
| `math.zeros(m, n)` | m×n matrix of zeros | `math.zeros(2,3)` → `[[0,0,0],[0,0,0]]` |
| `math.ones(m, n)` | m×n matrix of ones | `math.ones(2,3)` → `[[1,1,1],[1,1,1]]` |
| `math.eye(n)` | n×n identity matrix | `math.eye(3)` → `[[1,0,0],[0,1,0],[0,0,1]]` |

### Matrix Operations

| Function | Description | Example |
|----------|-------------|---------|
| `math.transpose(M)` | Matrix transpose | `math.transpose([[1,2],[3,4]])` → `[[1,3],[2,4]]` |
| `math.inv(M)` | Matrix inverse | `math.inv([[1,2],[3,4]])` → `[[-2,1],[1.5,-0.5]]` |
| `math.norm(M)` | Frobenius norm | `math.norm([[1,2],[3,4]])` → `5.477` |
| `math.rows(M)` | Number of rows | `math.rows([[1,2],[3,4]])` → `2` |
| `math.cols(M)` | Number of columns | `math.cols([[1,2],[3,4]])` → `2` |
| `math.size(M)` | Total elements | `math.size([[1,2],[3,4]])` → `4` |

## Statistical Functions (stats module)

Statistical functions are available through the `stats` module.

### Basic Statistics

| Function | Description | Example |
|----------|-------------|---------|
| `stats.mean(v)` | Mean of elements | `stats.mean([1,2,3])` → `2.0` |
| `stats.var(v)` | Variance of elements | `stats.var([1,2,3])` → `0.667` |
| `stats.std(v)` | Standard deviation | `stats.std([1,2,3])` → `0.816` |
| `stats.cov(x, y)` | Covariance between vectors | `stats.cov([1,2],[2,4])` → `1.0` |

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

## Input/Output Functions (io module)

Input/output operations are available through the `io` module.

### File Operations

| Function | Description | Example |
|----------|-------------|---------|
| `io.open(filename, mode)` | Open file, returns handle | `io.open("data.txt", "r")` |
| `io.close(handle)` | Close file handle | `io.close(file_handle)` |
| `io.read(handle)` | Read entire file | `io.read(file_handle)` |
| `io.write(handle, content)` | Write string to file | `io.write(file_handle, "text")` |

### File Information

| Function | Description | Example |
|----------|-------------|---------|
| `io.exists(filename)` | Check if file exists | `io.exists("file.txt")` |
| `io.filesize(filename)` | Get file size in bytes | `io.filesize("file.txt")` |
| `io.remove(filename)` | Delete file | `io.remove("file.txt")` |

### User Input

| Function | Description | Example |
|----------|-------------|---------|
| `io.input()` | Read line from stdin | `io.input()` |
| `io.input(prompt)` | Read line with prompt | `io.input("Enter name: ")` |

### Basic Output

| Function | Description | Example |
|----------|-------------|---------|
| `print(...)` | Print without newline | `print("Hello")` |
| `println(...)` | Print with newline | `println("Hello")` |

## Examples

### Complete Statistical Analysis

```rhodesia
// Generate sample data
vec: data = [12.5, 15.2, 18.7, 14.1, 16.8, 19.3, 13.9, 17.5, 15.8, 18.2]

println("Dataset Statistics:")
println("  Sum:     ", math.sum(data))
println("  Mean:    ", stats.mean(data))
println("  Norm:    ", math.norm(data))
println("  Size:    ", math.size(data))
println("  Variance:", stats.var(data))
println("  Std Dev: ", stats.std(data))
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
println(math.transpose(A))

println("Inverse of A:")
println(math.inv(A))
```

## Next Steps

- [Vector/Matrix Methods](vectors-matrices.md) - Learn about vector/matrix operations
- [Built-in Functions](builtins.md) - Explore core built-in functions
- [Examples](examples/basics.md) - See practical usage examples
- [Language Syntax](language/syntax.md) - Understand Rhodesia syntax
