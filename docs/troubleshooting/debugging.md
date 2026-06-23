---
title: Debugging
layout: default
nav_order: 2
parent: Troubleshooting
---

# Debugging Guide

This guide covers debugging techniques and common issues in Rhodesia programs.

## Common Error Types

### Syntax Errors

**Lexer Errors**: Invalid tokens or characters

```rhodesia
// Error: Invalid character
int: x = 5 @ 3

// Fix: Use valid operators
int: x = 5 + 3
```

**Parse Errors**: Incorrect syntax structure

```rhodesia
// Error: Missing parentheses or incorrect structure
if x > 0 print("positive")

// Fix: Proper if statement
if x > 0 {
    println("positive")
}
```

### Runtime Errors

**Type Errors**: Operations on incompatible types

```rhodesia
// Error: Can't add string and number
string: s = "hello"
int: result = s + 5

// Fix: Convert or use compatible types
string: s = "hello"
string: result = s + "5"
```

**Index Errors**: Accessing invalid array indices

```rhodesia
vec: v = [1, 2, 3]
// Error: Index 5 doesn't exist
float64: bad = v[5]

// Fix: Check bounds
if size(v) > 5 {
    float64: good = v[5]
}
```

### Logic Errors

**Off-by-one Errors**: Common in loops

```rhodesia
vec: data = range(10)  // [0, 1, 2, ..., 9]
// Error: Accesses index 10 which doesn't exist
for i in range(11) {
    println(data[i])
}

// Fix: Use correct range
for i in range(size(data)) {
    println(data[i])
}
```

## Debugging Techniques

### Print Debugging

Add print statements to trace execution:

```rhodesia
fun complex_function(vec: data) -> vec {
    println("Function called with data size:", size(data))

    // Process data
    vec: result = data * 2
    println("After multiplication:", result)

    // More processing
    result = result + 1
    println("After addition:", result)

    return result
}
```

### Variable Inspection

Check variable values at key points:

```rhodesia
vec: data = load_data()
println("Data loaded, size:", size(data))
println("First 5 elements:", data[0], data[1], data[2], data[3], data[4])

// Process data
vec: processed = transform(data)
println("After processing, size:", size(processed))
println("Min:", min(processed), "Max:", max(processed), "Mean:", mean(processed))
```

### Function Testing

Test functions with known inputs:

```rhodesia
fun add_numbers(float64: a, float64: b) -> float64 {
    return a + b
}

// Test the function
float64: test1 = add_numbers(2.0, 3.0)
println("add_numbers(2, 3) =", test1)  // Should be 5.0

float64: test2 = add_numbers(-1.0, 1.0)
println("add_numbers(-1, 1) =", test2)  // Should be 0.0
```

## Common Issues and Solutions

### 1. Variable Not Defined

**Error**: `RuntimeError: Variable 'x' not found`

**Cause**: Using a variable before declaration or out of scope

```rhodesia
// Error
println(x)
int: x = 5

// Fix
int: x = 5
println(x)
```

### 2. Type Mismatch

**Error**: `TypeError: Cannot operate on int and string`

**Cause**: Operations between incompatible types

```rhodesia
// Error
int: num = 5
string: text = "hello"
int: bad = num + text

// Fix: Convert types
string: result = string(num) + text
```

### 3. Division by Zero

**Error**: `RuntimeError: Division by zero`

**Cause**: Dividing by zero or very small numbers

```rhodesia
// Error
float64: bad = 1.0 / 0.0

// Fix: Check denominator
float64: denominator = compute_value()
if abs(denominator) < 1e-10 {
    println("Error: Division by near-zero")
} else {
    float64: good = 1.0 / denominator
}
```

### 4. Matrix Dimension Mismatch

**Error**: `RuntimeError: Matrix dimensions don't match`

**Cause**: Matrix operations with incompatible sizes

```rhodesia
// Error: Can't multiply 2x3 and 2x2 matrices
mat: A = zeros(2, 3)  // 2x3
mat: B = zeros(2, 2)  // 2x2
mat: bad = A * B

// Fix: Check dimensions or use compatible matrices
mat: C = zeros(3, 2)  // 3x2
mat: good = A * C     // 2x3 * 3x2 = 2x2
```

### 5. Index Out of Bounds

**Error**: `RuntimeError: Index out of bounds`

**Cause**: Accessing array elements beyond valid range

```rhodesia
vec: v = [1, 2, 3]
// Error: Valid indices are 0, 1, 2
float64: bad = v[5]

// Fix: Check bounds
int: index = 5
if index >= 0 and index < size(v) {
    float64: good = v[index]
} else {
    println("Index out of bounds")
}
```

## Advanced Debugging

### Binary Search for Bugs

