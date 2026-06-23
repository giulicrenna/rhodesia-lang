---
title: Getting Started
layout: default
nav_order: 2
---

# Getting Started with Rhodesia

Welcome! This guide walks you through building Rhodesia from source and
running your first program. If you just want to install it, see
[Installation & Editor Setup](installation.md).

## Quick Install (Windows)

[Download Rhodesia 0.1.2 for Windows (.exe)](https://github.com/giulicrenna/rhodesia-lang/releases/latest/download/rhodesia-setup-0.1.2.exe){: .btn .btn-primary }

The installer adds `rhodesia` to your PATH so the REPL and CLI work from any
terminal. Pair it with the VS Code extension (**Rhodesia Language Support**
by `GiulianoCrenna`) for syntax highlighting and tooling.

## Prerequisites (from source)

Before you begin, make sure you have:

- C++ compiler (GCC, Clang, or MSVC) with C++20 support
- CMake 3.16 or higher
- Eigen3 library
- (Windows only) Ninja is recommended

### Ubuntu / Debian

```bash
sudo apt update
sudo apt install build-essential cmake libeigen3-dev ninja-build
```

### Fedora

```bash
sudo dnf install gcc-c++ cmake eigen3-devel ninja-build
```

### macOS

```bash
brew install cmake eigen ninja
```

### Windows

Install Visual Studio (with the C++ workload), then from a Developer PowerShell:

```powershell
cmake --version    # confirm 3.16+
# Eigen3 is downloaded automatically via CMake FetchContent if not present
```

## Building from Source

1. Clone the repository:

```bash
git clone https://github.com/giulicrenna/rhodesia-lang.git
cd rhodesia-lang
```

2. Configure and build (Release by default):

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)      # Linux/macOS
# or
cmake --build build -j               # Windows
```

3. Verify:

```bash
./build/rhodesia --version
```

> Windows installer maintainers: see [Build & Packaging](tutorials/build.md) for
> Inno Setup steps, CPack config, and the `--vm` toggle.

## Your First Program

Create `hello.rho`:

```rhodesia
// hello.rho - Your first Rhodesia program
println("Hello, Rhodesia!")

// Basic arithmetic
int: a = 10
int: b = 20
int: sum = a + b
println("Sum:", sum)

// Vector operations
vec: v = [1, 2, 3, 4, 5]
println("Vector:", v)
println("Sum of vector:", math.sum(v))
println("Mean of vector:", math.mean(v))
```

Run it:

```bash
./build/rhodesia hello.rho
```

## Interactive Mode (REPL)

```bash
./build/rhodesia
```

Try:

```rhodesia
println("Hello from REPL!")
vec: data = [1, 2, 3, 4, 5]
println("Data:", data)
println("Mean:", math.mean(data))
```

REPL commands: `help`, `vars`, `exit`, `quit`. Multi-line input is enabled
whenever braces are unbalanced. See [REPL](tutorials/repl.md) for the full
reference.

## Running Inline Code

```bash
./build/rhodesia -e "println('Direct execution:', 2 * 21)"
```

## Cleaning Build Artifacts

```bash
rm -rf build
```

## Next Steps

- [Installation & Editor Setup](installation.md) — Windows installer and VS Code extension.
- [Language Syntax](language/syntax.md) — full syntax reference.
- [Type System](language/types.md) — primitive and collection types.
- [Basic Examples](examples/basics.md) — learn by doing.
- [Standard Library Overview](standard-library/functions.md) — what ships out of the box.

## Need Help?

- [Troubleshooting — Errors](troubleshooting/errors.md)
- [GitHub Issues](https://github.com/giulicrenna/rhodesia-lang/issues)

Happy coding with Rhodesia!