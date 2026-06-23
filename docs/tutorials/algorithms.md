---
title: Algorithms
layout: default
nav_order: 3
parent: Tutorials
---

# Algorithms Tutorial

This tutorial covers implementing numerical algorithms in Rhodesia, focusing on performance and correctness.

## Numerical Integration

### Trapezoidal Rule

```rhodesia
fun trapezoidal_rule(fun: f, float64: a, float64: b, int: n) -> float64 {
    float64: h = (b - a) / n
    float64: sum = f(a) + f(b)

    for i in range(1, n) {
        float64: x = a + i * h
        sum = sum + 2 * f(x)
    }

    return sum * h / 2
}

// Test function: integrate x^2 from 0 to 1
fun test_func(float64: x) -> float64 {
    return x * x
}

float64: result = trapezoidal_rule(test_func, 0.0, 1.0, 1000)
println("∫x² dx from 0 to 1 =", result)  // Should be ≈ 0.333
```

### Simpson's Rule

```rhodesia
fun simpsons_rule(fun: f, float64: a, float64: b, int: n) -> float64 {
    if n % 2 != 0 {
        n = n + 1  // Must be even
    }

    float64: h = (b - a) / n
    float64: sum = f(a) + f(b)

    for i in range(1, n) {
        float64: x = a + i * h
        if i % 2 == 0 {
            sum = sum + 2 * f(x)
        } else {
            sum = sum + 4 * f(x)
        }
    }

    return sum * h / 3
}

float64: simpson_result = simpsons_rule(test_func, 0.0, 1.0, 1000)
println("Simpson's rule result:", simpson_result)
```

## Root Finding

### Bisection Method

```rhodesia
fun bisection(fun: f, float64: a, float64: b, float64: tol, int: max_iter) -> float64 {
    if f(a) * f(b) >= 0 {
        println("Error: Function must have opposite signs at endpoints")
        return 0.0
    }

    for iter in range(max_iter) {
        float64: c = (a + b) / 2
        float64: fc = f(c)

        if abs(fc) < tol {
            return c
        }

        if f(a) * fc < 0 {
            b = c
        } else {
            a = c
        }
    }

    return (a + b) / 2
}

// Find root of x^2 - 2 = 0 (sqrt(2) ≈ 1.414)
fun quadratic(float64: x) -> float64 {
    return x * x - 2
}

float64: root = bisection(quadratic, 1.0, 2.0, 1e-10, 100)
println("Root of x²-2=0:", root)
println("Verification:", quadratic(root))
```

### Newton-Raphson Method

```rhodesia
fun newton_raphson(fun: f, fun: df, float64: x0, float64: tol, int: max_iter) -> float64 {
    float64: x = x0

    for iter in range(max_iter) {
        float64: fx = f(x)
        float64: dfx = df(x)

        if abs(dfx) < 1e-10 {
            println("Error: Derivative too small")
            return x
        }

        float64: x_new = x - fx / dfx

        if abs(x_new - x) < tol {
            return x_new
        }

        x = x_new
    }

    return x
}

// Derivative of x^2 - 2
fun d_quadratic(float64: x) -> float64 {
    return 2 * x
}

float64: nr_root = newton_raphson(quadratic, d_quadratic, 2.0, 1e-10, 100)
println("Newton-Raphson root:", nr_root)
```

## Matrix Algorithms

### Gaussian Elimination

