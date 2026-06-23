---
title: Functions & Lambdas
layout: default
nav_order: 4
parent: Language Reference
---

# Functions & Lambdas

Functions are first-class in Rhodesia. A named function is declared with
`fun`; an anonymous function is a lambda declared with `lambda` (short alias
`fn`).

## Named Functions

```rhodesia
fun name(type1: p1, type2: p2, ...) -> return_type {
    // body
    return value
}
```

### Basic Examples

```rhodesia
fun square(float64: x) -> float64 {
    return x * x
}

fun greet(string: name) -> void {
    println("Hello,", name, "!")
}
```

### Recursion

```rhodesia
fun factorial(int: n) -> int {
    if n <= 1 {
        return 1
    }
    return n * factorial(n - 1)
}
```

### Multiple Return Values

Use a `tuple` to return more than one value:

```rhodesia
fun min_max(vec: v) -> tuple {
    float64: lo = v[0]
    float64: hi = v[0]
    for x in v {
        if x < lo { lo = x }
        if x > hi { hi = x }
    }
    return (lo, hi)
}

tuple: bounds = min_max([3.0, 1.0, 9.0, 2.0])
float64: lo = bounds[0]
float64: hi = bounds[1]
```

### Functions Taking Functions

Function types are first-class. Declare a parameter as `function`:

```rhodesia
fun apply(float64: x, function: f) -> float64 {
    return f(x)
}

fun double(float64: x) -> float64 { return 2.0 * x }

float64: r = apply(3.0, double)        // 6.0
```

## Lambdas

```rhodesia
lambda(params) -> expr      // expression body
fn(params) -> expr           // short alias
lambda(params) { block }     // block body
```

```rhodesia
// As an expression
float64: r = apply(3.0, lambda(x) -> x * x)   // 9.0

// Stored in a variable
function: sq = lambda(float64: x) -> x * x

// Block body
function: noisy = lambda(string: msg) {
    println("[log]", msg)
    return msg
}
```

See [Closures & Higher-Order](lambdas-closures.md) for capturing, function
factories, and the built-in `map` / `filter` / `compose` helpers.

## Function Calls

```rhodesia
float64: result = square(5.0)
greet("World")
```

Arguments are evaluated left-to-right. Recursion is fully supported but
not tail-call-optimized — very deep recursion may exhaust the stack.

## Naming Conflicts

If a parameter or local variable shadows a function name, the local binding
takes precedence inside its scope. Rename to avoid ambiguity.