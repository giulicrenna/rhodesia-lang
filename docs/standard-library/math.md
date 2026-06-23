---
title: math
layout: default
nav_order: 2
parent: Standard Library
---

# `math` module

Elementwise math, trigonometry, and basic linear algebra. Most `math.*`
functions accept scalars or vectors; for matrices they apply elementwise.

## Constants

| Constant | Value |
|---|---|
| `math.PI` | π |
| `math.E` | e |
| `math.PHI` | golden ratio |
| `math.SQRT_2` | √2 |
| `math.SQRT_3` | √3 |

## Creation

| Function | Returns |
|---|---|
| `math.zeros(n)` | vector of n zeros |
| `math.zeros(m, n)` | m×n matrix of zeros |
| `math.ones(n)` | vector of n ones |
| `math.ones(m, n)` | m×n matrix of ones |
| `math.eye(n)` | n×n identity matrix |
| `math.range(n)` | `[0, 1, …, n-1]` |
| `math.range(a, b)` | `[a, a+1, …, b-1]` |

## Elementwise

`math.sqrt`, `math.exp`, `math.log`, `math.abs`.

## Trigonometry

`math.sin`, `math.cos`, `math.tan`, `math.asin`, `math.acos`, `math.atan`,
`math.atan2`, `math.sinh`, `math.cosh`, `math.tanh`.

## Power / Roots

`math.pow(x, n)`, `math.cbrt(x)`.

## Rounding

`math.floor(x)`, `math.ceil(x)`, `math.round(x)`, `math.trunc(x)`.

## Min / Max

`math.min(...)` and `math.max(...)` accept a mix of scalars and vectors.
When every argument is an integer, the result is an integer.

## Utilities

`math.clamp(x, lo, hi)`, `math.sign(x)`, `math.lerp(a, b, t)`,
`math.deg2rad(x)`, `math.rad2deg(x)`, `math.factorial(n)`.

## Linear Algebra

| Function | Description |
|---|---|
| `math.norm(v)` | L2 norm of a vector, Frobenius norm of a matrix |
| `math.dot(u, v)` | Dot product of two vectors |
| `math.transpose(M)` | Matrix (or vector) transpose |
| `math.inv(M)` | Matrix inverse (square, non-singular) |
| `math.rows(M)` | Number of rows |
| `math.cols(M)` | Number of columns |
| `math.size(x)` | Size — vector length, matrix element count, string length |

## Reductions

`math.sum`, `math.mean` — accept vectors and matrices.

## Examples

```rhodesia
float64: a = math.sqrt(2.0)             // 1.4142...
float64: b = math.sin(math.PI / 2.0)    // 1.0

vec: v = math.range(5)                  // [0, 1, 2, 3, 4]
mat: I = math.eye(3)                    // 3×3 identity

vec: u = [1.0, 2.0, 3.0]
vec: w = [4.0, 5.0, 6.0]
float64: d = math.dot(u, w)             // 32.0
```