---
title: Indexing & Slicing
layout: default
nav_order: 4
parent: Language Reference
---

# Indexing & Slicing

`vec` and `mat` values support indexing (single element) and slicing
(sub-range). Both are 0-based and bounds-checked.

## Vector Indexing

```rhodesia
vec: v = [10, 20, 30, 40, 50]
int: x = v[0]      // 10
int: y = v[4]      // 50
// v[5]              // IndexError: out of bounds
```

### Negative Indices

Negative indices count from the end:

```rhodesia
vec: v = [10, 20, 30, 40, 50]
int: last = v[-1]    // 50
int: prev = v[-2]    // 40
```

### Indexed Assignment

```rhodesia
vec: v = [1, 2, 3]
v[0] = 99
println(v)        // [99, 2, 3]
```

## Vector Slicing

```rhodesia
vec: v = [10, 20, 30, 40, 50]
vec: s = v[1:4]    // [20, 30, 40]
```

The slice `v[start:end]` includes `start` and excludes `end`. Both ends are
optional:

```rhodesia
vec: v = [10, 20, 30, 40, 50]
vec: a = v[:3]     // [10, 20, 30]
vec: b = v[2:]     // [30, 40, 50]
vec: c = v[:]      // [10, 20, 30, 40, 50]  (full copy)
```

## Matrix Indexing

```rhodesia
mat: M = [[1, 2, 3],
          [4, 5, 6],
          [7, 8, 9]]

float64: a = M[0, 0]   // 1
float64: c = M[2, 2]   // 9
```

Indexed assignment:

```rhodesia
M[1, 1] = 50
println(M)              // diagonal modified
```

## Matrix Slicing

Slice rows, columns, or both:

```rhodesia
mat: M = [[1, 2, 3],
          [4, 5, 6],
          [7, 8, 9]]

mat: r1 = M[1, :]   // row 1: [4, 5, 6]
mat: c1 = M[:, 1]   // column 1: [2, 5, 8]
mat: q  = M[0:2, 0:2]   // top-left 2×2 block
```

## Bounds Checking

Out-of-range indices raise an `IndexError` at runtime. To pre-check, use
`math.size`, `math.rows`, or `math.cols`:

```rhodesia
vec: v = [1, 2, 3]
if i >= 0 && i < math.size(v) {
    println(v[i])
}
```

## Slicing Tuples

Tuples support indexing (read-only) but not slicing:

```rhodesia
tuple: t = (10, 20, 30)
int: x = t[0]      // 10
// t[1] = 99         // Error: tuples are immutable
```

## Strings

Strings support indexing for characters and `string.slice` for substrings:

```rhodesia
string: s = "Hello"
string: ch = string.at(s, 0)            // "H"
string: sub = string.slice(s, 1, 4)      // "ell"
```

See [string module](../standard-library/string.md) for the full reference.