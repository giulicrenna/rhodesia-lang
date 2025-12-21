# Data Structures Examples

This page demonstrates vector and matrix operations in Rhodesia with practical examples.

## Table of Contents

- [Vector Operations](#vector-operations)
- [Matrix Operations](#matrix-operations)
- [Linear Algebra](#linear-algebra)
- [Statistical Operations](#statistical-operations)
- [Advanced Examples](#advanced-examples)

## Vector Operations

### Basic Vector Arithmetic

```rhodesia
// Vector creation and arithmetic
vec: a = [1.0, 2.0, 3.0, 4.0, 5.0]
vec: b = [5.0, 4.0, 3.0, 2.0, 1.0]

println("Vector a:", a)
println("Vector b:", b)
println("")

// Basic operations
println("a + b:", a + b)        // [6, 6, 6, 6, 6]
println("a - b:", a - b)        // [-4, -2, 0, 2, 4]
println("a * 2:", a * 2.0)      // [2, 4, 6, 8, 10]
println("a / 2:", a / 2.0)      // [0.5, 1, 1.5, 2, 2.5]
```

### Vector Products

```rhodesia
vec: u = [1.0, 2.0, 3.0]
vec: v = [4.0, 5.0, 6.0]

// Dot product
float64: dot_product = dot(u, v)
println("Dot product of u and v:", dot_product)  // 32.0

// Element-wise multiplication
vec: elementwise = u * v
println("Element-wise product:", elementwise)  // [4, 10, 18]
```

### Vector Functions

```rhodesia
vec: data = [1.0, 2.0, 3.0, 4.0, 5.0]

println("Norm:", norm(data))          // 7.416 (Euclidean norm)
println("Sum:", sum(data))            // 15.0
println("Mean:", mean(data))          // 3.0
println("Size:", size(data))          // 5

// Mathematical functions
println("Square root:", sqrt(data))   // [1, 1.414, 1.732, 2, 2.236]
println("Exponential:", exp(data/2))  // [1.648, 2.718, 4.481, 7.389, 12.182]
println("Logarithm:", log(data))      // [0, 0.693, 1.098, 1.386, 1.609]
```

### Vector Creation

```rhodesia
// Different ways to create vectors
vec: zeros_vec = zeros(5)        // [0, 0, 0, 0, 0]
vec: ones_vec = ones(5)         // [1, 1, 1, 1, 1]
vec: range_vec = range(5)       // [0, 1, 2, 3, 4]
vec: custom_range = range(2, 8) // [2, 3, 4, 5, 6, 7]

println("Zeros:", zeros_vec)
println("Ones:", ones_vec)
println("Range:", range_vec)
println("Custom range:", custom_range)
```

## Matrix Operations

### Basic Matrix Arithmetic

```rhodesia
mat: A = [[1.0, 2.0], [3.0, 4.0]]
mat: B = [[5.0, 6.0], [7.0, 8.0]]

println("Matrix A:")
println(A)
println("Matrix B:")
println(B)
println("")

// Matrix operations
println("A + B:")
println(A + B)  // [[6, 8], [10, 12]]

println("A - B:")
println(A - B)  // [[-4, -4], [-4, -4]]

println("A * 2:")
println(A * 2.0)  // [[2, 4], [6, 8]]
```

### Matrix Multiplication

```rhodesia
mat: X = [[1.0, 2.0, 3.0],
          [4.0, 5.0, 6.0]]
mat: Y = [[7.0, 8.0],
          [9.0, 10.0],
          [11.0, 12.0]]

// Matrix multiplication (X is 2x3, Y is 3x2, result is 2x2)
mat: Z = X * Y
println("Matrix multiplication result:")
println(Z)
```

### Matrix Functions

```rhodesia
mat: M = [[1.0, 2.0], [3.0, 4.0]]

println("Original matrix:")
println(M)

println("Transpose:")
println(transpose(M))  // [[1, 3], [2, 4]]

println("Inverse:")
println(inv(M))        // [[-2, 1], [1.5, -0.5]]

println("Norm (Frobenius):", norm(M))  // 5.477
```

### Matrix Creation

```rhodesia
// Different ways to create matrices
mat: zeros_mat = zeros(2, 3)    // 2x3 matrix of zeros
mat: ones_mat = ones(3, 2)      // 3x2 matrix of ones
mat: identity = eye(3)          // 3x3 identity matrix

println("Zeros matrix:")
println(zeros_mat)

println("Ones matrix:")
println(ones_mat)

println("Identity matrix:")
println(identity)
```

## Linear Algebra

### Solving Linear Systems

```rhodesia
// System: 2x + 3y = 8
//         4x + 5y = 14

mat: A = [[2.0, 3.0], [4.0, 5.0]]
vec: b = [8.0, 14.0]

// Solve Ax = b
vec: solution = inv(A) * b
println("Solution to linear system:")
println("x =", solution[0])  // 1.0
println("y =", solution[1])  // 2.0
```

### Matrix Decomposition Example

```rhodesia
// Simple matrix decomposition demonstration
mat: matrix = [[4.0, 7.0], [2.0, 6.0]]

println("Original matrix:")
println(matrix)

mat: inverse = inv(matrix)
println("Inverse matrix:")
println(inverse)

mat: identity_check = matrix * inverse
println("Matrix * Inverse (should be identity):")
println(identity_check)
```

## Statistical Operations

### Basic Statistics

```rhodesia
vec: dataset = [12.5, 15.2, 18.7, 14.1, 16.8, 19.3, 13.9, 17.5, 15.8, 18.2]

println("Dataset:", dataset)
println("Sum:", sum(dataset))            // 162.0
println("Mean:", mean(dataset))          // 16.2
println("Norm:", norm(dataset))          // 56.72
println("Size:", size(dataset))          // 10
```

### Vector Normalization

```rhodesia
fun normalize(vec: v) -> vec {
    float64: n = norm(v)
    if n == 0 {
        return v  // Avoid division by zero
    }
    return v / n
}

vec: vector = [3.0, 4.0, 5.0]
vec: normalized = normalize(vector)

println("Original vector:", vector)
println("Normalized vector:", normalized)
println("Norm of normalized:", norm(normalized))  // Should be 1.0
```

### Covariance and Correlation

```rhodesia
fun covariance(vec: x, vec: y) -> float64 {
    float64: mx = mean(x)
    float64: my = mean(y)
    vec: dx = x - mx
    vec: dy = y - my
    return mean(dx * dy)
}

vec: x = [1.0, 2.0, 3.0, 4.0, 5.0]
vec: y = [2.0, 4.0, 6.0, 8.0, 10.0]

float64: cov = covariance(x, y)
println("Covariance between x and y:", cov)
```

## Advanced Examples

### Matrix Transformations

```rhodesia
// Rotation matrix
fun rotation_matrix(float64: angle) -> mat {
    float64: c = cos(angle)
    float64: s = sin(angle)
    return [[c, -s], [s, c]]
}

// Scaling matrix
fun scaling_matrix(float64: scale_x, float64: scale_y) -> mat {
    return [[scale_x, 0.0], [0.0, scale_y]]
}

// Apply transformations
vec: point = [1.0, 0.0]
float64: angle = 3.14159 / 4  // 45 degrees

mat: rotation = rotation_matrix(angle)
mat: scaled = scaling_matrix(2.0, 2.0)

vec: rotated = rotation * point
vec: transformed = scaled * rotated

println("Original point:", point)
println("After rotation:", rotated)
println("After scaling:", transformed)
```

### Vector Projection

```rhodesia
fun project(vec: u, vec: v) -> vec {
    // Projection of u onto v
    float64: dot_uv = dot(u, v)
    float64: dot_vv = dot(v, v)
    return (dot_uv / dot_vv) * v
}

vec: vector1 = [2.0, 3.0]
vec: vector2 = [1.0, 1.0]

vec: projection = project(vector1, vector2)
println("Projection of", vector1, "onto", vector2, ":", projection)
```

### Orthogonalization

```rhodesia
fun gram_schmidt(mat: basis) -> mat {
    int: n = rows(basis)
    int: m = cols(basis)
    mat: result = zeros(n, m)

    for j in range(m) {
        vec: v = zeros(n)
        for i in range(n) {
            v[i] = basis[i, j]
        }

        for k in range(j) {
            vec: u = zeros(n)
            for i in range(n) {
                u[i] = result[i, k]
            }
            vec: proj = project(v, u)
            v = v - proj
        }

        float64: norm_v = norm(v)
        if norm_v > 0 {
            v = v / norm_v
        }

        for i in range(n) {
            result[i, j] = v[i]
        }
    }

    return result
}

// Example usage
mat: basis = [[1.0, 1.0, 1.0],
              [1.0, 0.0, 2.0],
              [0.0, 1.0, 1.0]]

mat: orthogonal = gram_schmidt(basis)
println("Orthogonal basis:")
println(orthogonal)
```

## Next Steps

- [Machine Learning Examples](machine-learning.md) - Explore ML algorithms
- [Basic Examples](basics.md) - Review basic Rhodesia concepts
- [Standard Library](standard-library/vectors-matrices.md) - Learn about vector/matrix methods
- [Performance Guide](api/performance.md) - Optimize your code
