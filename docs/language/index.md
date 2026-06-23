---
title: Language Reference
layout: default
nav_order: 4
has_children: true
---

# Language Reference

The Rhodesia language: a strongly-typed, imperative language with first-class
support for vectors and matrices. This section walks through the language
from syntax to advanced features.

## In this section

1. [Syntax](syntax.md) — comments, literals, statement structure.
2. [Types](types.md) — primitive and collection types.
3. [Variables & Declarations](variables.md) — `type: name = value` and scoping.
4. [Operators](operators.md) — arithmetic, comparison, logical, **bitwise**, ternary, matrix/vector.
5. [Control Flow](control-flow.md) — `if`, `for`, `while`, `break`, `continue`.
6. [Functions & Lambdas](functions.md) — `fun`, parameters, return types.
7. [Closures & Higher-Order](lambdas-closures.md) — capturing, `map`/`filter`/`compose`.
8. [Exceptions](exceptions.md) — `try` / `catch` / `throw`.
9. [Using Blocks](using-blocks.md) — `using io.open(...) as f { ... }`.
10. [Collections](collections.md) — `set`, `tuple`, `record`, `map`, `array`.
11. [Match Statement](match.md) — pattern matching over a single value.
12. [Modules](modules.md) — `include`, aliases, `info.json` discovery.
13. [Indexing & Slicing](indexing-slicing.md) — `v[i]`, `v[i:j]`, `m[i,j]`, `m[i,:]`.

## Design principles

- **Explicit over implicit**: every binding carries its declared type; there
  is no type inference.
- **Vectorize by default**: `vec` and `mat` are first-class, and operators
  follow their mathematical meaning (`A * B` is matrix product, `2 * v`
  broadcasts).
- **Small surface, deep core**: a handful of language primitives cover the
  full numerical workflow; standard-library modules extend the runtime.