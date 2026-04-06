# Collections: Set, Tuple, and Record

Rhodesia provides three collection types for heterogeneous or structured data: `set`, `tuple`, and `record`. All three support a direct literal syntax, making them easy to create and use inline.

## Table of Contents

- [Set](#set)
- [Tuple](#tuple)
- [Record](#record)
- [Combining Collection Types](#combining-collection-types)

---

## Set

A `set` is an unordered collection that automatically eliminates duplicates. It is useful when you need membership testing or need to work with unique values.

### Literal Syntax

```rhodesia
set: name = {value1, value2, value3}
```

### Basic Usage

```rhodesia
// Create a set with literal syntax
set: colors = {"red", "green", "blue"}
println(colors)  // set{red, green, blue}

// Duplicates are silently ignored
set: primes = {2, 3, 5, 7, 11, 2, 3}
println(primes)  // set{2, 3, 5, 7, 11}

// Empty set
set: empty = {}
```

### Set Operations

Use the `set` module for operations after creation:

```rhodesia
set: s = {10, 20, 30}

// Add an element
set.add(s, 40)

// Remove an element
set.remove(s, 20)

// Check membership
bool: found = set.contains(s, 30)  // true

// Size
int: n = set.size(s)
```

### When to Use Sets

| Use Case | Example |
|---|---|
| Unique items | Unique categories in a dataset |
| Membership test | Is this label already seen? |
| Deduplication | Remove repeated entries |
| Tag collections | Keywords, feature flags |

### Example: Unique Labels

```rhodesia
// Collect all unique categories from a dataset
fun unique_categories(arr: labels) -> set {
    set: seen = {}
    for label in labels {
        set.add(seen, label)
    }
    return seen
}
```

---

## Tuple

A `tuple` is an immutable, ordered, heterogeneous sequence. Unlike `vec`, which stores only `float64` values, a tuple can hold values of any type. Tuples are ideal for returning multiple values from a function or representing fixed-structure data.

### Literal Syntax

```rhodesia
tuple: name = (value1, value2, value3)
```

A trailing comma creates a single-element tuple:

```rhodesia
tuple: single = (42,)
```

An empty tuple:

```rhodesia
tuple: empty = ()
```

### Basic Usage

```rhodesia
// Heterogeneous tuple
tuple: person = ("Alice", 30, true)

// Access by zero-based index
str: name   = person[0]  // "Alice"
int: age    = person[1]  // 30
bool: active = person[2] // true
```

### Tuple as Multiple Return Values

```rhodesia
fun min_max(vec: v) -> tuple {
    float64: lo = v[0]
    float64: hi = v[0]
    for x in v {
        if x < lo { lo = x }
        if x > hi { hi = x }
    }
    return (lo, hi)
}

vec: data = [4.0, 1.0, 9.0, 2.0, 7.0]
tuple: bounds = min_max(data)

println("Min:", bounds[0])  // 1
println("Max:", bounds[1])  // 9
```

### Tuples Are Immutable

Tuples cannot be modified after creation. To "change" a tuple, create a new one:

```rhodesia
tuple: original = (1, 2, 3)
// original[0] = 99  // Not allowed — tuples are read-only
```

### When to Use Tuples

| Use Case | Example |
|---|---|
| Multiple return values | `(min, max)`, `(ok, error)` |
| Fixed-structure data | `(x, y, z)` coordinates |
| Heterogeneous records | `(name, age, score)` |
| Immutable sequences | Configuration constants |

### Example: RGB Color

```rhodesia
tuple: red   = (255, 0, 0)
tuple: green = (0, 255, 0)
tuple: blue  = (0, 0, 255)

println("Red   R:", red[0], "G:", red[1], "B:", red[2])
println("Green R:", green[0], "G:", green[1], "B:", green[2])
```

---

## Record

A `record` is a structure with named fields. It is the closest Rhodesia equivalent to a struct or object. Fields are accessed using dot notation.

### Literal Syntax

```rhodesia
record: name = {
    field1: value1,
    field2: value2
}
```

### Basic Usage

```rhodesia
record: employee = {
    name: "Carlos",
    age: 35,
    salary: 55000.0,
    active: true
}

// Access fields with dot notation
println(employee.name)    // Carlos
println(employee.age)     // 35
println(employee.salary)  // 55000
println(employee.active)  // true
```

### Records in Functions

```rhodesia
fun annual_bonus(record: emp) -> float64 {
    if emp.active {
        return emp.salary * 0.1
    }
    return 0.0
}

record: emp = { name: "Ana", salary: 60000.0, active: true }
float64: bonus = annual_bonus(emp)
println("Bonus:", bonus)  // 6000
```

### Nested Access

Records can be stored in tuples, arrays, or other records:

```rhodesia
record: point = { x: 1.5, y: 2.5 }

// Use record fields in expressions
float64: dist = math.sqrt(point.x * point.x + point.y * point.y)
println("Distance from origin:", dist)
```

### When to Use Records

| Use Case | Example |
|---|---|
| Domain entities | `employee`, `product`, `sensor` |
| Function parameters | Pass structured data cleanly |
| Configuration | Group related settings |
| Data modelling | Rows in a dataset |

### Example: Student Grades

```rhodesia
record: student = {
    name:  "Maria",
    math:  9.5,
    physics: 8.0,
    chemistry: 7.5
}

float64: avg = (student.math + student.physics + student.chemistry) / 3.0
println("Average for", student.name, ":", avg)  // 8.33...
```

---

## Combining Collection Types

The collection types can be freely combined:

```rhodesia
// A tuple holding two records
record: a = { label: "A", value: 1.0 }
record: b = { label: "B", value: 2.0 }
tuple: pair = (a, b)

// A set of strings
set: tags = {"ml", "data", "stats"}

// A record that references a tuple result
record: result = { name: "test", bounds: (0.0, 1.0) }
```

---

## Type Summary

| Type | Ordered | Mutable | Heterogeneous | Duplicates | Access |
|------|---------|---------|---------------|------------|--------|
| `set` | No | Yes | No | Not allowed | `set.*` module |
| `tuple` | Yes | No | Yes | Allowed | `t[i]` |
| `record` | N/A | Yes | Yes | N/A (named) | `r.field` |
| `vec` | Yes | Yes | No (float64) | Allowed | `v[i]` |
| `arr` | Yes | Yes | Yes | Allowed | `a[i]` |

---

## Next Steps

- [Match Statement](match.md) — Pattern matching using collection values
- [Control Flow](control-flow.md) — Conditionals and loops
- [Standard Library — Set Module](../standard-library/builtins.md) — Set operations
- [Examples — Collections](../examples/collections.md) — Practical examples
