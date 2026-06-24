---
title: Operators
layout: default
nav_order: 4
parent: Language Reference
---

# Operators

Rhodesia supports arithmetic, comparison, logical, **bitwise**, ternary, and
matrix/vector operators. Most operators follow their mathematical meaning
on `vec` and `mat` values (e.g. `A * B` is matrix product, `2 * v`
broadcasts).

## Arithmetic

| Operator | Description | Applies to |
|----------|-------------|------------|
| `+` | Addition | `int`, `float64`, `vec`, `mat` |
| `-` | Subtraction | `int`, `float64`, `vec`, `mat` |
| `*` | Multiplication | `int`, `float64`, `vec` (elementwise), `mat` (matrix product) |
| `/` | Division | `int`, `float64`, `vec` (elementwise) |
| `%` | Modulo | `int` |

For `/` on `vec`: `v / s`, `s / v`, and `v / v` all work element-wise
(raises a division-by-zero runtime error if any divisor element is zero).

Broadcasting: a scalar combined with a vector or matrix applies elementwise.

```rhodesia
vec: v = [1, 2, 3]
vec: scaled = 2 * v          // [2, 4, 6]

mat: M = [[1, 2], [3, 4]]
mat: scaled = 0.5 * M        // elementwise
```

## Comparison

| Operator | Meaning |
|---|---|
| `==` | Equal |
| `!=` | Not equal |
| `<` | Less than |
| `<=` | Less or equal |
| `>` | Greater than |
| `>=` | Greater or equal |

Result type is `int` (`0` or `1`).

## Logical

| Operator | Meaning |
|---|---|
| `&&` (`and`) | Logical AND |
| `\|\|` (`or`) | Logical OR |
| `!` (`not`) | Logical NOT |

```rhodesia
if x > 0 && y > 0 {
    println("First quadrant")
}
```

## Bitwise

Bitwise operators work on integer types (`int`, `int8`–`int32`,
`uint8`–`uint32`, `byte`).

| Operator | Description |
|----------|-------------|
| `&` | AND |
| `\|` | OR |
| `^` | XOR |
| `~` | NOT (one's complement) |
| `<<` | Left shift |
| `>>` | Right shift |

```rhodesia
int: mask = 0b1100
int: val  = 0b1010
int: bit  = mask & val      // 0b1000 = 8
```

## Ternary

```rhodesia
int: sign = x >= 0 ? 1 : -1
string: label = n == 1 ? "one" : "many"
```

## Vector / Matrix Operators

| Expression | Meaning |
|---|---|
| `u + v`, `u - v` | elementwise |
| `s * v` | scalar broadcast |
| `v * s` | scalar broadcast |
| `u * v` (vectors) | elementwise product |
| `u * M`, `M * v` | matrix-vector product |
| `A * B` | matrix product |
| `M / s` | scalar divide (broadcast) |
| `s / v` | scalar / vector (elementwise broadcast) |
| `v / v` | elementwise divide |

## Element-wise user functions: `.map(fn)`

To apply a user-defined function (or lambda) over every element of a
`vec` or `mat`, call `.map(fn)`:

```rhodesia
vec: x = numerical.linspace(0.1, 10.0, 50)

fun reciprocal_sin(float64: xx) -> float64 {
    return 1.0 / math.sin(xx)
}

vec: y = x.map(reciprocal_sin)              // named fn
vec: y2 = x.map(fn(xx) -> xx * xx + 1.0)   // inline lambda
```

`.map(fn)` is always available — no `include` required. The function
must take a single numeric scalar and return a numeric scalar. For
`mat`, the function is applied to every cell.

For reductions, use the `math` module: `math.sum`, `math.mean`, `math.dot`,
`math.norm`. For inversion use `math.inv(M)`.

## Operator Precedence

From highest to lowest:

1. Parentheses: `()`
2. Indexing / slicing: `[]`
3. Unary: `-`, `!`, `~`
4. Multiplicative: `*`, `/`, `%`
5. Additive: `+`, `-`
6. Shift: `<<`, `>>`
7. Comparison: `<`, `<=`, `>`, `>=`
8. Equality: `==`, `!=`
9. Bitwise AND: `&`
10. Bitwise XOR: `^`
11. Bitwise OR: `|`
12. Logical AND: `&&`
13. Logical OR: `||`
14. Ternary: `? :`

Use parentheses whenever the precedence is unclear.