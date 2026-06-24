---
title: libs/math user library
layout: default
nav_order: 13
parent: Standard Library
---

# `libs/math` — User-Space Library

Rhodesia ships a sizeable user-space library called `math`. It is **not**
the same as the built-in [`math`](math.md) module — the built-in `math.*`
covers basic elementwise operations and a few reductions; this `math`
library (imported with `include math`) bundles ~150 higher-level helpers
across four sub-modules.

> the naming overlap is intentional but documented to avoid
> confusion. The built-in `math.*` is in C++ (`Builtins.hpp`); this
> library lives in `libs/math/*.rho`.

## Importing

```rhodesia
include math
```

Functions become available under their sub-module prefix:
`math.vec_normalize_safe`, `math.linear_regression`, `math.normal_pdf`,
etc.

## Sub-Modules

| Sub-module | What it adds |
|---|---|
| `core` | Safe-divide, power, sign, clamp, lerp, step, smoothstep, … |
| `core/trigonometry` | Degrees↔radians, wrap-angle, Taylor/approx trig, hyperbolic |
| `linear_algebra/matrices` | Trace, determinant 2×2/3×3, Frobenius, Hadamard, row/col stats, block |
| `linear_algebra/vectors` | Magnitude, distance, project, reject, angle, cross, reflect, normalize-safe |
| `linear_algebra/decompositions` | LU / QR / Cholesky (3×3), 2×2 eigenvalues / SVD, rank / null-space estimates |
| `numerical/integration` | Rectangle, midpoint, trapezoidal, Simpson, adaptive, Romberg, Gaussian, Monte Carlo, tanh-sinh |
| `numerical/optimization` | Golden section, gradient descent, conjugate gradient, Newton, Nelder-Mead, BFGS, simulated annealing, genetic |
| `numerical/root_finding` | Bisection, Newton-Raphson, secant, fixed-point, Brent, polynomial roots |
| `statistics/descriptive` | Mean, variance, std-dev, median, mode, IQR, skew, kurtosis, covariance, z-score normalize |
| `statistics/probability` | Normal / uniform / exponential / Poisson / binomial / chi-square / student-t distributions |
| `statistics/regression` | Linear / polynomial / ridge / lasso / logistic regression, MSE / RMSE / MAE / MAPE, k-fold |

## Source Layout

```
libs/
  math/
    info.json
    index.rho
    core/
      core.rho
      trigonometry.rho
    linear_algebra/
      vectors.rho
      matrices.rho
      decompositions.rho
    numerical/
      integration.rho
      optimization.rho
      root_finding.rho
    statistics/
      descriptive.rho
      probability.rho
      regression.rho
```

## Example

```rhodesia
include math

vec: x = [1.0, 2.0, 3.0, 4.0]
vec: y = [2.1, 3.9, 6.1, 7.8]

// Linear regression: returns (slope, intercept)
vec: coef = math.linear_regression(x, y)
float64: slope     = coef[0]
float64: intercept = coef[1]

float64: r2 = math.r_squared(y, math.linear_regression_predict(coef, x))
println("slope =", slope, "intercept =", intercept, "R² =", r2)
```

A full app using this library lives in
[`examples/10_applications/math_library/`](https://github.com/giulicrenna/rhodesia-lang/tree/main/examples/10_applications/math_library).

## Authoring Your Own Library

To package your own `.rho` files as a library, create a folder with an
`info.json` next to your source files. See [Modules](../language/modules.md)
for the full format.