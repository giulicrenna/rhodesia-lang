---
title: Data Science
layout: default
nav_order: 2
parent: Tutorials
---

# Data Science Tutorial

This tutorial covers data science applications in Rhodesia, including statistical analysis, data manipulation, and machine learning basics.

## Data Loading and Preparation

### Creating Sample Data

```rhodesia
// Generate synthetic data
vec: x_data = range(100)
vec: noise = zeros(100)

// Add some noise
for i in range(100) {
    noise[i] = (rand() - 0.5) * 10.0
}

// Create a linear relationship with noise
vec: y_data = zeros(100)
for i in range(100) {
    y_data[i] = 2.0 * x_data[i] + 5.0 + noise[i]
}

println("Generated", size(x_data), "data points")
```

### Basic Data Exploration

```rhodesia
fun explore_data(vec: x, vec: y) -> void {
    println("=== Data Exploration ===")
    println("X data size:", size(x))
    println("Y data size:", size(y))
    println("X range:", x[0], "to", x[size(x)-1])
    println("Y statistics:")
    println("  Mean:", mean(y))
    println("  Min:", min(y))
    println("  Max:", max(y))
    println("  Std Dev:", sqrt(var(y)))
}

explore_data(x_data, y_data)
```

## Statistical Analysis

### Descriptive Statistics

```rhodesia
fun descriptive_stats(vec: data) -> void {
    float64: n = size(data)
    float64: mean_val = mean(data)

    // Variance
    vec: diff = data - mean_val
    vec: squared_diff = diff * diff
    float64: variance = sum(squared_diff) / (n - 1)
    float64: std_dev = sqrt(variance)

    // Quartiles (simplified)
    vec: sorted = sort(data)
    float64: q1 = sorted[n/4]
    float64: median = sorted[n/2]
    float64: q3 = sorted[3*n/4]

    println("=== Descriptive Statistics ===")
    println("Count:", n)
    println("Mean:", mean_val)
    println("Std Dev:", std_dev)
    println("Min:", sorted[0])
    println("Q1:", q1)
    println("Median:", median)
    println("Q3:", q3)
    println("Max:", sorted[n-1])
}

// Helper function for sorting (simplified bubble sort)
fun sort(vec: data) -> vec {
    vec: sorted = data  // Copy
    int: n = size(sorted)

    for i in range(n) {
        for j in range(n-i-1) {
            if sorted[j] > sorted[j+1] {
                // Swap
                float64: temp = sorted[j]
                sorted[j] = sorted[j+1]
                sorted[j+1] = temp
            }
        }
    }

    return sorted
}
```

### Correlation Analysis

```rhodesia
fun correlation(vec: x, vec: y) -> float64 {
    float64: n = size(x)
    float64: mean_x = mean(x)
    float64: mean_y = mean(y)

    vec: dx = x - mean_x
    vec: dy = y - mean_y

    float64: numerator = dot(dx, dy)
    float64: denominator = sqrt(sum(dx*dx)) * sqrt(sum(dy*dy))

    return numerator / denominator
}

float64: corr = correlation(x_data, y_data)
println("Correlation coefficient:", corr)
```

## Linear Regression

### Simple Linear Regression

```rhodesia
fun linear_regression(vec: x, vec: y) -> vec {
    float64: n = size(x)
    float64: mean_x = mean(x)
    float64: mean_y = mean(y)

    vec: dx = x - mean_x
    vec: dy = y - mean_y

    float64: slope = dot(dx, dy) / dot(dx, dx)
    float64: intercept = mean_y - slope * mean_x

    vec: coefficients = [intercept, slope]
    return coefficients
}

vec: coeffs = linear_regression(x_data, y_data)
println("Intercept:", coeffs[0])
println("Slope:", coeffs[1])
```

### Making Predictions

```rhodesia
fun predict(vec: coeffs, vec: x_values) -> vec {
    float64: intercept = coeffs[0]
    float64: slope = coeffs[1]

    vec: predictions = slope * x_values + intercept
    return predictions
}

vec: test_x = [10, 20, 30, 40, 50]
vec: predictions = predict(coeffs, test_x)

println("Predictions for x =", test_x, ":")
println(predictions)
```