Isolate the problematic section:

```rhodesia
fun buggy_function(vec: data) -> vec {
    println("Function start")

    // Comment out sections to isolate the bug
    vec: step1 = data * 2
    println("After step 1")

    // vec: step2 = step1 + 1
    // println("After step 2")

    vec: step3 = step1 / 2
    println("After step 3")

    return step3
}
```

### Assertion Checks

Add runtime checks:

```rhodesia
fun safe_divide(float64: a, float64: b) -> float64 {
    // Assertion: denominator should not be zero
    if abs(b) < 1e-10 {
        println("Error: Division by zero or near-zero")
        return 0.0
    }
    return a / b
}

fun process_data(vec: data) -> vec {
    // Assertion: data should not be empty
    if size(data) == 0 {
        println("Error: Empty data vector")
        return []
    }

    vec: result = zeros(size(data))
    for i in range(size(data)) {
        result[i] = safe_divide(data[i], data[i-1])
    }

    return result
}
```

## REPL Debugging

Use the interactive REPL to test code snippets:

```bash
./rhodesia

# Test expressions
> 2 + 3
5

# Test functions
> fun test(x) { return x * 2 }
> test(5)
10

# Inspect variables
> vec: data = [1, 2, 3, 4, 5]
> mean(data)
3.0

# Exit REPL
> exit
```

## Performance Debugging

### Timing Code Sections

```rhodesia
fun time_operation(fun: operation, string: description) -> void {
    // Note: Rhodesia doesn't have built-in timing
    // Use external timing or count iterations

    println("Starting:", description)
    int: start_marker = 0  // Conceptual

    operation()

    int: end_marker = 0  // Conceptual
    println("Completed:", description)
}

fun slow_operation() -> void {
    vec: data = zeros(100000)
    for i in range(100000) {
        data[i] = i * i
    }
}

time_operation(slow_operation, "Large vector processing")
```

### Memory Debugging

Monitor memory usage patterns:

```rhodesia
fun memory_test() -> void {
    println("Before allocation")
    vec: large_vec = zeros(1000000)
    println("After large vector allocation")

    mat: large_mat = zeros(1000, 1000)
    println("After large matrix allocation")

    // Operations
    large_vec = large_vec * large_vec
    println("After vector operations")

    large_mat = large_mat * large_mat
    println("After matrix operations")

    // Cleanup happens automatically
    println("End of function")
}
```

## Best Practices

### 1. Write Test Cases

```rhodesia
fun test_add_function() -> void {
    // Test normal cases
    assert_equal(add(2, 3), 5, "Basic addition")

    // Test edge cases
    assert_equal(add(0, 0), 0, "Zero addition")
    assert_equal(add(-1, 1), 0, "Negative addition")

    println("All tests passed!")
}

fun assert_equal(actual, expected, message) -> void {
    if actual != expected {
        println("FAIL:", message)
        println("Expected:", expected, "Got:", actual)
    }
}
```

### 2. Use Meaningful Variable Names

```rhodesia
// Bad
vec: d = load("data.csv")
float64: m = mean(d)
vec: r = d - m

// Good
vec: data = load_data("data.csv")
float64: data_mean = mean(data)
vec: centered_data = data - data_mean
```

### 3. Add Error Handling

```rhodesia
fun safe_matrix_inverse(mat: A) -> mat {
    // Check if matrix is square
    if rows(A) != cols(A) {
        println("Error: Matrix must be square for inversion")
        return zeros(rows(A), cols(A))
    }

    // Check if matrix is invertible (simplified check)
    float64: det = determinant(A)  // Assuming we have this function
    if abs(det) < 1e-10 {
        println("Warning: Matrix is near singular")
    }

    return inv(A)
}
```

### 4. Document Assumptions

```rhodesia
fun process_sensor_data(vec: readings) -> vec {
    // Assumptions:
    // - readings is not empty
    // - all values are valid (not NaN or infinite)
    // - readings are in chronological order

    if size(readings) == 0 {
        println("Error: No sensor readings provided")
        return []
    }

    // Processing logic
    vec: filtered = apply_filter(readings)
    vec: normalized = (filtered - mean(filtered)) / std_dev(filtered)

    return normalized
}
```

## Getting Help

### Error Messages

Pay attention to error locations and messages:

```
RuntimeError at line 15, column 8: Variable 'undefined_var' not found
```

This tells you exactly where the error occurred.

### Community Resources

- Check existing examples in the `examples/` directory
- Review the language documentation
- Test code in small pieces using the REPL

## Next Steps

- [Error Reference](errors.md) - Complete list of error types
- [Performance Guide](../api/performance.md) - Optimization techniques
- [Examples](../examples/basics.md) - Working code examples
