---
title: Installation & Editor Setup
layout: default
nav_order: 3
---

# Installation & Editor Setup

Get up and running with Rhodesia in under two minutes.

## Download for Windows

[Download Rhodesia 0.1.2 for Windows (.exe)](https://github.com/giulicrenna/rhodesia-lang/releases/latest/download/rhodesia-setup-0.1.2.exe){: .btn .btn-primary }

The Windows installer (`rhodesia-setup-0.1.2.exe`) bundles the `rhodesia`
interpreter, the Eigen3-backed runtime, and adds `rhodesia` to your PATH so
the REPL and CLI work from any terminal.

> Source builds are still supported — see [Getting Started](getting-started.md)
> for Linux, macOS, and from-source instructions.

## Install the VS Code extension

1. Open Visual Studio Code.
2. Open the Extensions sidebar (`Ctrl+Shift+X` on Windows / Linux,
   `Cmd+Shift+X` on macOS).
3. Search for **"Rhodesia Language Support"** or simply **"Rhodesia"**.
4. Install the extension published by **GiulianoCrenna**
   (Marketplace ID: `GiulianoCrenna.RhodesiaLanguage`).
5. Open any `.rho` file — syntax highlighting activates automatically.

Marketplace link:
<https://marketplace.visualstudio.com/items?itemName=GiulianoCrenna.RhodesiaLanguage>

### Install from VSIX (offline / air-gapped)

A prebuilt VSIX ships with each release:

```bash
code --install-extension extension/RhodesiaLanguage-0.1.2.vsix
```

## Verify

```bash
rhodesia --version     # should print 0.1.2
rhodesia -e 'println("hi from rhodesia")'
```

## What's included

- **`rhodesia` CLI**: REPL + script runner (`./rhodesia`, `./rhodesia file.rho`,
  `./rhodesia -e "code"`, `./rhodesia --vm file.rho`).
- **Standard library** (math, stats, numerical, vector, matrix, string,
  mapping, array, io, datetime, net).
- **`libs/math`** user-space library — imported via `include math` (linear
  algebra, numerical methods, statistics, probability, regression).
- **VS Code extension** — syntax highlighting, snippets, linting,
  autocomplete, hover documentation, semantic highlighting, and formatting
  for `.rho` files.

## Troubleshooting

- `rhodesia` not found after install → reopen your terminal or run
  `refreshenv` (cmder) / restart PowerShell.
- VS Code doesn't pick up `.rho` files → confirm the extension is enabled
  in the Extensions panel (look for the publisher `GiulianoCrenna`).
- See [Errors](troubleshooting/errors.md) and [Debugging](troubleshooting/debugging.md)
  for more.