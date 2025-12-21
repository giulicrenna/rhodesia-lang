# Machine Learning Examples

This page demonstrates machine learning algorithms implemented in Rhodesia.

## Table of Contents

- [Machine Learning Examples](#machine-learning-examples)
  - [Table of Contents](#table-of-contents)
  - [Linear Regression](#linear-regression)
    - [Simple Linear Regression](#simple-linear-regression)
    - [Multiple Linear Regression](#multiple-linear-regression)
  - [K-Means Clustering](#k-means-clustering)
    - [Simple K-Means Implementation](#simple-k-means-implementation)
  - [Data Science Examples](#data-science-examples)
    - [Statistical Analysis](#statistical-analysis)
    - [Data Normalization](#data-normalization)
  - [Next Steps](#next-steps)

## Linear Regression

### Simple Linear Regression

```rhodesia
// Linear regression example: y = β₀ + β₁x + ε
println("=== Simple Linear Regression ===")
println("")

// Sample data: x values
vec: x = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0]

// Sample data: y values (approximately y = 0.5 + 2*x + noise)
vec: y = [2.3, 4.1, 6.8, 8.5, 10.2, 12.9, 14.1, 16.4, 18.7, 20.3]

println("X values:", x)
println("Y values:", y)
println("")

// Design matrix with intercept
mat: X = math.ones(math.size(x), 2)
for i in range(math.size(x)) {
    X[i, 1] = x[i]
}

println("Design matrix X:")
println(X)
println("")

// OLS Estimator: β = (X'X)^(-1) X'y
fun ols_fit(mat: X, vec: y) -> vec {
    mat: xt = math.transpose(X)
    mat: xtx = xt * X
    mat: xtx_inv = math.inv(xtx)
    vec: xty = xt * y
    vec: beta = xtx_inv * xty
    return beta
}

// Compute coefficients
vec: beta = ols_fit(X, y)

println("Regression Results:")
println("Intercept (β₀):", beta[0])
println("Slope (β₁):    ", beta[1])
println("")

// Predictions
vec: y_pred = X * beta

println("Predictions vs Actual:")
println("Index | Actual | Predicted")
for i in range(math.size(y)) {
    print(i)
    print("     | ")
    print(y[i])
    print(" | ")
    println(y_pred[i])
}
println("")

// Model evaluation
float64: y_mean = stats.mean(y)
vec: residuals = y - y_pred
vec: y_centered = y - y_mean * math.ones(math.size(y))

float64: ss_res = math.dot(residuals, residuals)
float64: ss_tot = math.dot(y_centered, y_centered)
float64: r_squared = 1.0 - ss_res / ss_tot
float64: rmse = math.sqrt(ss_res / math.size(y))

println("Model Evaluation:")
println("Sum of Squared Residuals:", ss_res)
println("Total Sum of Squares:    ", ss_tot)
println("R-squared:               ", r_squared)
println("RMSE:                    ", rmse)
println("")
```

### Multiple Linear Regression

```rhodesia
println("=== Multiple Linear Regression ===")
println("")

// Generate synthetic data: y = 2 + 3*x1 + 1.5*x2 + noise
int: n_obs = 50
mat: X_multi = math.ones(n_obs, 3)  // intercept + 2 variables

vec: x1 = math.range(n_obs) / 10.0
vec: x2 = math.range(n_obs) / 5.0

// y = 2 + 3*x1 + 1.5*x2 + noise
vec: noise = [0.1, -0.2, 0.3, -0.1, 0.4, -0.3, 0.2, -0.2, 0.1, -0.1,
              0.3, -0.4, 0.2, -0.3, 0.1, -0.2, 0.4, -0.1, 0.3, -0.2,
              0.2, -0.3, 0.1, -0.4, 0.3, -0.2, 0.1, -0.3, 0.4, -0.1,
              0.2, -0.2, 0.3, -0.4, 0.1, -0.3, 0.2, -0.1, 0.4, -0.2,
              0.3, -0.1, 0.2, -0.3, 0.1, -0.2, 0.4, -0.1, 0.3, -0.2]

vec: y_multi = 2 + 3*x1 + 1.5*x2 + noise

// Fill design matrix
for i in range(n_obs) {
    X_multi[i, 1] = x1[i]
    X_multi[i, 2] = x2[i]
}

// Fit model
vec: beta_multi = ols_fit(X_multi, y_multi)

println("Multiple Regression Results:")
println("Intercept (β₀):", beta_multi[0], "(expected: 2.0)")
println("Coefficient x1 (β₁):", beta_multi[1], "(expected: 3.0)")
println("Coefficient x2 (β₂):", beta_multi[2], "(expected: 1.5)")
println("")

// Predictions and evaluation
vec: y_pred_multi = X_multi * beta_multi
float64: r_squared_multi = r_squared(y_multi, y_pred_multi)

println("Multiple Regression Evaluation:")
println("R-squared:", r_squared_multi)
println("")
```

## K-Means Clustering

### Simple K-Means Implementation

```rhodesia
println("=== K-Means Clustering ===")
println("")

fun k_means(mat: data, int: k, int: max_iter) -> vec {
    int: n = math.rows(data)
    int: d = math.cols(data)

    // Initialize centroids randomly (simple deterministic approach)
    mat: centroids = math.zeros(k, d)
    for i in range(k) {
        int: idx = i * n / k
        for j in range(d) {
            centroids[i, j] = data[idx, j]
        }
    }

    // Main K-means loop
    for iter in range(max_iter) {
        vec: labels = math.zeros(n)

        // Assign points to nearest centroids
        for i in range(n) {
            float64: min_dist = 1e100
            int: best_centroid = 0

            for c in range(k) {
                vec: point = math.zeros(d)
                vec: centroid = math.zeros(d)

                for j in range(d) {
                    point[j] = data[i, j]
                    centroid[j] = centroids[c, j]
                }

                vec: diff = point - centroid
                float64: dist = math.norm(diff)

                if dist < min_dist {
                    min_dist = dist
                    best_centroid = c
                }
            }

            labels[i] = best_centroid
        }

        // Update centroids
        vec: counts = math.zeros(k)
        mat: new_centroids = math.zeros(k, d)

        for i in range(n) {
            int: c = labels[i]
            counts[c] = counts[c] + 1

            for j in range(d) {
                new_centroids[c, j] = new_centroids[c, j] + data[i, j]
            }
        }

        for c in range(k) {
            if counts[c] > 0 {
                for j in range(d) {
                    centroids[c, j] = new_centroids[c, j] / counts[c]
                }
            }
        }
    }

    return labels
}

// Sample 2D data
mat: data = [
    [1.0, 1.0],
    [1.5, 2.0],
    [3.0, 3.0],
    [5.0, 8.0],
    [8.0, 8.0],
    [1.0, 0.0],
    [0.0, 1.0],
    [2.0, 1.5],
    [3.5, 3.5],
    [5.5, 8.5]
]

println("Sample data for clustering:")
println(data)
println("")

// Run K-means with 2 clusters
vec: clusters = k_means(data, 2, 10)

println("Cluster assignments:")
for i in range(math.rows(data)) {
    print("Point ")
    print(i)
    print(": [")
    print(data[i, 0])
    print(", ")
    print(data[i, 1])
    print("] -> Cluster ")
    println(clusters[i])
}
println("")
```

## Data Science Examples

### Statistical Analysis

```rhodesia
println("=== Statistical Analysis ===")
println("")

// Generate sample data
vec: sample1 = [12.5, 15.2, 18.7, 14.1, 16.8, 19.3, 13.9, 17.5, 15.8, 18.2]
vec: sample2 = [14.1, 16.8, 19.3, 15.2, 17.5, 20.1, 14.8, 18.3, 16.5, 19.0]

println("Sample 1:", sample1)
println("Sample 2:", sample2)
println("")

// Basic statistics
fun calculate_stats(vec: data) -> vec {
    float64: mean_val = stats.mean(data)
    vec: diff = data - mean_val
    vec: diff_sq = diff * diff
    float64: variance = stats.mean(diff_sq)
    float64: std_dev = math.sqrt(variance)

    return [mean_val, variance, std_dev, math.min(data), math.max(data)]
}

vec: stats1 = calculate_stats(sample1)
vec: stats2 = calculate_stats(sample2)

println("Sample 1 Statistics:")
println("  Mean:    ", stats1[0])
println("  Variance:", stats1[1])
println("  Std Dev: ", stats1[2])
println("  Min:     ", stats1[3])
println("  Max:     ", stats1[4])
println("")

println("Sample 2 Statistics:")
println("  Mean:    ", stats2[0])
println("  Variance:", stats2[1])
println("  Std Dev: ", stats2[2])
println("  Min:     ", stats2[3])
println("  Max:     ", stats2[4])
println("")

// T-test (simplified)
fun t_test(vec: a, vec: b) -> vec {
    float64: mean_a = stats.mean(a)
    float64: mean_b = stats.mean(b)

    int: n_a = math.size(a)
    int: n_b = math.size(b)

    vec: diff_a = a - mean_a
    vec: diff_b = b - mean_b

    float64: var_a = stats.mean(diff_a * diff_a)
    float64: var_b = stats.mean(diff_b * diff_b)

    // t-statistic
    float64: diff_means = mean_a - mean_b
    float64: se = math.sqrt(var_a/n_a + var_b/n_b)
    float64: t_stat = diff_means / se

    // degrees of freedom (Welch-Satterthwaite)
    float64: df = (var_a/n_a + var_b/n_b) * (var_a/n_a + var_b/n_b) /
                  (var_a*var_a/(n_a*n_a*(n_a-1)) + var_b*var_b/(n_b*n_b*(n_b-1)))

    return [t_stat, df]
}

vec: t_result = t_test(sample1, sample2)
println("T-test Results:")
println("  t-statistic: ", t_result[0])
println("  degrees of freedom: ", t_result[1])
println("  Significant if |t| > 2")
println("")
```

### Data Normalization

```rhodesia
println("=== Data Normalization ===")
println("")

fun min_max_normalize(vec: data) -> vec {
    float64: min_val = 1e100
    float64: max_val = -1e100

    // Find min and max
    for val in data {
        if val < min_val { min_val = val }
        if val > max_val { max_val = val }
    }

    // Normalize
    vec: normalized = math.zeros(math.size(data))
    float64: range_val = max_val - min_val

    if range_val == 0 {
        return normalized  // All values are the same
    }

    for i in range(math.size(data)) {
        normalized[i] = (data[i] - min_val) / range_val
    }

    return normalized
}

fun z_score_normalize(vec: data) -> vec {
    float64: mean_val = stats.mean(data)

    // Calculate standard deviation
    vec: diff = data - mean_val
    vec: diff_sq = diff * diff
    float64: variance = stats.mean(diff_sq)
    float64: std_dev = math.sqrt(variance)

    if std_dev == 0 {
        return math.zeros(math.size(data))
    }

    return diff / std_dev
}

vec: raw_data = [10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0, 100.0]

println("Original data:", raw_data)
println("Min-Max normalized:", min_max_normalize(raw_data))
println("Z-score normalized:", z_score_normalize(raw_data))
println("")
```

## Next Steps

- [Data Structures Examples](data-structures.md) - Review vector/matrix operations
- [Basic Examples](basics.md) - Learn basic Rhodesia syntax
- [Standard Library](standard-library/functions.md) - Explore built-in functions
- [Performance Guide](api/performance.md) - Optimize your ML algorithms
