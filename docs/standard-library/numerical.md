---
title: numerical
layout: default
nav_order: 4
parent: Standard Library
---

# `numerical` module

Numerical methods: integration, root finding, differentiation, polynomial
fitting, and assorted utilities.

## Polynomials & Equations

| Function | Description |
|---|---|
| `numerical.solve_quadratic(a, b, c)` | Roots of `ax² + bx + c = 0` |
| `numerical.polyval(p, x)` | Evaluate polynomial `p` at `x` |
| `numerical.polyfit(x, y, deg)` | Least-squares polynomial fit of degree `deg` |

## Sequences

| Function | Description |
|---|---|
| `numerical.linspace(a, b, n)` | `n` evenly-spaced points from `a` to `b` |
| `numerical.logspace(a, b, n)` | log-spaced points `10^a` … `10^b` |

## Cumulative / Diff

| Function | Description |
|---|---|
| `numerical.cumsum(v)` | Cumulative sum |
| `numerical.cumprod(v)` | Cumulative product |
| `numerical.diff(v)` | First differences |
| `numerical.gradient(v)` | Numerical gradient |

## Integration

| Function | Description |
|---|---|
| `numerical.trapz(y, x)` | Trapezoidal integration |
| `numerical.simps(y, x)` | Simpson's rule integration |
| `numerical.interp1d(x, y, xi)` | 1-D linear interpolation |

## Examples

```rhodesia
vec: x = numerical.linspace(0.0, 1.0, 100)
vec: y = numerical.linspace(0.0, 1.0, 100)
y = y * y                          // y = x²

float64: area = numerical.simps(y, x)   // ≈ 0.333 (∫x² dx from 0 to 1)
```

For more advanced methods — adaptive quadrature, Monte Carlo, gradient
descent, BFGS, Newton's method, Brent, bisection — see the bundled
[`libs/math`](../standard-library/libs-math.md) library.