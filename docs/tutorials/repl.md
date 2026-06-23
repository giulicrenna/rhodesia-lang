---
title: REPL
layout: default
nav_order: 4
parent: Tutorials
---

# REPL

The interactive **R**ead–**E**val–**P**rint **L**oop is the fastest way to
explore Rhodesia. Start it with no arguments:

```bash
./build/rhodesia
```

(or `rhodesia` if you installed via the [Windows installer](../installation.md))

## Commands

Type any of these at the prompt:

| Command | Effect |
|---|---|
| `help` | Show available commands |
| `vars` | List all currently-bound variables and their types |
| `exit` / `quit` | Leave the REPL |
| `clear` | Clear the screen |

## Statements and Expressions

The REPL accepts both statements (variable declarations, function
definitions) and bare expressions. A bare expression prints its value:

```
> vec: v = [1, 2, 3]
> v
[1, 2, 3]
> math.sum(v)
6.0
```

## Multi-Line Input

The REPL detects unbalanced braces and continues reading until the block
is closed:

```
> fun square(float64: x) -> float64 {
...     return x * x
... }
> square(5.0)
25.0
```

## State Persists Between Lines

A variable declared on one line is visible on the next:

```
> int: counter = 0
> counter = counter + 1
> counter
1
> counter = counter + 1
> counter
2
```

`vars` lists everything in scope:

```
> vars
counter  : int
```

## Inspecting Types

```bash
> vec: u = [1, 2, 3]
> vec: w = [4, 5, 6]
> math.dot(u, w)
32.0
> math.inv([[1, 2], [3, 4]])
[[-2, 1], [1.5, -0.5]]
```

## Running `.rho` Files from the REPL

You cannot `include` an external file from inside the REPL — start the
interpreter on the file directly:

```bash
./build/rhodesia script.rho
```

## Tips

- Use **Up / Down arrows** to recall previous lines (terminal-dependent).
- **Ctrl+C** interrupts the current evaluation but keeps the REPL alive.
- **Ctrl+D** (Unix) / **Ctrl+Z, Enter** (Windows) leaves the REPL.
- Long expressions wrap freely; statements end at the newline unless the
  next line is required to close a block.

## See Also

- [Getting Started](../getting-started.md) — first program.
- [Language Syntax](../language/syntax.md) — declarations and statements.