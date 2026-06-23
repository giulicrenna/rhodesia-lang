---
title: API & Architecture
layout: default
nav_order: 8
has_children: true
---

# API & Architecture

Technical reference for the Rhodesia implementation.

## Sections

1. [Architecture](architecture.md) — overall pipeline (Lexer → Parser → AST → Evaluator / VM).
2. [Technical API](technical.md) — AST node types, `RhoValue` variant, type system.
3. [Performance](performance.md) — optimization tips, including the bytecode VM (`--vm`) flag.

For end-user documentation, see the [Language Reference](../language/index.md)
and the [Standard Library](../standard-library/index.md).