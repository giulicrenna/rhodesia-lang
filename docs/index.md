---
title: Home
layout: default
nav_order: 1
---

# Rhodesia Language Documentation

A high-performance, strongly-typed programming language designed for **data
science, numerical computing, and machine learning**. Clean syntax on top of
[Eigen3](https://eigen.tuxfamily.org/) for vectorized linear algebra.

## Get Rhodesia

[Download Rhodesia 0.1.2 for Windows (.exe)](https://github.com/giulicrenna/rhodesia-lang/releases/latest/download/rhodesia-setup-0.1.2.exe){: .btn .btn-primary }

**Editor**

Install the **Rhodesia Language Support** extension by publisher
`GiulianoCrenna` from the VS Code Marketplace
([GiulianoCrenna.RhodesiaLanguage](https://marketplace.visualstudio.com/items?itemName=GiulianoCrenna.RhodesiaLanguage))
for syntax highlighting, snippets, and language tooling on `.rho` files.

See [Installation & Editor Setup](installation.md) for full instructions,
verification steps, and offline VSIX install.

## What's Inside

- [Getting Started](getting-started.md) — first program, REPL, build from source.
- [Installation & Editor Setup](installation.md) — Windows installer + VS Code extension.
- [Language Reference](language/index.md) — syntax, types, control flow, functions, lambdas, modules, exceptions, etc.
- [Standard Library](standard-library/index.md) — math, stats, vector, matrix, string, mapping, array, datetime, io, net.
- [Examples](examples/basics.md) — basics, data structures, collections, I/O, ML, modules.
- [Tutorials](tutorials/basics.md) — guided walkthroughs + REPL + build.
- [API & Architecture](api/architecture.md) — internals, technical reference, performance.
- [Troubleshooting](troubleshooting/errors.md) — errors and debugging.

## Quick Example

```rhodesia
// Basic Rhodesia program
println("Hello, Rhodesia!")

// Vector operations
vec: u = [1, 2, 3]
vec: v = [4, 5, 6]
float64: dot_product = math.dot(u, v)
println("Dot product:", dot_product)

// Matrix operations
mat: A = [[1, 2], [3, 4]]
mat: B = [[5, 6], [7, 8]]
mat: C = A * B
println("Matrix multiplication:")
println(C)
```

## Features

- **Vectorized by default** — `vec` and `mat` are Eigen3-typed; loops compile down to SIMD-friendly ops.
- **Strongly typed** — explicit `type: name = value` declarations for every primitive and collection.
- **Standard library** — math, stats, numerical, vector, matrix, string, mapping, array, datetime, io, net.
- **Networking built-in** — TCP sockets and HTTP/1.1 client via the `net` module, zero extra dependencies.
- **Module system** — `include mod{symbols}` with selective imports and aliases; user libraries discovered via `info.json`.
- **REPL** — interactive mode with `help`, `vars`, multi-line input via unbalanced braces.
- **Bytecode VM** — `--vm` flag runs the same code through a small bytecode VM (faster than the tree-walking interpreter).

## Community

- **GitHub**: [https://github.com/giulicrenna/rhodesia-lang](https://github.com/giulicrenna/rhodesia-lang)
- **Issues**: [report a bug or request a feature](https://github.com/giulicrenna/rhodesia-lang/issues)
- **Discussions**: [Q&A and ideas](https://github.com/giulicrenna/rhodesia-lang/discussions)
- **Contributions**: pull requests welcome.

---

© 2025 Rhodesia Language Project — MIT License.