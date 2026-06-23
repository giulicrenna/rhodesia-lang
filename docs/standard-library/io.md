---
title: io
layout: default
nav_order: 11
parent: Standard Library
---

# `io` module

File I/O, console I/O, and CSV helpers.

## File Handles

| Function | Description |
|---|---|
| `io.open(path, mode)` | Open file (mode: `"r"`, `"w"`, `"a"`, …) → handle |
| `io.close(handle)` | Close handle |
| `io.flush(handle)` | Flush buffers |

## Reading

| Function | Description |
|---|---|
| `io.read(handle)` | Read entire file (text mode) |
| `io.readline(handle)` | Read one line |
| `io.readlines(handle)` | Read all lines into an array |
| `io.read_file(path)` | Read whole file by path |
| `io.read_csv(path)` | Read CSV → record-per-row |

## Writing

| Function | Description |
|---|---|
| `io.write(handle, data)` | Write a string |
| `io.write_file(path, data)` | Write whole file |
| `io.append_file(path, data)` | Append to file |

## Position

| Function | Description |
|---|---|
| `io.seek(handle, pos)` | Seek to byte position |
| `io.tell(handle)` | Current byte position |

## File Metadata

| Function | Description |
|---|---|
| `io.exists(path)` | `true` if file exists |
| `io.file_size(path)` | File size in bytes |
| `io.remove(path)` | Delete file |

## CSV

| Function | Description |
|---|---|
| `io.read_csv(path)` | Read CSV file → record-per-row |
| `io.lines(path)` | Iterate file line by line |

## Console

| Function | Description |
|---|---|
| `io.input()` | Read line from stdin |
| `io.input(prompt)` | Read line with prompt |
| `io.stdin()` | Read everything from stdin |

## Using Blocks (recommended)

Always pair `io.open` with a [`using`](../language/using-blocks.md) block
so handles are released on every exit path:

```rhodesia
using io.open("data.txt", "r") as f {
    string: contents = io.read(f)
    println(contents)
}
```

## Examples

```rhodesia
// Read a whole file
string: text = io.read_file("config.txt")

// Write a file
io.write_file("out.txt", "hello\n")

// CSV
arr: rows = io.read_csv("data.csv")
for r in rows {
    println(r)
}
```