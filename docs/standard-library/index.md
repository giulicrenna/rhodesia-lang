---
title: Standard Library
layout: default
nav_order: 5
has_children: true
---

# Standard Library

Rhodesia's standard library is organized as a set of **modules**, each
exposing a focused set of functions. Modules are accessed with the
`module.function` syntax — no `import` statement is required.

## Modules

| Module | Purpose |
|---|---|
| [`math`](math.md) | Elementwise math, trig, linear algebra basics |
| [`stats`](stats.md) | Mean, variance, moments, correlation |
| [`numerical`](numerical.md) | Integration, root-finding, polynomials |
| [`vector`](vector.md) | Immutable vector helpers |
| [`matrix`](matrix.md) | Immutable matrix helpers |
| [`string`](string.md) | String manipulation |
| [`mapping`](mapping.md) | Map / dictionary operations |
| [`array`](array.md) | Dynamic array helpers |
| [`datetime`](datetime.md) | Date / time / timestamp types |
| [`io`](io.md) | File I/O and user input |
| [`net`](net.md) | TCP sockets and HTTP/1.1 client |

A second-tier user-space library — [`libs/math`](libs-math.md) — bundles
~150 functions for linear algebra, numerical methods, statistics, and
regression. Import it with `include math`.

## Top-level Built-ins

A handful of functions live at the top level (no module prefix):

- `print(...)` — variadic, no newline.
- `println(...)` — variadic, with newline.
- `get_tick()` — monotonic high-precision timestamp.
- `make_complex(real, imag)`, `make_set(...)`, `make_tuple(...)`,
  `make_record(...)`.
- Free aliases of `math.*`: `size`, `rows`, `cols`, `sum`, `mean`, `norm`,
  `dot`, `transpose`, `inv`, `sqrt`, `abs`, `floor`, `ceil`, `round`,
  `min`, `max`, `range`, `zeros`, `ones`, `eye`, `clamp`.

See [Overview](functions.md) for the cross-module cheatsheet.

## Conventions

- **Functions are pure unless documented otherwise.** `vector.append`
  returns a new vector rather than mutating in place.
- **Errors are raised, not returned.** Functions that can fail throw
  exceptions; wrap the call in `try` / `catch` if you need recovery (see
  [Exceptions](../language/exceptions.md)).
- **Numeric results are `float64` by default.** Integer inputs are
  promoted automatically.