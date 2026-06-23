---
title: Variables & Declarations
layout: default
nav_order: 4
parent: Language Reference
---

# Variables & Declarations

Every value in Rhodesia is bound to a typed name with an explicit
declaration. There is no type inference — the type is part of the syntax.

## Declaration Syntax

```rhodesia
type: name = expression
```

Examples:

```rhodesia
int: count = 0
float64: pi = 3.14159
vec: data = [1.0, 2.0, 3.0]
mat: identity = [[1, 0], [0, 1]]
string: name = "Rhodesia"
bool: ready = true
```

## Rules

- **Begin** with a letter or underscore (`_`).
- **Continue** with letters, digits, and underscores.
- **Case-sensitive**: `Data` and `data` are different names.
- **Must be initialized** at declaration.
- **Must be declared before use**.

## Mutability

Variables can be reassigned after declaration. The new value must be
assignment-compatible with the declared type:

```rhodesia
int: counter = 0
counter = counter + 1   // OK: same type

// counter = "hello"     // TypeError at runtime
```

Indexed assignment modifies elements in place even when the container is
otherwise immutable:

```rhodesia
vec: v = [1.0, 2.0, 3.0]
v[0] = 99.0             // OK: in-place element update
```

See [Indexing & Slicing](indexing-slicing.md) for details.

## Scoping

Variables are block-scoped. A name is visible from its declaration to the
end of the enclosing block:

```rhodesia
int: outer = 1

if outer > 0 {
    int: inner = 2
    println(outer)   // 1 — outer is visible
    println(inner)   // 2 — inner is visible
}

// println(inner)    // Error: inner is out of scope
```

Inner scopes may shadow outer names; the inner binding takes precedence
inside the block but does not affect the outer value.

## Constants

The `const` keyword produces a binding that cannot be reassigned. Use it for
mathematical constants and configuration values:

```rhodesia
const: float64: PI = 3.14159
const: int: MAX_RETRIES = 3

// PI = 3.0   // Error: cannot reassign const
```

## Declaration Forms Summary

| Form | Meaning |
|---|---|
| `int: x = 5` | Mutable typed binding |
| `const: float64: PI = 3.14` | Immutable typed binding |
| `int: x` (no `= expr`) | Reserved for future declaration-only form; not yet supported |

See [Types](types.md) for the full list of supported primitive and
collection types.