---
title: Modules
layout: default
nav_order: 4
parent: Language Reference
---

# Modules

Rhodesia supports two kinds of code organization:

1. **Source files** — single `.rho` files that you include with `include`.
2. **User libraries** — folders with an `info.json` index that describe a
   package; included the same way but discovered automatically.

Both forms use the same `include` statement.

## Including a Source File

```rhodesia
include "math_utils.rho"          // by relative path
include "./lib/math_utils.rho"    // explicit relative path
include "math_utils.rho" as m     // alias the module to `m`
```

After `include "math_utils.rho"` the symbols defined in that file become
available as `math_utils.symbol_name` — or `m.symbol_name` if aliased.

## Selective Import

```rhodesia
include "math_utils.rho"{square, cube}
```

Only `square` and `cube` are imported. Other symbols remain hidden, which
avoids name conflicts.

## User Libraries (`libs/`)

A library is a folder with an `info.json` index. The bundled `libs/math/`
ships with Rhodesia and is imported with a single statement:

```rhodesia
include math
```

Functions are then namespaced: `math.sqrt`, `math.sin`, `math.vec_abs`,
`math.linear_regression`, etc.

The folder structure looks like:

```
libs/
  math/
    info.json     # index
    index.rho     # re-exports
    core/
    linear_algebra/
    numerical/
    statistics/
```

### `info.json`

The index file describes the library and its sub-modules. Example:

```json
{
  "name": "math",
  "version": "1.0.0",
  "description": "Numerical extensions for Rhodesia",
  "modules": {
    "core":          { "file": "core/core.rho" },
    "linear_algebra":{ "file": "linear_algebra/index.rho" },
    "numerical":     { "file": "numerical/index.rho" },
    "statistics":    { "file": "statistics/index.rho" }
  }
}
```

See [libs-math](../standard-library/libs-math.md) for the full surface area
of the bundled `math` library.

## Search Path

When you write `include "foo"` Rhodesia searches:

1. The directory of the current file.
2. The current working directory.
3. Paths in `RHO_PATH` (colon-separated).
4. `libs/` folders discovered relative to the language installation.

## Conflict Resolution

If two included modules expose the same symbol, qualify it with the module
name. To pick a winner explicitly, use selective import:

```rhodesia
include "stats_utils.rho"{mean}     // pick `mean` from stats_utils
include "vec_utils.rho"{sum}        // pick `sum` from vec_utils
```

## Examples

- [examples/06_modules/simple/](../examples/modules.md) — small files with
  `include`.
- [examples/06_modules/alias/](../examples/modules.md) — aliasing and
  selective import.
- [examples/10_applications/math_library/main.rho](https://github.com/giulicrenna/rhodesia-lang/blob/main/examples/10_applications/math_library/main.rho)
  — using `include math` on the bundled library.