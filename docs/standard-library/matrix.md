---
title: matrix
layout: default
nav_order: 6
parent: Standard Library
---

# `matrix` module

Immutable matrix helpers. Every operation returns a **new** matrix; the
input is never mutated.

| Function | Description |
|---|---|
| `matrix.append_row(M, row)` | Append `row` (vector) as a new row |
| `matrix.append_col(M, col)` | Append `col` (vector) as a new column |
| `matrix.remove_row(M, i)` | Remove row at index `i` |
| `matrix.remove_col(M, j)` | Remove column at index `j` |
| `matrix.rows(M)` | Number of rows (alias of `math.rows`) |
| `matrix.cols(M)` | Number of columns (alias of `math.cols`) |
| `matrix.size(M)` | Total element count (alias of `math.size`) |

## Examples

```rhodesia
mat: M = [[1.0, 2.0, 3.0],
          [4.0, 5.0, 6.0]]

vec: new_row = [7.0, 8.0, 9.0]
mat: M2 = matrix.append_row(M, new_row)        // 3×3

mat: M3 = matrix.remove_col(M2, 0)             // 3×2
```

For matrix math (transpose, inverse, multiplication), see
[math](math.md). For advanced linear-algebra helpers (determinant, trace,
Hadamard, eigenvalues, decompositions), see
[libs/math](../standard-library/libs-math.md).