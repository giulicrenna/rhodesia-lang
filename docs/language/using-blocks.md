---
title: Using Blocks
layout: default
nav_order: 4
parent: Language Reference
---

# Using Blocks

A `using` block pairs a resource acquisition with a deterministic release,
even when exceptions are thrown. It is the recommended way to work with
files, sockets, and any other handle that must be closed.

## Syntax

```rhodesia
using <acquire-expression> as <name> {
    // use name
}
```

When the block exits — normally or via `throw` — the resource is released
automatically.

## Files

```rhodesia
using io.open("data.txt", "r") as f {
    string: contents = io.read(f)
    println(contents)
}   // f is closed here, even on exception
```

Compare to the manual form, which leaks the handle on exception:

```rhodesia
int: f = io.open("data.txt", "r")
string: contents = io.read(f)
io.close(f)
```

## Sockets

```rhodesia
using net.socket("example.com", 80) as sock {
    net.connect(sock)
    net.send(sock, "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n")
    string: response = net.recv_all(sock)
    println(response)
}
```

## How It Works

The block:

1. Evaluates the acquire expression and binds the result to `<name>`.
2. Runs the body.
3. Calls the matching close function on `<name>` (for `io.open` it is
   `io.close`, for `net.socket` it is `net.close`).
4. Re-raises any in-flight exception **after** the close.

## When to Use

Use `using` for any handle that has a single owner and an obvious cleanup
action — files, sockets, database connections, lock guards. For other
cleanup patterns, structure your code so resources are released on every
path (early return, exception, fall-through).

See also: [Exceptions](exceptions.md), [io module](../standard-library/io.md).