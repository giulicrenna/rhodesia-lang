---
title: Match Statement
layout: default
nav_order: 11
parent: Language Reference
---

# Match Statement

The `match` statement provides exhaustive, readable pattern matching against a value. It is the preferred alternative to long chains of `if / else if` when testing a single value against multiple concrete cases.

## Table of Contents

- [Syntax](#syntax)
- [Patterns](#patterns)
- [Wildcard](#wildcard)
- [Match Body](#match-body)
- [Nested Match](#nested-match)
- [Examples](#examples)
- [Match vs If-Else](#match-vs-if-else)

---

## Syntax

```rhodesia
match expression {
    pattern1 -> {
        // executed when expression == pattern1
    }
    pattern2 -> {
        // executed when expression == pattern2
    }
    _ -> {
        // default: executed when no other pattern matched
    }
}
```

- The **scrutinee** (`expression`) is evaluated once.
- Each **arm** (`pattern -> { body }`) is tested in order, top to bottom.
- The **first matching arm** executes; remaining arms are skipped.
- If no arm matches and there is no wildcard, the statement is a no-op.

---

## Patterns

A pattern can be any literal value or expression that evaluates to a comparable value:

| Pattern | Matches when scrutinee is equal to |
|---|---|
| Integer literal `42` | `int` or compatible numeric value `42` |
| Float literal `3.14` | `float64` value `3.14` |
| String literal `"ok"` | `str` value `"ok"` |
| Boolean `true` / `false` | `bool` value |
| `null` | a null value |
| Any expression | the evaluated result of that expression |

### Integer Patterns

```rhodesia
int: code = 404

match code {
    200 -> { println("OK") }
    201 -> { println("Created") }
    400 -> { println("Bad Request") }
    404 -> { println("Not Found") }
    500 -> { println("Internal Server Error") }
    _   -> { println("Unknown status") }
}
// Output: Not Found
```

### String Patterns

```rhodesia
str: day = "saturday"

match day {
    "monday"    -> { println("Start of the week") }
    "friday"    -> { println("End of work week") }
    "saturday"  -> { println("Weekend") }
    "sunday"    -> { println("Weekend") }
    _           -> { println("Regular work day") }
}
// Output: Weekend
```

### Boolean Patterns

```rhodesia
bool: passed = true

match passed {
    true  -> { println("Passed") }
    false -> { println("Failed") }
}
// Output: Passed
```

---

## Wildcard

The wildcard `_` matches any value and serves as the default case. It must be the **last arm** since it always matches:

```rhodesia
match status {
    "ok"    -> { println("success") }
    "error" -> { println("failure") }
    _       -> { println("unexpected:", status) }  // catches everything else
}
```

---

## Match Body

Each arm's body is a full block `{ ... }`. The body can contain any statements, including variable declarations, loops, function calls, and `return`:

```rhodesia
fun describe(int: n) -> str {
    match n {
        0 -> { return "zero" }
        1 -> { return "one" }
        _ -> {
            if n < 0 {
                return "negative"
            }
            return "positive"
        }
    }
    return ""
}
```

---

## Nested Match

Match statements can be nested to handle multi-dimensional dispatch:

```rhodesia
fun next_state(str: state, str: event) -> str {
    match state {
        "idle" -> {
            match event {
                "start" -> { return "running" }
                _       -> { return "idle" }
            }
            return "idle"
        }
        "running" -> {
            match event {
                "pause" -> { return "paused" }
                "stop"  -> { return "idle" }
                "error" -> { return "error" }
                _       -> { return "running" }
            }
            return "running"
        }
        _ -> { return state }
    }
    return state
}
```

---

## Examples

### HTTP Status Descriptions

```rhodesia
fun http_message(int: code) -> str {
    match code {
        200 -> { return "OK" }
        201 -> { return "Created" }
        204 -> { return "No Content" }
        301 -> { return "Moved Permanently" }
        400 -> { return "Bad Request" }
        401 -> { return "Unauthorized" }
        403 -> { return "Forbidden" }
        404 -> { return "Not Found" }
        500 -> { return "Internal Server Error" }
        503 -> { return "Service Unavailable" }
        _   -> { return "Unknown" }
    }
    return ""
}

println(http_message(200))  // OK
println(http_message(404))  // Not Found
println(http_message(418))  // Unknown
```

### Grade Classification

```rhodesia
fun classify(int: grade) -> str {
    match grade {
        10 -> { return "Excellent" }
        9  -> { return "Excellent" }
        8  -> { return "Good" }
        7  -> { return "Average" }
        6  -> { return "Passing" }
        5  -> { return "Borderline" }
        _  -> {
            if grade < 5 {
                return "Fail"
            }
            return "Invalid grade"
        }
    }
    return ""
}
```

### Finite State Machine

```rhodesia
// Traffic light controller
fun next_light(str: current) -> str {
    match current {
        "red"    -> { return "green" }
        "green"  -> { return "yellow" }
        "yellow" -> { return "red" }
        _        -> { return "red" }  // safe default
    }
    return "red"
}

str: light = "red"
for i in range(6) {
    println("Light:", light)
    light = next_light(light)
}
```

### Match with Record Fields

```rhodesia
record: op = { kind: "multiply", a: 6, b: 7 }

fun compute(record: operation) -> int {
    match operation.kind {
        "add"      -> { return operation.a + operation.b }
        "subtract" -> { return operation.a - operation.b }
        "multiply" -> { return operation.a * operation.b }
        _ -> {
            println("Unknown operation:", operation.kind)
            return 0
        }
    }
    return 0
}

println(compute(op))  // 42
```

---

## Match vs If-Else

Match is preferred when testing **one value against multiple concrete cases**. Use `if / else if` when conditions are complex Boolean expressions.

### Prefer match

```rhodesia
// Clear, one value, multiple cases
match status_code {
    200 -> { handle_ok() }
    404 -> { handle_not_found() }
    500 -> { handle_error() }
    _   -> { handle_unknown() }
}
```

### Prefer if-else

```rhodesia
// Complex conditions on different variables
if x > 0 and y > 0 {
    println("First quadrant")
} else if x < 0 and y > 0 {
    println("Second quadrant")
} else {
    println("Other")
}
```

---

## Next Steps

- [Control Flow](control-flow.md) — If-else, loops, break, continue
- [Collections](collections.md) — Set, Tuple, Record
- [Functions](syntax.md#functions) — Functions and return values
- [Examples — Match](../examples/collections.md#match-statement-examples) — More match patterns
