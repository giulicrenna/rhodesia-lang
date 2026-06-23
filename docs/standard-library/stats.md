---
title: stats
layout: default
nav_order: 3
parent: Standard Library
---

# `stats` module

Descriptive and inferential statistics on `vec` and `mat`.

## Central Tendency

| Function | Description |
|---|---|
| `stats.mean(v)` | Arithmetic mean |
| `stats.median(v)` | Median value |
| `stats.percentile(v, p)` | p-th percentile (0–100) |

## Dispersion

| Function | Description |
|---|---|
| `stats.var(v)` | Variance (population) |
| `stats.std(v)` | Standard deviation |
| `stats.zscore(v)` | Z-score normalized copy of `v` |

## Covariance & Correlation

| Function | Description |
|---|---|
| `stats.cov(x, y)` | Covariance of `x` and `y` |
| `stats.corr(x, y)` | Pearson correlation coefficient |

## Higher Moments

| Function | Description |
|---|---|
| `stats.skewness(v)` | Sample skewness |
| `stats.kurtosis(v)` | Sample excess kurtosis |

## Min / Max

`stats.min(v)`, `stats.max(v)`.

## Examples

```rhodesia
vec: data = [1.0, 2.0, 3.0, 4.0, 5.0]

float64: m  = stats.mean(data)         // 3.0
float64: sd = stats.std(data)          // ≈ 1.414
float64: p  = stats.percentile(data, 90)

vec: z = stats.zscore(data)            // ≈ [-1.41, -0.71, 0, 0.71, 1.41]
```

For more advanced statistics — descriptive stats helpers, probability
distributions, regression — see the bundled
[`libs/math`](../standard-library/libs-math.md) library.