---
title: vector
layout: default
nav_order: 5
parent: Standard Library
---

# `vector` module

Immutable vector helpers. Every operation returns a **new** vector; the
input is never mutated.

| Function | Description |
|---|---|
| `vector.append(v, x)` | Append scalar `x` to `v` |
| `vector.remove(v, i)` | Remove element at index `i` |
| `vector.reverse(v)` | Reverse element order |
| `vector.insert(v, i, x)` | Insert `x` at index `i` |
| `vector.size(v)` | Number of elements |

## Examples

```rhodesia
vec: v = [1.0, 2.0, 3.0]
vec: v2 = vector.append(v, 4.0)            // [1, 2, 3, 4]
vec: v3 = vector.reverse(v2)              // [4, 3, 2, 1]
vec: v4 = vector.remove(v3, 1)            // [4, 2, 1]

println(v)                                // [1, 2, 3]   — unchanged
println(v2)                               // [1, 2, 3, 4]
```

For vector math (dot, norm, sum, mean), see [math](math.md). For advanced
linear-algebra helpers (normalize, project, cross, reflect), see
[libs/math](../standard-library/libs-math.md).