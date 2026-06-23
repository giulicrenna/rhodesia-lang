---
title: mapping
layout: default
nav_order: 8
parent: Standard Library
---

# `mapping` module

Map / dictionary operations on key-value stores. Keys are typically strings
or integers; values can be any Rhodesia type.

## Construction

| Function | Description |
|---|---|
| `mapping.create()` | New empty map |
| `mapping.copy(m)` | Shallow copy |
| `mapping.clear(m)` | Remove all entries |

## Mutation

| Function | Description |
|---|---|
| `mapping.set(m, key, value)` | Set `key` to `value` (alias `put`) |
| `mapping.update(m, other)` | Merge `other` into `m` |
| `mapping.delete(m, key)` | Remove `key` (alias `remove`) |

## Lookup

| Function | Description |
|---|---|
| `mapping.get(m, key)` | Value for `key` (throws if missing) |
| `mapping.has(m, key)` | `true` if `key` is present |

## Inspection

| Function | Description |
|---|---|
| `mapping.size(m)` | Number of entries |
| `mapping.isempty(m)` | `true` if no entries |
| `mapping.keys(m)` | List of keys |
| `mapping.values(m)` | List of values |

## Examples

```rhodesia
map: m = mapping.create()
mapping.set(m, "name", "Rhodesia")
mapping.set(m, "version", 1)

string: n = mapping.get(m, "name")         // "Rhodesia"
bool:    ok = mapping.has(m, "version")    // true

arr: ks = mapping.keys(m)                  // ["name", "version"]
int: sz = mapping.size(m)                  // 2
```

## Literals

For short inline construction, use the `{key: value, …}` literal syntax
which produces a `record` (fixed fields). Use `mapping.create()` when the
shape is dynamic.