---
title: datetime
layout: default
nav_order: 10
parent: Standard Library
---

# `datetime` module

Date, time, duration, and timestamp types.

## Construction

| Function | Description |
|---|---|
| `datetime.now()` | Current timestamp |
| `datetime.today()` | Today at 00:00:00 |
| `datetime.current_time()` | Current time-of-day |
| `datetime.make(y, mo, d, h, mi, s)` | Build a timestamp |
| `datetime.make_date(y, mo, d)` | Date-only timestamp |
| `datetime.make_time(h, mi, s)` | Time-of-day value |
| `datetime.from_timestamp(secs)` | Seconds-since-epoch → timestamp |

## Conversion

| Function | Description |
|---|---|
| `datetime.timestamp(t)` | Seconds since Unix epoch |
| `datetime.format(t, pattern)` | Render `t` using a `strftime`-style pattern |

## Differences

| Function | Description |
|---|---|
| `datetime.diff_seconds(a, b)` | `(a - b)` in seconds |
| `datetime.diff_days(a, b)` | `(a - b)` in days |

## Format Tokens

`datetime.format` accepts the conventional `strftime` tokens:
`%Y` year, `%m` month, `%d` day, `%H` hour, `%M` minute, `%S` second, plus
the usual `%a` / `%A` / `%b` / `%B` names. See the [C++ `std::put_time`
reference](https://en.cppreference.com/w/cpp/io/manip/put_time) for the full
list.

## Examples

```rhodesia
ts: t = datetime.now()
string: s = datetime.format(t, "%Y-%m-%d %H:%M:%S")
println("now:", s)

ts: t0  = datetime.make(2025, 1, 1, 0, 0, 0)
ts: t1  = datetime.now()
int: days = datetime.diff_days(t1, t0)
```