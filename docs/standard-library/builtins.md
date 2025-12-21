# Built-in Functions

Rhodesia provides several built-in functions that are available without importing any modules. These core functions handle basic operations, I/O, and type information.

## Core Functions

### print(values...) → void

Prints values to stdout without a newline.

**Parameters:**
- `values...`: Any number of values to print

**Examples:**
```rhodesia
print("Hello")
print("x =", 42)
print("vector:", [1, 2, 3])
```

### println(values...) → void

Prints values to stdout with a newline.

**Parameters:**
- `values...`: Any number of values to print

**Examples:**
```rhodesia
println("Hello World")
println("x =", 42, "y =", 3.14)
println("Result:", [1, 2, 3] + [4, 5, 6])
```

## Type Information Functions

### size(vec|mat|string) → int

Returns the size of a vector, matrix, or string.

**Parameters:**
- `vec|mat|string`: The data structure

**Returns:**
- Number of elements (int)

**Examples:**
```rhodesia
vec: v = [1, 2, 3]
int: len = size(v)  // 3

mat: m = [[1, 2], [3, 4]]
int: total = size(m)  // 4

string: s = "hello"
int: chars = size(s)  // 5
```

### rows(mat) → int

Returns the number of rows in a matrix.

**Parameters:**
- `mat`: Matrix

**Returns:**
- Number of rows (int)

**Examples:**
```rhodesia
mat: m = [[1, 2, 3], [4, 5, 6]]
int: r = rows(m)  // 2
```

### cols(mat) → int

Returns the number of columns in a matrix.

**Parameters:**
- `mat`: Matrix

**Returns:**
- Number of columns (int)

**Examples:**
```rhodesia
mat: m = [[1, 2, 3], [4, 5, 6]]
int: c = cols(m)  // 3
```

## Mathematical Built-ins

### sqrt(float64) → float64

Returns the square root of a number.

**Parameters:**
- `float64`: Input value

**Returns:**
- Square root (float64)

**Examples:**
```rhodesia
float64: s = sqrt(16.0)  // 4.0
```

### abs(int|float64) → int|float64

Returns the absolute value.

**Parameters:**
- `int|float64`: Input value

**Returns:**
- Absolute value (same type)

**Examples:**
```rhodesia
int: ai = abs(-5)  // 5
float64: af = abs(-3.14)  // 3.14
```

## Vector/Matrix Creation

### zeros(int) → vec
### zeros(int, int) → mat

Creates a vector or matrix of zeros.

**Parameters:**
- `int`: Size (vector) or rows (matrix)
- `int`: Columns (matrix only)

**Returns:**
- Vector or matrix of zeros

**Examples:**
```rhodesia
vec: v = zeros(3)  // [0, 0, 0]
mat: m = zeros(2, 3)  // [[0, 0, 0], [0, 0, 0]]
```

### ones(int) → vec
### ones(int, int) → mat

Creates a vector or matrix of ones.

**Parameters:**
- `int`: Size (vector) or rows (matrix)
- `int`: Columns (matrix only)

**Returns:**
- Vector or matrix of ones

**Examples:**
```rhodesia
vec: v = ones(3)  // [1, 1, 1]
mat: m = ones(2, 3)  // [[1, 1, 1], [1, 1, 1]]
```

### eye(int) → mat

Creates an identity matrix.

**Parameters:**
- `int`: Size of the square matrix

**Returns:**
- Identity matrix

**Examples:**
```rhodesia
mat: i = eye(3)  // [[1, 0, 0], [0, 1, 0], [0, 0, 1]]
```

### range(int) → vec
### range(int, int) → vec

Creates a vector with a sequence of integers.

**Parameters:**
- `int`: End value (exclusive), start from 0
- `int, int`: Start (inclusive) and end (exclusive)

**Returns:**
- Vector of integers

**Examples:**
```rhodesia
vec: seq1 = range(5)  // [0, 1, 2, 3, 4]
vec: seq2 = range(2, 8)  // [2, 3, 4, 5, 6, 7]
```

## Linear Algebra Built-ins

### norm(vec|mat) → float64

Computes the norm of a vector or matrix.

**Parameters:**
- `vec|mat`: Input data

**Returns:**
- L2 norm for vectors, Frobenius norm for matrices

**Examples:**
```rhodesia
vec: v = [3, 4]
float64: n = norm(v)  // 5.0

mat: m = [[1, 2], [3, 4]]
float64: f = norm(m)  // ≈ 5.477
```

### dot(vec, vec) → float64

Computes the dot product of two vectors.

**Parameters:**
- `vec, vec`: Input vectors

**Returns:**
- Dot product (float64)

**Examples:**
```rhodesia
vec: u = [1, 2, 3]
vec: v = [4, 5, 6]
float64: dp = dot(u, v)  // 32.0
```

### sum(vec|mat) → float64

Sums all elements.

**Parameters:**
- `vec|mat`: Input data

**Returns:**
- Sum of all elements (float64)

**Examples:**
```rhodesia
vec: v = [1, 2, 3]
float64: total = sum(v)  // 6.0
```

### mean(vec|mat) → float64

Computes the arithmetic mean.

**Parameters:**
- `vec|mat`: Input data

**Returns:**
- Mean of all elements (float64)

**Examples:**
```rhodesia
vec: data = [1, 2, 3, 4, 5]
float64: avg = mean(data)  // 3.0
```

### transpose(vec|mat) → mat

Computes the transpose.

**Parameters:**
- `vec|mat`: Input data

**Returns:**
- Transposed matrix

**Examples:**
```rhodesia
vec: v = [1, 2, 3]
mat: vt = transpose(v)  // [[1, 2, 3]]

mat: m = [[1, 2], [3, 4]]
mat: mt = transpose(m)  // [[1, 3], [2, 4]]
```

### inv(mat) → mat

Computes the matrix inverse.

**Parameters:**
- `mat`: Square matrix

**Returns:**
- Inverse matrix

**Errors:**
- RuntimeError if matrix is not square or singular

**Examples:**
```rhodesia
mat: m = [[1, 2], [3, 4]]
mat: mi = inv(m)
mat: identity = m * mi  // ≈ [[1, 0], [0, 1]]
```

## Next Steps

- [Functions](functions.md) - Module-based mathematical functions
- [Vectors & Matrices](vectors-matrices.md) - Vector and matrix operations
- [Examples](examples/basics.md) - Practical usage examples