### Model Evaluation

```rhodesia
fun evaluate_model(vec: y_true, vec: y_pred) -> void {
    vec: residuals = y_true - y_pred
    vec: squared_residuals = residuals * residuals

    float64: mse = mean(squared_residuals)
    float64: rmse = sqrt(mse)

    float64: mean_y = mean(y_true)
    vec: total_variance = y_true - mean_y
    vec: squared_total = total_variance * total_variance
    float64: r_squared = 1.0 - sum(squared_residuals) / sum(squared_total)

    println("=== Model Evaluation ===")
    println("MSE:", mse)
    println("RMSE:", rmse)
    println("R²:", r_squared)
}

// Evaluate on training data
vec: y_pred = predict(coeffs, x_data)
evaluate_model(y_data, y_pred)
```

## Data Visualization (Text-based)

### Simple Histogram

```rhodesia
fun text_histogram(vec: data, int: bins) -> void {
    // Find min and max
    float64: min_val = 1e100
    float64: max_val = -1e100

    for val in data {
        if val < min_val { min_val = val }
        if val > max_val { max_val = val }
    }

    float64: range = max_val - min_val
    float64: bin_width = range / bins

    // Count values in each bin
    vec: counts = zeros(bins)

    for val in data {
        int: bin_idx = (val - min_val) / bin_width
        if bin_idx >= bins { bin_idx = bins - 1 }
        if bin_idx < 0 { bin_idx = 0 }
        counts[bin_idx] = counts[bin_idx] + 1
    }

    // Find max count for scaling
    float64: max_count = 0
    for count in counts {
        if count > max_count { max_count = count }
    }

    // Print histogram
    println("=== Histogram ===")
    for i in range(bins) {
        float64: bin_start = min_val + i * bin_width
        float64: bin_end = bin_start + bin_width
        int: bar_length = counts[i] / max_count * 50  // Scale to 50 chars

        print("[", bin_start, "-", bin_end, "]: ")
        for j in range(bar_length) {
            print("*")
        }
        println(" (", counts[i], ")")
    }
}

text_histogram(y_data, 10)
```

## Machine Learning Basics

### K-Means Clustering (Simple Implementation)

```rhodesia
fun kmeans(vec: data, int: k, int: max_iters) -> vec {
    int: n = size(data)

    // Initialize centroids randomly
    vec: centroids = zeros(k)
    for i in range(k) {
        int: rand_idx = rand() * n
        centroids[i] = data[rand_idx]
    }

    vec: labels = zeros(n)

    for iter in range(max_iters) {
        // Assign points to nearest centroid
        for i in range(n) {
            float64: min_dist = 1e100
            int: best_cluster = 0

            for j in range(k) {
                float64: dist = abs(data[i] - centroids[j])
                if dist < min_dist {
                    min_dist = dist
                    best_cluster = j
                }
            }

            labels[i] = best_cluster
        }

        // Update centroids
        vec: cluster_sums = zeros(k)
        vec: cluster_counts = zeros(k)

        for i in range(n) {
            int: cluster = labels[i]
            cluster_sums[cluster] = cluster_sums[cluster] + data[i]
            cluster_counts[cluster] = cluster_counts[cluster] + 1
        }

        for j in range(k) {
            if cluster_counts[j] > 0 {
                centroids[j] = cluster_sums[j] / cluster_counts[j]
            }
        }
    }

    return labels
}

// Generate clustered data
vec: cluster1 = zeros(50) + 10 + randn(50) * 2
vec: cluster2 = zeros(50) + 30 + randn(50) * 2
vec: mixed_data = cluster1
// Note: In real implementation, you'd concatenate vectors

println("K-means clustering example would go here")
```

## Next Steps

- [Algorithms Tutorial](algorithms.md) - Advanced numerical algorithms
- [Machine Learning Examples](examples/machine-learning.md) - Complete ML implementations
- [Performance Guide](api/performance.md) - Optimization techniques

## Key Takeaways

1. Rhodesia provides efficient vectorized operations for data science
2. Statistical functions are built-in and optimized
3. Linear algebra operations enable machine learning implementations
4. The language is designed for numerical computing workflows

Experiment with the code examples and try implementing your own data analysis functions!
