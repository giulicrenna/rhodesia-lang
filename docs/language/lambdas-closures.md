---
title: Closures & Higher-Order
layout: default
nav_order: 4
parent: Language Reference
---

# Closures & Higher-Order

Lambdas in Rhodesia are real closures: they capture the variables in scope
at the point of definition and carry that environment with them. This page
covers capturing, function factories, and the most common higher-order
patterns.

See [examples/05_functions](../examples/basics.md) for runnable samples.

## Closures Capture by Reference

```rhodesia
fun make_counter(int: start) -> function {
    int: count = start
    return lambda() -> {
        count = count + 1
        return count
    }
}

function: c = make_counter(10)
println(c())   // 11
println(c())   // 12
println(c())   // 13
```

Each call to `c()` increments the *same* `count` — the closure carries the
original binding, not a copy.

## Function Factories

Returning a function is a clean way to build specialized callables:

```rhodesia
fun multiplier(float64: k) -> function {
    return lambda(float64: x) -> k * x
}

function: triple = multiplier(3.0)
float64: r = triple(5.0)        // 15.0
```

## Higher-Order Helpers

Rhodesia ships a small set of higher-order helpers. They are particularly
useful with lambdas:

### `map(vec, function)` → `vec`

Apply `function` to each element of `vec`.

```rhodesia
vec: xs = [1.0, 2.0, 3.0]
vec: sq = map(xs, lambda(float64: x) -> x * x)
// sq == [1, 4, 9]
```

### `filter(vec, function)` → `vec`

Keep elements where `function` returns non-zero.

```rhodesia
vec: xs = [1, 2, 3, 4, 5]
vec: evens = filter(xs, lambda(int: x) -> x % 2 == 0)
// evens == [2, 4]
```

### `compose(f, g)` → `function`

Return a function that applies `g` then `f`.

```rhodesia
function: inc   = lambda(int: x) -> x + 1
function: dbl   = lambda(int: x) -> 2 * x
function: inc_then_dbl = compose(dbl, inc)

int: r = inc_then_dbl(3)   // (3 + 1) * 2 == 8
```

## Practical Pattern: Map + Filter

```rhodesia
vec: raw = [-2.0, -1.0, 0.0, 1.0, 2.0]

vec: positive = filter(raw, lambda(float64: x) -> x > 0)
// [1, 2]
vec: doubled = map(positive, lambda(float64: x) -> 2 * x)
// [2, 4]
```

## Storing Functions in Records

Records and arrays can hold functions as fields:

```rhodesia
record: handlers = {
    inc: lambda(int: x) -> x + 1,
    dec: lambda(int: x) -> x - 1
}

int: r = handlers.inc(5)   // 6
```

## Caveats

- Closures keep references to captured variables, not copies — be careful
  when capturing loop variables if you intend each iteration to have its
  own value.
- Lambdas do not capture the `return` of their enclosing function — a lambda
  `return`s into the lambda, not its surrounding `fun`.