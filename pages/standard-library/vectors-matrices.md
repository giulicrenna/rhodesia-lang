# Vector and Matrix Methods

This page documents the vector and matrix operations available in Rhodesia.

## Table of Contents

- [Vector Methods](#vector-methods)
- [Matrix Methods](#matrix-methods)
- [Indexed Assignment](#indexed-assignment)
- [Examples](#examples)

## Vector Methods

All vector methods are **immutable** - they return new vectors without modifying the original.

### `vector.append(vec, value) -> vec`

Appends a scalar value to the end of a vector.

```rhodesia
vec: v = [1.0, 2.0, 3.0]
vec: v2 = vector.append(v, 4.0)
println(v2)  // [1, 2, 3, 4]
println(v)   // [1, 2, 3] - unchanged
```

### `vector.remove(vec, index) -> vec`

Removes an element at the specified index from a vector.

```rhodesia
vec: v = [1.0, 2.0, 3.0, 4.0, 5.0]
vec: v2 = vector.remove(v, 2)
println(v2)  // [1, 2, 4, 5]
println(v)   // [1, 2, 3, 4, 5] - unchanged
```

### `vector.reverse(vec) -> vec`

Reverses the order of elements in a vector.

```rhodesia
vec: v = [1.0, 2.0, 3.0, 4.0, 5.0]
vec: v2 = vector.reverse(v)
println(v2)  // [5, 4, 3, 2, 1]
println(v)   // [1, 2, 3, 4, 5] - unchanged
```

### `vector.insert(vec, index, value) -> vec`

Inserts a scalar value at the specified index in a vector.

```rhodesia
vec: v = [1.0, 2.0, 3.0]
vec: v2 = vector.insert(v, 1, 99.0)
println(v2)  // [1, 99, 2, 3]
println(v)   // [1, 2, 3] - unchanged
```

### `vector.size(vec) -> int`

Returns the number of elements in a vector.

```rhodesia
vec: v = [1.0, 2.0, 3.0, 4.0, 5.0]
println(vector.size(v))  // 5
```

## Matrix Methods

All matrix methods are **immutable** - they return new matrices without modifying the original.

### `matrix.append_row(mat, row_vec) -> mat`

Appends a row (vector) to the bottom of a matrix.

```rhodesia
mat: m = [[1.0, 2.0, 3.0],
          [4.0, 5.0, 6.0]]
vec: new_row = [7.0, 8.0, 9.0]
mat: m2 = matrix.append_row(m, new_row)
println(m2)  // 3x3 matrix with new row
println(m)   // 2x3 matrix - unchanged
```

### `matrix.append_col(mat, col_vec) -> mat`

Appends a column (vector) to the right side of a matrix.

```rhodesia
mat: m = [[1.0, 2.0],
          [3.0, 4.0],
          [5.0, 6.0]]
vec: new_col = [10.0, 20.0, 30.0]
mat: m2 = matrix.append_col(m, new_col)
println(m2)  // 3x3 matrix with new column
println(m)   // 3x2 matrix - unchanged
```

### `matrix.remove_row(mat, index) -> mat`

Removes a row at the specified index from a matrix.

```rhodesia
mat: m = [[1.0, 2.0, 3.0],
          [4.0, 5.0, 6.0],
          [7.0, 8.0, 9.0]]
mat: m2 = matrix.remove_row(m, 1)
println(m2)  // 2x3 matrix without row 1
println(m)   // 3x3 matrix - unchanged
```

### `matrix.remove_col(mat, index) -> mat`

Removes a column at the specified index from a matrix.

```rhodesia
mat: m = [[1.0, 2.0, 3.0],
          [4.0, 5.0, 6.0],
          [7.0, 8.0, 9.0]]
mat: m2 = matrix.remove_col(m, 1)
println(m2)  // 3x2 matrix without column 1
println(m)   // 3x3 matrix - unchanged
```

### `matrix.rows(mat) -> int`

Returns the number of rows in a matrix.

```rhodesia
mat: m = [[1.0, 2.0],
          [3.0, 4.0],
          [5.0, 6.0]]
println(matrix.rows(m))  // 3
```

### `matrix.cols(mat) -> int`

Returns the number of columns in a matrix.

```rhodesia
mat: m = [[1.0, 2.0],
          [3.0, 4.0],
          [5.0, 6.0]]
println(matrix.cols(m))  // 2
```

### `matrix.size(mat) -> int`

Returns the total number of elements in a matrix (rows × columns).

```rhodesia
mat: m = [[1.0, 2.0],
          [3.0, 4.0],
          [5.0, 6.0]]
println(matrix.size(m))  // 6
```

## Indexed Assignment

Both vectors and matrices support indexed reassignment, allowing you to modify individual elements.

### Vector Indexed Reassignment

```rhodesia
vec: v = [1.0, 2.0, 3.0, 4.0, 5.0]
v[0] = 100.0    // Modify first element
v[2] = 300.0    // Modify third element
println(v)      // [100, 2, 300, 4, 5]
```

### Matrix Indexed Reassignment

```rhodesia
mat: m = [[1.0, 2.0, 3.0],
          [4.0, 5.0, 6.0],
          [7.0, 8.0, 9.0]]
m[0, 0] = 999.0    // Modify element at row 0, column 0
m[1, 1] = 888.0    // Modify element at row 1, column 1
m[2, 2] = 777.0    // Modify element at row 2, column 2
println(m)         // Matrix with modified diagonal elements
```

## Examples

### Combining Vector Operations

```rhodesia
vec: v = [10.0, 20.0, 30.0]
vec: v2 = vector.append(v, 40.0)         // [10, 20, 30, 40]
vec: v3 = vector.reverse(v2)              // [40, 30, 20, 10]
vec: v4 = vector.remove(v3, 1)            // [40, 20, 10]
println(v4)
```

### Combining Matrix Operations

```rhodesia
mat: m = [[1.0, 2.0],
          [3.0, 4.0]]

vec: new_row = [5.0, 6.0]
mat: m2 = matrix.append_row(m, new_row)   // 3x2 matrix

vec: new_col = [10.0, 20.0, 30.0]
mat: m3 = matrix.append_col(m2, new_col)  // 3x3 matrix

mat: m4 = matrix.remove_row(m3, 0)        // 2x3 matrix
println(m4)
```

### Chaining Operations

```rhodesia
vec: result = vector.reverse(
    vector.remove(
        vector.append([1.0, 2.0, 3.0], 4.0),
        1
    )
)
println(result)  // [4, 3, 1]
```

## Important Notes

1. **Immutability**: All `vector.*` and `matrix.*` methods return **new** vectors/matrices without modifying the original. This ensures functional programming patterns and prevents unintended side effects.

2. **Indexed Assignment**: While the methods are immutable, indexed assignment (`v[i] = x` or `m[i,j] = x`) **does** modify the vector/matrix in place.

3. **0-based Indexing**: All indices are 0-based (first element is at index 0).

4. **Type Safety**: Functions validate argument types and throw errors for invalid inputs.

5. **Bounds Checking**: All index operations perform bounds checking and throw errors for out-of-bounds access.

## Next Steps

- [Standard Library Functions](functions.md) - Learn about other built-in functions
- [Examples](examples/data-structures.md) - See practical vector/matrix examples
- [Language Syntax](language/syntax.md) - Understand Rhodesia syntax
- [Performance Guide](api/performance.md) - Optimize your code
