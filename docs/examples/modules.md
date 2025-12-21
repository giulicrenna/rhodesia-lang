# Modules

Rhodesia supports a modular system that allows you to organize code into reusable components. This page demonstrates how to create and use modules.

## Basic Module Usage

### Creating a Module

Create a file `math_utils.rho`:

```rhodesia
// math_utils.rho - Mathematical utility functions
fun square(float64: x) -> float64 {
    return x * x
}

fun cube(float64: x) -> float64 {
    return x * x * x
}

fun average(vec: data) -> float64 {
    return sum(data) / size(data)
}
```

### Using a Module

Create a file `main.rho`:

```rhodesia
// main.rho - Main program using the math_utils module
include math_utils{square, cube, average}

float64: num = 4.0
vec: data = [1, 2, 3, 4, 5]

println("square(4) =", square(num))
println("cube(4) =", cube(num))
println("average([1,2,3,4,5]) =", average(data))
```

## Import Variations

### Selective Import

Import only specific functions:

```rhodesia
include math_utils{square, average}
// Now only square and average are available
```

### Import All

Import everything from a module:

```rhodesia
include math_utils{}
// All functions from math_utils are now available
```

### Import with Aliases

Rename functions during import:

```rhodesia
include math_utils{square as sq, cube as cb}
// Use sq() instead of square(), cb() instead of cube()
```

## Module Organization

### Directory Structure

```
my_project/
├── main.rho
├── math_utils.rho
└── stats/
    └── analysis.rho
```

### Using Subdirectories

```rhodesia
// From main.rho
include math_utils{square}           // ./math_utils.rho
include stats/analysis{mean}         // ./stats/analysis.rho
```

## Advanced Examples

### Multiple Modules

**vector_ops.rho:**
```rhodesia
fun normalize(vec: v) -> vec {
    float64: n = norm(v)
    return v / n
}

fun magnitude(vec: v) -> float64 {
    return norm(v)
}
```

**matrix_ops.rho:**
```rhodesia
fun is_square(mat: m) -> int {
    return rows(m) == cols(m)
}

fun trace(mat: m) -> float64 {
    if not is_square(m) {
        return 0.0  // Error case
    }

    float64: sum = 0.0
    for i in range(rows(m)) {
        sum = sum + m[i, i]
    }
    return sum
}
```

**combined_demo.rho:**
```rhodesia
include vector_ops{normalize, magnitude}
include matrix_ops{trace}

vec: v = [3, 4]
mat: m = [[1, 2], [3, 4]]

println("Vector:", v)
println("Magnitude:", magnitude(v))
println("Normalized:", normalize(v))

println("Matrix:")
println(m)
println("Trace:", trace(m))
```

### Avoiding Name Conflicts

**module_a.rho:**
```rhodesia
fun process(float64: x) -> float64 {
    return x * 2.0
}
```

**module_b.rho:**
```rhodesia
fun process(float64: x) -> float64 {
    return x + 10.0
}
```

**conflict_resolution.rho:**
```rhodesia
include module_a{process as double}
include module_b{process as add_ten}

float64: num = 5.0
println("Double:", double(num))      // 10.0
println("Add ten:", add_ten(num))    // 15.0
```

## Best Practices

1. **Organize by functionality**: Group related functions in the same module
2. **Use descriptive names**: Make module and function names clear
3. **Import selectively**: Only import what you need to avoid namespace pollution
4. **Document your modules**: Add comments explaining what each function does

## Complete Project Example

```
statistics_project/
├── main.rho
├── data/
│   ├── loader.rho
│   └── cleaner.rho
├── stats/
│   ├── basic.rho
│   └── advanced.rho
└── utils/
    └── helpers.rho
```

**main.rho:**
```rhodesia
include data/loader{load_csv}
include stats/basic{mean, std_dev}
include utils/helpers{print_summary}

vec: data = load_csv("data.csv")
println("Data loaded, size:", size(data))

print_summary(data)
println("Mean:", mean(data))
println("Standard deviation:", std_dev(data))
```

## Next Steps

- [Language Syntax](language/syntax.md) - Core language features
- [Standard Library](standard-library/functions.md) - Built-in functions
- [Examples](basics.md) - Basic usage examples