```rhodesia
fun gaussian_elimination(mat: A, vec: b) -> vec {
    int: n = rows(A)
    mat: augmented = zeros(n, n+1)

    // Create augmented matrix [A|b]
    for i in range(n) {
        for j in range(n) {
            augmented[i, j] = A[i, j]
        }
        augmented[i, n] = b[i]
    }

    // Forward elimination
    for i in range(n) {
        // Find pivot
        int: max_row = i
        for k in range(i+1, n) {
            if abs(augmented[k, i]) > abs(augmented[max_row, i]) {
                max_row = k
            }
        }

        // Swap rows
        if max_row != i {
            for j in range(n+1) {
                float64: temp = augmented[i, j]
                augmented[i, j] = augmented[max_row, j]
                augmented[max_row, j] = temp
            }
        }

        // Eliminate
        for k in range(i+1, n) {
            float64: factor = augmented[k, i] / augmented[i, i]
            for j in range(i, n+1) {
                augmented[k, j] = augmented[k, j] - factor * augmented[i, j]
            }
        }
    }

    // Back substitution
    vec: x = zeros(n)
    for i in range(n-1, -1, -1) {
        x[i] = augmented[i, n]
        for j in range(i+1, n) {
            x[i] = x[i] - augmented[i, j] * x[j]
        }
        x[i] = x[i] / augmented[i, i]
    }

    return x
}

// Test: Solve Ax = b where A is 3x3 identity, b = [1,2,3]
mat: A_test = eye(3)
vec: b_test = [1, 2, 3]
vec: solution = gaussian_elimination(A_test, b_test)
println("Solution:", solution)
```

## Optimization

### Gradient Descent

```rhodesia
fun gradient_descent(fun: f, fun: grad_f, vec: x0, float64: alpha, float64: tol, int: max_iter) -> vec {
    vec: x = x0

    for iter in range(max_iter) {
        vec: grad = grad_f(x)
        float64: grad_norm = norm(grad)

        if grad_norm < tol {
            break
        }

        x = x - alpha * grad
    }

    return x
}

// Minimize f(x,y) = (x-1)² + (y-2)²
fun quadratic_2d(vec: point) -> float64 {
    float64: x = point[0]
    float64: y = point[1]
    return (x - 1) * (x - 1) + (y - 2) * (y - 2)
}

fun grad_quadratic_2d(vec: point) -> vec {
    float64: x = point[0]
    float64: y = point[1]
    vec: gradient = [(x - 1) * 2, (y - 2) * 2]
    return gradient
}

vec: start = [10, 10]
vec: minimum = gradient_descent(quadratic_2d, grad_quadratic_2d, start, 0.1, 1e-6, 1000)
println("Minimum found at:", minimum)
println("Function value:", quadratic_2d(minimum))
```

## Sorting Algorithms

### Quick Sort

```rhodesia
fun partition(vec: arr, int: low, int: high) -> int {
    float64: pivot = arr[high]
    int: i = low - 1

    for j in range(low, high) {
        if arr[j] < pivot {
            i = i + 1
            // Swap arr[i] and arr[j]
            float64: temp = arr[i]
            arr[i] = arr[j]
            arr[j] = temp
        }
    }

    // Swap arr[i+1] and arr[high]
    float64: temp = arr[i+1]
    arr[i+1] = arr[high]
    arr[high] = temp

    return i + 1
}

fun quick_sort(vec: arr, int: low, int: high) -> void {
    if low < high {
        int: pi = partition(arr, low, high)

        quick_sort(arr, low, pi - 1)
        quick_sort(arr, pi + 1, high)
    }
}

// Usage
vec: data = [64, 34, 25, 12, 22, 11, 90]
quick_sort(data, 0, size(data) - 1)
println("Sorted array:", data)
```

## Performance Considerations

### Vectorization Benefits

```rhodesia
// Inefficient element-wise operations
fun slow_sum(vec: data) -> float64 {
    float64: total = 0.0
    for val in data {
        total = total + val
    }
    return total
}

// Efficient vectorized operations
fun fast_sum(vec: data) -> float64 {
    return sum(data)  // Uses optimized Eigen operations
}

// Benchmark
vec: large_data = range(100000)
println("Vectorized sum is much faster than manual loops!")
```

## Next Steps

- [Performance Guide](api/performance.md) - Optimization techniques
- [Examples](examples/basics.md) - More algorithm implementations
- [API Architecture](api/architecture.md) - System internals

## Algorithm Complexity

| Algorithm | Time Complexity | Space Complexity |
|-----------|----------------|------------------|
| Bisection | O(max_iter) | O(1) |
| Newton-Raphson | O(max_iter) | O(1) |
| Gaussian Elimination | O(n³) | O(n²) |
| Gradient Descent | O(max_iter × d) | O(d) |
| Quick Sort | O(n log n) avg | O(log n) |

Experiment with these algorithms and try implementing your own numerical methods!
