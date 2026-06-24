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

### Default Parameter Values

Any parameter can carry a default expression with `=`. Defaults let
callers omit the argument and pick it up from the function declaration.

```rhodesia
fun greet(string: who, string: greeting = "Hello") -> string {
    return string.concat(greeting, ", ", who, "!")
}

greet("World")                // "Hello, World!"
greet("World", "Hi")          // "Hi, World!"     (positional override)
greet("World", greeting: "Hi") // "Hi, World!"   (keyword override)
```

Defaults can reference any earlier parameter or any visible binding:

```rhodesia
fun window(int: width, int: height, string: title = "Untitled",
            bool: resizable = true) -> string {
    return string.concat(title, " ", string.from(width), "x",
                         string.from(height))
}
```

### Keyword Arguments at the Call Site

Use `name: value` to bind an argument by name instead of position. Order
is then free, and required arguments can be skipped (provided a default
exists or the parameter is filled by another keyword argument).

```rhodesia
print(makeWindow(width: 1024, height: 768))                 // both keyword
print(makeWindow(800, 600, title: "App"))                   // mix
print(makeWindow(height: 768, width: 1024, resizable: false)) // reorder
```

**Rules:**
- Once a keyword argument appears, every later argument must also be
  keyword (`positional after keyword` is a parse error).
- Each parameter may be bound at most once.
- Keyword arguments work for user-defined `fun`, `lambda`, and for all
  registered builtins (`math.sqrt(x: 16)`, `string.upper(s: "hi")`,
  `vec.sum(v: data)`, `print(label: x)` if the builtin has `args`).

### Variadic Parameters (`*args`)

A trailing `*type: name` collects all remaining positional arguments
into a single parameter. When the type is `vec` (or numeric), the
parameter receives an `Eigen::VectorXd`; otherwise it receives an
`arr` (`RhoArray`) that can still be iterated with `for`.

```rhodesia
fun sumAll(int: first, *int: rest) -> int {
    int: total = first
    for n in rest { total = total + n }
    return total
}

sumAll(1, 2, 3, 4, 5)   // 15
sumAll(10)              // 10  (rest is empty)

fun joinAll(string: sep, *string: parts) -> string {
    return string.join(parts, sep)
}

joinAll(", ", "a", "b", "c")  // "a, b, c"
```

Variadic parameters can be combined with defaults (the variadic marker
must be the last parameter) and with keyword binding:

```rhodesia
sumAll(1, rest: [2, 3, 4])    // rest is keyword-bound to [2, 3, 4]
```

## Naming Conflicts

If a parameter or local variable shadows a function name, the local binding
takes precedence inside its scope. Rename to avoid ambiguity.