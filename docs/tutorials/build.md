---
title: Build & Packaging
layout: default
nav_order: 5
parent: Tutorials
---

# Build & Packaging

Build Rhodesia from source, run it through the bytecode VM, and produce
release artifacts (Windows installer, source tarballs).

## Prerequisites

- C++ compiler with C++20 support (GCC 10+, Clang 13+, MSVC 19.30+).
- CMake 3.16 or higher.
- Eigen3 development files.
- (Recommended on Windows) Ninja.

## Configure and Build (Linux / macOS)

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/rhodesia --version
```

The default build type is `Release` (set in `CMakeLists.txt` if you don't
override it on the command line).

## Configure and Build (Windows)

From a *Developer PowerShell* or *x64 Native Tools Command Prompt*:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
.\build\rhodesia.exe --version
```

## Build Types

| Type | Flags | Use |
|---|---|---|
| `Release` | `-O3 -march=native` (GCC/Clang) | Default. Use for benchmarks and releases. |
| `Debug` | `-g` | Debugger-friendly; much slower. |
| `RelWithDebInfo` | `-O2 -g` | Profiling. |
| `MinSizeRel` | `-Os` | Small binaries (rarely useful here). |

Pass `-DCMAKE_BUILD_TYPE=<type>` to override.

## Run Modes

The interpreter has two execution engines:

| Mode | Flag | Notes |
|---|---|---|
| Tree-walking | (default) | Slowest startup, easiest to debug |
| Bytecode VM | `--vm` | Faster steady-state; bytecode is compiled once and executed on the VM |

```bash
./build/rhodesia script.rho        # tree-walking
./build/rhodesia --vm script.rho   # bytecode VM
./build/rhodesia -e "println(1+1)" # inline, tree-walking
./build/rhodesia --vm -e "println(1+1)"  # inline, bytecode VM
```

## Packaging (CPack)

CPack is wired into `CMakeLists.txt`. To produce a source tarball and a
binary package from the current build directory:

```bash
cd build
cpack -G TGZ       # source / binary tar.gz
cpack -G ZIP       # zip archive (Windows-friendly)
cpack -G NSIS      # Windows NSIS installer (needs NSIS on PATH)
```

The package name is `rhodesia-<version>` and the description is
*"A strongly-typed language for Data Science"*.

## Windows Installer (Inno Setup)

The recommended Windows installer is built with **Inno Setup**, not CPack.
See [`installer/windows/`](https://github.com/giulicrenna/rhodesia-lang/tree/main/installer/windows):

```powershell
cd installer/windows
.\build-installer.ps1
# Output: .\Output\rhodesia-setup-<version>.exe
```

The script:

1. Downloads `rhodesia_win64.exe` from the latest GitHub release.
2. Stages `LICENSE`, `README.md`, and `libs\*` into a staging folder.
3. Invokes `ISCC.exe` (Inno Setup Compiler) on `rhodesia.iss`.

Output filename comes from `rhodesia.iss` (`MyAppVersion`).

## Cleaning

```bash
rm -rf build
```

## VS Code Extension

The editor extension lives in `extension/` and is independent of the
C++ build. See [Publishing the VS Code extension](../extension/publishing.md)
for the full release flow.

## See Also

- [Getting Started](../getting-started.md) — first build.
- [Installation & Editor Setup](../installation.md) — end-user install.
- [Architecture](../api/architecture.md) — how the lexer / parser / VM fit
  together.
- [Performance](../api/performance.md) — the `--vm` flag in context.