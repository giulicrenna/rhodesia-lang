# Vector and Matrix Methods Documentation

This document describes the new `vector` and `matrix` modules in Rhodesia, providing methods for manipulating vectors and matrices.

## Vector Module (`vector`)

All vector methods are **immutable** - they return new vectors without modifying the original.

### `vector.append(vec, value) -> vec`

Appends a scalar value to the end of a vector.

**Parameters:**
- `vec`: The vector to append to
- `value`: A scalar value (int or float64) to append

**Returns:** A new vector with the value appended

**Example:**
```rhodesia
vec: v = [1.0, 2.0, 3.0]
vec: v2 = vector.append(v, 4.0)
println(v2)  // vec[4](1, 2, 3, 4)
println(v)   // vec[3](1, 2, 3) - unchanged
```

---

### `vector.remove(vec, index) -> vec`

Removes an element at the specified index from a vector.

**Parameters:**
- `vec`: The vector to remove from
- `index`: The 0-based index of the element to remove (int)

**Returns:** A new vector without the element at the specified index

**Example:**
```rhodesia
vec: v = [1.0, 2.0, 3.0, 4.0, 5.0]
vec: v2 = vector.remove(v, 2)
println(v2)  // vec[4](1, 2, 4, 5)
println(v)   // vec[5](1, 2, 3, 4, 5) - unchanged
```

---

### `vector.reverse(vec) -> vec`

Reverses the order of elements in a vector.

**Parameters:**
- `vec`: The vector to reverse

**Returns:** A new vector with elements in reverse order

**Example:**
```rhodesia
vec: v = [1.0, 2.0, 3.0, 4.0, 5.0]
vec: v2 = vector.reverse(v)
println(v2)  // vec[5](5, 4, 3, 2, 1)
println(v)   // vec[5](1, 2, 3, 4, 5) - unchanged
```

---

### `vector.insert(vec, index, value) -> vec`

Inserts a scalar value at the specified index in a vector.

**Parameters:**
- `vec`: The vector to insert into
- `index`: The 0-based index where the value should be inserted (int)
- `value`: A scalar value (int or float64) to insert

**Returns:** A new vector with the value inserted

**Example:**
```rhodesia
vec: v = [1.0, 2.0, 3.0]
vec: v2 = vector.insert(v, 1, 99.0)
println(v2)  // vec[4](1, 99, 2, 3)
println(v)   // vec[3](1, 2, 3) - unchanged
```

---

### `vector.size(vec) -> int`

Returns the number of elements in a vector.

**Parameters:**
- `vec`: The vector

**Returns:** The size of the vector (int)

**Example:**
```rhodesia
vec: v = [1.0, 2.0, 3.0, 4.0, 5.0]
println(vector.size(v))  // 5
```

**Note:** This is an alias for `math.size(vec)`.

---

## Matrix Module (`matrix`)

All matrix methods are **immutable** - they return new matrices without modifying the original.

### `matrix.append_row(mat, row_vec) -> mat`

Appends a row (vector) to the bottom of a matrix.

**Parameters:**
- `mat`: The matrix to append to
- `row_vec`: A vector representing the new row (must have size equal to matrix column count)

**Returns:** A new matrix with the row appended

**Example:**
```rhodesia
mat: m = [[1.0, 2.0, 3.0],
          [4.0, 5.0, 6.0]]
vec: new_row = [7.0, 8.0, 9.0]
mat: m2 = matrix.append_row(m, new_row)
println(m2)  // mat[3x3] with new row added
println(m)   // mat[2x3] - unchanged
```

---

### `matrix.append_col(mat, col_vec) -> mat`

Appends a column (vector) to the right side of a matrix.

**Parameters:**
- `mat`: The matrix to append to
- `col_vec`: A vector representing the new column (must have size equal to matrix row count)

**Returns:** A new matrix with the column appended

**Example:**
```rhodesia
mat: m = [[1.0, 2.0],
          [3.0, 4.0],
          [5.0, 6.0]]
vec: new_col = [10.0, 20.0, 30.0]
mat: m2 = matrix.append_col(m, new_col)
println(m2)  // mat[3x3] with new column added
println(m)   // mat[3x2] - unchanged
```

---

### `matrix.remove_row(mat, index) -> mat`

Removes a row at the specified index from a matrix.

**Parameters:**
- `mat`: The matrix to remove from
- `index`: The 0-based index of the row to remove (int)

**Returns:** A new matrix without the row at the specified index

**Example:**
```rhodesia
mat: m = [[1.0, 2.0, 3.0],
          [4.0, 5.0, 6.0],
          [7.0, 8.0, 9.0]]
mat: m2 = matrix.remove_row(m, 1)
println(m2)  // mat[2x3] without row 1
println(m)   // mat[3x3] - unchanged
```

---

### `matrix.remove_col(mat, index) -> mat`

Removes a column at the specified index from a matrix.

**Parameters:**
- `mat`: The matrix to remove from
- `index`: The 0-based index of the column to remove (int)

**Returns:** A new matrix without the column at the specified index

**Example:**
```rhodesia
mat: m = [[1.0, 2.0, 3.0],
          [4.0, 5.0, 6.0],
          [7.0, 8.0, 9.0]]
mat: m2 = matrix.remove_col(m, 1)
println(m2)  // mat[3x2] without column 1
println(m)   // mat[3x3] - unchanged
```

---

### `matrix.rows(mat) -> int`

Returns the number of rows in a matrix.

**Parameters:**
- `mat`: The matrix

**Returns:** The number of rows (int)

**Example:**
```rhodesia
mat: m = [[1.0, 2.0],
          [3.0, 4.0],
          [5.0, 6.0]]
println(matrix.rows(m))  // 3
```

**Note:** This is an alias for `math.rows(mat)`.

---

### `matrix.cols(mat) -> int`

Returns the number of columns in a matrix.

**Parameters:**
- `mat`: The matrix

**Returns:** The number of columns (int)

**Example:**
```rhodesia
mat: m = [[1.0, 2.0],
          [3.0, 4.0],
          [5.0, 6.0]]
println(matrix.cols(m))  // 2
```

**Note:** This is an alias for `math.cols(mat)`.

---

### `matrix.size(mat) -> int`

Returns the total number of elements in a matrix (rows × columns).

**Parameters:**
- `mat`: The matrix

**Returns:** The total size (int)

**Example:**
```rhodesia
mat: m = [[1.0, 2.0],
          [3.0, 4.0],
          [5.0, 6.0]]
println(matrix.size(m))  // 6
```

**Note:** This is an alias for `math.size(mat)`.

---

## Indexed Reassignment

Both vectors and matrices support indexed reassignment, allowing you to modify individual elements.

### Vector Indexed Reassignment

```rhodesia
vec: v = [1.0, 2.0, 3.0, 4.0, 5.0]
v[0] = 100.0    // Modify first element
v[2] = 300.0    // Modify third element
println(v)      // vec[5](100, 2, 300, 4, 5)
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

---

## Important Notes

1. **Immutability**: All `vector.*` and `matrix.*` methods return **new** vectors/matrices without modifying the original. This ensures functional programming patterns and prevents unintended side effects.

2. **Indexed Assignment**: While the methods are immutable, indexed assignment (`v[i] = x` or `m[i,j] = x`) **does** modify the vector/matrix in place.

3. **0-based Indexing**: All indices are 0-based (first element is at index 0).

4. **Type Safety**: Functions validate argument types and throw errors for invalid inputs.

5. **Bounds Checking**: All index operations perform bounds checking and throw errors for out-of-bounds access.

---

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
