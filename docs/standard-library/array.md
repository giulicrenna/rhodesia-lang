---
title: array
layout: default
nav_order: 9
parent: Standard Library
---

# `array` module

Dynamic, mutable, heterogeneous array helpers. Use `array` when you need
push/pop semantics; use `vec` for fixed-shape numeric work.

## Construction

| Function | Description |
|---|---|
| `array.create(...)` | Create from a list of values |
| `array.empty()` | New empty array |

## Mutation

| Function | Description |
|---|---|
| `array.push(a, value)` | Append `value` to `a` |
| `array.pop(a)` | Remove and return the last element |
| `array.resize(a, n)` | Resize `a` to length `n` |
| `array.clear(a)` | Remove all elements |

## Inspection

| Function | Description |
|---|---|
| `array.size(a)` | Number of elements |
| `array.isempty(a)` | `true` if `a` has no elements |

## Indexing

Arrays support `a[i]` for both reading and writing. Indices are 0-based
and bounds-checked.

## Examples

```rhodesia
arr: a = array.empty()
array.push(a, 1)
array.push(a, "two")
array.push(a, [3.0, 4.0])

int: n = array.size(a)          // 3
a[0] = 99                       // in-place update
```

For literal syntax, use `vec` for numeric arrays or `tuple` for short
immutable sequences.