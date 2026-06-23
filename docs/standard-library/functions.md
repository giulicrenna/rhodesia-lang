---
title: Overview
layout: default
nav_order: 1
parent: Standard Library
---

# Standard Library — Overview

A small set of functions and modules ship with the `rhodesia` interpreter.
This page summarizes them; each module has its own page in this section.

## Top-level built-ins

These work without any module prefix:

| Name | Purpose |
|---|---|
| `print(...)` | Print values, no newline |
| `println(...)` | Print values, with newline |
| `get_tick()` | Monotonic high-resolution timestamp |
| `make_complex(real, imag)` | Construct a complex number |
| `make_set(...)` | Build a set from values |
| `make_tuple(...)` | Build a tuple |
| `make_record(...)` | Build a record |
| `size(x)` | Size — vector length, matrix element count, or string length |
| `rows(M)` | Matrix rows (alias of `math.rows`) |
| `cols(M)` | Matrix columns (alias of `math.cols`) |
| `sum(v)` | Sum of elements (alias of `math.sum`) |
| `mean(v)` | Mean of elements (alias of `math.mean`) |
| `norm(v)` | L2 / Frobenius norm (alias of `math.norm`) |
| `dot(u, v)` | Dot product (alias of `math.dot`) |
| `transpose(M)` | Transpose (alias of `math.transpose`) |
| `inv(M)` | Matrix inverse (alias of `math.inv`) |
| `sqrt(x)` | Square root (alias of `math.sqrt`) |
| `abs(x)` | Absolute value (alias of `math.abs`) |
| `floor` / `ceil` / `round` / `trunc` | Rounding aliases |
| `min(...)` / `max(...)` | Min/max aliases |
| `range(...)` | Integer sequence (alias of `math.range`) |
| `zeros(...)` / `ones(...)` / `eye(...)` | Allocation helpers |
| `clamp(x, lo, hi)` | Clamp helper |

## Modules

| Module | Page |
|---|---|
| `math` | [math](math.md) |
| `stats` | [stats](stats.md) |
| `numerical` | [numerical](numerical.md) |
| `vector` | [vector](vector.md) |
| `matrix` | [matrix](matrix.md) |
| `string` | [string](string.md) |
| `mapping` | [mapping](mapping.md) |
| `array` | [array](array.md) |
| `datetime` | [datetime](datetime.md) |
| `io` | [io](io.md) |
| `net` | [net](net.md) |

## User-Space Library: `libs/math`

A larger numerical library (linear algebra, numerical methods, statistics,
regression) ships in `libs/math/` and is loaded with `include math`. See
[libs-math](libs-math.md) for the full surface area.

## Conventions

- Functions are **pure** unless documented otherwise. `vector.append`
  returns a new vector rather than mutating in place.
- Errors are **raised, not returned**. Wrap risky calls in
  [`try`/`catch`](../language/exceptions.md).
- Numeric results default to `float64`; integer arguments are promoted.