---
title: string
layout: default
nav_order: 7
parent: Standard Library
---

# `string` module

String manipulation. Functions accept `string` values; in some cases they
also accept `tuple` / `record` field access via the dot-free call form
(`string.length(s)`).

## Inspection

| Function | Description |
|---|---|
| `string.length(s)` | Character count |
| `string.size(s)` | Alias of `length` |
| `string.isempty(s)` | `true` if `s` is empty |
| `string.count(s, sub)` | Number of (non-overlapping) occurrences of `sub` |

## Case

| Function | Description |
|---|---|
| `string.lower(s)` | Lowercase |
| `string.upper(s)` | Uppercase |

## Trim

| Function | Description |
|---|---|
| `string.trim(s)` | Strip whitespace from both ends |

## Search

| Function | Description |
|---|---|
| `string.find(s, sub)` | Index of first occurrence of `sub`, or `-1` |
| `string.contains(s, sub)` | `true` if `sub` is in `s` |
| `string.startswith(s, prefix)` | `true` if `s` starts with `prefix` |
| `string.endswith(s, suffix)` | `true` if `s` ends with `suffix` |

## Transform

| Function | Description |
|---|---|
| `string.substr(s, start, length)` | Substring starting at `start` of `length` chars |
| `string.slice(s, start, end)` | Substring `[start, end)` (Python-like) |
| `string.replace(s, old, new)` | Replace occurrences of `old` with `new` |
| `string.reverse(s)` | Reversed string |
| `string.repeat(s, n)` | Repeat `s` `n` times |
| `string.at(s, i)` | Character at index `i` |
| `string.concat(a, b)` | Concatenate two strings |

## Split / Join

| Function | Description |
|---|---|
| `string.split(s, sep)` | Split on `sep` (currently a placeholder) |
| `string.join(parts, sep)` | Join `parts` with `sep` (placeholder) |

> `split` and `join` are placeholders in the current build. Add a
> real implementation when needed — current users rely on manual
> `string.find` + `string.substr` loops.

## Conversion

| Function | Description |
|---|---|
| `string.to_int(s)` | Parse `s` as integer (throws on failure) |
| `string.to_int_or(s, default)` | Parse `s` or return `default` |
| `string.to_float(s)` | Parse `s` as `float64` |

## Examples

```rhodesia
string: s = "  Hello, Rhodesia!  "

string: t   = string.trim(s)            // "Hello, Rhodesia!"
string: up  = string.upper(t)           // "HELLO, RHODESIA!"
int:     n   = string.length(t)         // 17
bool:    has = string.contains(t, "Rhodesia")  // true

string: sub = string.slice(t, 0, 5)     // "Hello"
string: rep = string.repeat("ab", 3)    // "ababab"
```