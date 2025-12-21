# Getting Started with Rhodesia

Welcome to Rhodesia! This guide will help you get up and running with your first Rhodesia program.

## Prerequisites

Before you begin, make sure you have:

- C++ compiler (GCC, Clang, or MSVC)
- CMake (version 3.10 or higher)
- Eigen3 library

### Installation on Ubuntu/Debian

```bash
sudo apt update
sudo apt install build-essential cmake libeigen3-dev
```

### Installation on Fedora

```bash
sudo dnf install gcc-c++ cmake eigen3-devel
```

### Installation on macOS

```bash
brew install cmake eigen
```

## Building Rhodesia

1. Clone the repository:
```bash
git clone https://github.com/giulicrenna/rhodesia-lang.git
cd rhodesia-lang
```

2. Create build directory and compile:
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

3. Test the installation:
```bash
./rhodesia --version
```

## Your First Program

Create a file called `hello.rho`:

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
println("Sum of vector:", sum(v))
println("Mean of vector:", mean(v))
```

Run your program:

```bash
./rhodesia hello.rho
```

## Interactive Mode

Rhodesia also supports an interactive REPL (Read-Eval-Print Loop):

```bash
./rhodesia
```

Try these commands:

```rhodesia
println("Hello from REPL!")
vec: data = [1, 2, 3, 4, 5]
println("Data:", data)
println("Mean:", mean(data))
```

Type `exit` or `quit` to leave the REPL.

## Running Inline Code

You can execute code directly from the command line:

```bash
./rhodesia -e "println('Direct execution:', 2 * 21)"
```

## Next Steps

- Learn about [Language Syntax](language/syntax.md)
- Explore [Basic Examples](examples/basics.md)
- Read about [Standard Library Functions](standard-library/functions.md)

## Need Help?

If you encounter issues:

1. Check the [Troubleshooting Guide](troubleshooting/errors.md)
2. Report bugs on [GitHub](https://github.com/giulicrenna/rhodesia-lang/issues)

Happy coding with Rhodesia!
