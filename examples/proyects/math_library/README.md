# Rhodesia Math Library

A comprehensive modular math library for scientific computing in Rhodesia language.

## Overview

The Rhodesia Math Library is a complete, modular mathematical framework that demonstrates the power of Rhodesia's module system. It provides a wide range of mathematical functions organized into logical modules for easy integration into scientific and engineering applications.

## Project Structure

```
math_library/
├── core/                  # Core mathematical functions and utilities
│   ├── core.rho           # Basic math operations, constants, utilities
│   └── trigonometry.rho   # Trigonometric functions and angle conversions
├── linear_algebra/        # Advanced linear algebra operations
│   ├── vectors.rho        # Vector operations beyond built-ins
│   ├── matrices.rho       # Matrix operations beyond built-ins
│   └── decompositions.rho # Matrix decompositions (LU, QR, Cholesky, etc.)
├── statistics/            # Statistical analysis functions
│   ├── descriptive.rho    # Descriptive statistics (mean, std dev, etc.)
│   ├── probability.rho    # Probability distributions (normal, binomial, etc.)
│   └── regression.rho     # Regression analysis (linear, polynomial, etc.)
├── numerical/             # Numerical methods for scientific computing
│   ├── root_finding.rho   # Root finding algorithms (Newton, bisection, etc.)
│   ├── optimization.rho   # Optimization algorithms (gradient descent, etc.)
│   └── integration.rho    # Numerical integration methods (Simpson, Monte Carlo, etc.)
├── main.rho               # Comprehensive demonstration program
└── README.md              # Project documentation
```

## Features

### Core Mathematics
- Mathematical constants (π, e, golden ratio, etc.)
- Basic arithmetic with error handling
- Power functions and polynomial operations
- Interpolation and range mapping
- Vector and matrix utility functions

### Trigonometry
- Angle conversions (degrees ↔ radians)
- Trigonometric functions with Taylor series approximations
- Inverse trigonometric functions
- Hyperbolic functions
- Angle wrapping and normalization

### Linear Algebra
- **Vector Operations**: distance, projection, angle calculation, rotation, cross products
- **Matrix Operations**: trace, determinant, transpose, norms, matrix properties
- **Decompositions**: LU, QR, Cholesky decompositions
- **Eigenvalue Estimation**: for 2x2 and 3x3 matrices
- **Matrix Analysis**: condition number, rank estimation, null space

### Statistics
- **Descriptive Statistics**: mean, variance, standard deviation, median, mode, skewness, kurtosis
- **Probability Distributions**: normal, uniform, exponential, Poisson, binomial, chi-squared, t-distribution
- **Regression Analysis**: linear, polynomial, weighted, ridge, lasso regression
- **Statistical Tests**: correlation, covariance, R-squared, error metrics (MSE, RMSE, MAE, MAPE)
- **Data Normalization**: z-score, min-max normalization

### Numerical Methods
- **Root Finding**: bisection, Newton-Raphson, secant, fixed-point, false position, Brent's method
- **Optimization**: golden section search, gradient descent, conjugate gradient, Newton's method, BFGS, Nelder-Mead, simulated annealing, genetic algorithms
- **Integration**: rectangle rules, midpoint rule, trapezoidal rule, Simpson's rule, adaptive quadrature, Romberg integration, Gaussian quadrature, Monte Carlo integration

## Usage Examples

### Importing Modules

```rhodesia
// Import specific functions from modules
include core/core{PI, power, clamp}
include linear_algebra/vectors{vec_distance, vec_normalize_safe}
include statistics/descriptive{mean, std_dev}
```

### Basic Operations

```rhodesia
// Core math
float64: result = power(2.0, 3)  // 8.0
vec: clamped = clamp([1.0, 5.0, 10.0], 2.0, 8.0)  // [2.0, 5.0, 8.0]

// Trigonometry
float64: angle = degrees_to_radians(45.0)
float64: sine = sin_taylor(angle, 10)

// Linear algebra
vec: v1 = [1.0, 2.0, 3.0]
vec: v2 = [4.0, 5.0, 6.0]
float64: dist = vec_distance(v1, v2)

// Statistics
vec: data = [1.0, 2.0, 3.0, 4.0, 5.0]
float64: avg = mean(data)
float64: std = std_dev(data)
```

### Advanced Applications

```rhodesia
// Matrix decomposition
mat: A = [[1.0, 2.0], [3.0, 4.0]]
(mat: L, mat: U) = lu_decompose_3x3(A)

// Regression analysis
vec: x = [1.0, 2.0, 3.0, 4.0, 5.0]
vec: y = [2.0, 4.0, 5.0, 4.0, 5.0]
(float64: a, float64: b) = linear_regression(x, y)

// Optimization
fun objective(float64: x) -> float64 {
    return x * x + 2.0 * x + 1.0
}
float64: minimum = golden_section_search(objective, -2.0, 0.0, 0.0001, 100)

// Integration
fun integrand(float64: x) -> float64 {
    return x * x
}
float64: integral = simpsons_rule(integrand, 0.0, 1.0, 100)
```

## Running the Demo

To see all modules in action, run the main demonstration:

```bash
./rhodesia examples/proyects/math_library/main.rho
```

## Key Design Principles

1. **Modularity**: Each mathematical domain is separated into its own module
2. **Comprehensive Coverage**: From basic math to advanced numerical methods
3. **Practical Focus**: Real-world applications and use cases
4. **Interoperability**: Modules designed to work together seamlessly
5. **Documentation**: Clear examples and usage patterns

## Performance Considerations

- The library uses Taylor series approximations for trigonometric functions
- Matrix operations are optimized for small matrices (2x2, 3x3)
- Numerical methods include adaptive algorithms for better accuracy
- Optimization algorithms support various convergence criteria

## Future Enhancements

- Additional matrix decomposition methods (SVD, eigenvalue algorithms)
- More probability distributions and statistical tests
- Advanced optimization techniques
- Machine learning algorithms
- Signal processing functions

## License

This math library is part of the Rhodesia language examples and is open source.
