---
title: Exceptions
layout: default
nav_order: 4
parent: Language Reference
---

# Exceptions

Use `try` / `catch` / `throw` to handle runtime errors (division by zero,
out-of-bounds access, type errors, file-not-found, etc.) without aborting
the program.

## Table of Contents

- [Throwing](#throwing)
- [Catching](#catching)
- [Catch with a message](#catch-with-a-message)
- [Patterns and limits](#patterns-and-limits)

See [examples/08_advanced/exceptions.rho](https://github.com/giulicrenna/rhodesia-lang/blob/main/examples/08_advanced/exceptions.rho)
for runnable samples.

## Throwing

```rhodesia
fun divide(float64: a, float64: b) -> float64 {
    if b == 0 {
        throw "division by zero"
    }
    return a / b
}
```

`throw` accepts any expression — strings are the conventional choice, but
you can throw records, integers, or custom values.

## Catching

```rhodesia
try {
    float64: r = divide(10.0, 0.0)
    println(r)
} catch {
    println("failed")
}
```

If the body of `try` throws, control jumps to `catch`. After `catch`,
execution continues normally.

## Catch with a Message

Bind the thrown value to a name to inspect it:

```rhodesia
try {
    int: x = vec_at([1, 2, 3], 99)
} catch msg {
    println("Error:", msg)
}
```

`msg` is in scope only inside the `catch` block.

## Patterns and Limits

- **Try blocks can be nested.** An inner `catch` handles inner throws; if
  the inner block re-throws (or has no `catch`), the outer block sees it.
- **There is no `finally`** in this version. To run cleanup, put it after
  the `try / catch` pair, or use a [Using Block](using-blocks.md).
- **Throws are synchronous.** A throw crosses function boundaries until it
  reaches a `try / catch`.
- **Throw values can be any type**, but strings are conventional and produce
  readable errors.

## Example: Defensive Parsing

```rhodesia
fun parse_int(string: s) -> int {
    try {
        return string.to_int(s)
    } catch {
        return 0   // sensible default
    }
}
```

See also: [Errors](troubleshooting/errors.md), [Debugging](troubleshooting/debugging.md).