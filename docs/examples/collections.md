# Collections Examples

This page demonstrates practical uses of `set`, `tuple`, `record`, and the `match` statement in Rhodesia.

## Table of Contents

- [Set Examples](#set-examples)
- [Tuple Examples](#tuple-examples)
- [Record Examples](#record-examples)
- [Match Statement Examples](#match-statement-examples)
- [Combined Examples](#combined-examples)

---

## Set Examples

### Unique Word Counter

```rhodesia
// Count how many distinct values appear in a dataset
fun count_unique(arr: items) -> int {
    set: seen = {}
    for item in items {
        set.add(seen, item)
    }
    return set.size(seen)
}
```

### Set Intersection (manual)

```rhodesia
fun intersect(set: a, set: b) -> set {
    set: result = {}
    for elem in a {
        if set.contains(b, elem) {
            set.add(result, elem)
        }
    }
    return result
}

set: x = {1, 2, 3, 4, 5}
set: y = {3, 4, 5, 6, 7}
set: common = intersect(x, y)
println("Intersection:", common)  // set{3, 4, 5}
```

### Visited Nodes in a Graph Traversal

```rhodesia
set: visited = {}
vec: queue  = [0.0]       // Start from node 0
int: steps  = 0

while not set.contains(visited, queue[0]) {
    float64: node = queue[0]
    set.add(visited, node)
    steps = steps + 1

    // (in a real graph, add neighbors to queue here)
    break  // illustrative
}

println("Nodes visited:", set.size(visited))
```

---

## Tuple Examples

### Returning Multiple Values

```rhodesia
fun stats(vec: data) -> tuple {
    float64: lo  = data[0]
    float64: hi  = data[0]
    float64: sum = 0.0
    for x in data {
        if x < lo { lo = x }
        if x > hi { hi = x }
        sum = sum + x
    }
    float64: avg = sum / size(data)
    return (lo, hi, avg)
}

vec: measurements = [3.2, 1.8, 4.5, 2.1, 5.9]
tuple: result = stats(measurements)

println("Min:", result[0])    // 1.8
println("Max:", result[1])    // 5.9
println("Average:", result[2])  // 3.5
```

### 2D and 3D Points

```rhodesia
tuple: p1 = (0.0, 0.0)
tuple: p2 = (3.0, 4.0)

// Euclidean distance
fun distance(tuple: a, tuple: b) -> float64 {
    float64: dx = b[0] - a[0]
    float64: dy = b[1] - a[1]
    return math.sqrt(dx * dx + dy * dy)
}

println("Distance:", distance(p1, p2))  // 5.0
```

### Iterating Results

```rhodesia
// Collect (index, value) pairs above a threshold
fun above_threshold(vec: data, float64: threshold) -> tuple {
    int: count = 0
    float64: total = 0.0
    for x in data {
        if x > threshold {
            count = count + 1
            total = total + x
        }
    }
    return (count, total)
}

vec: readings = [0.5, 1.2, 0.3, 2.1, 1.8, 0.9]
tuple: r = above_threshold(readings, 1.0)
println("Count above 1.0:", r[0])   // 3
println("Sum above 1.0:",  r[1])    // 5.1
```

---

## Record Examples

### Sensor Reading

```rhodesia
record: sensor = {
    id:    "S01",
    value: 23.7,
    unit:  "Celsius",
    valid: true
}

fun format_reading(record: s) -> str {
    if s.valid {
        return s.id
    }
    return "INVALID"
}

println(format_reading(sensor))  // S01
println("Value:", sensor.value, sensor.unit)
```

### Employee Management

```rhodesia
fun net_salary(record: emp) -> float64 {
    float64: tax_rate = 0.0

    if emp.salary > 100000.0 {
        tax_rate = 0.30
    } else if emp.salary > 50000.0 {
        tax_rate = 0.20
    } else {
        tax_rate = 0.10
    }

    return emp.salary * (1.0 - tax_rate)
}

record: e1 = { name: "Alice", salary: 80000.0, dept: "Engineering" }
record: e2 = { name: "Bob",   salary: 45000.0, dept: "Marketing" }

println(e1.name, "net salary:", net_salary(e1))  // 64000
println(e2.name, "net salary:", net_salary(e2))  // 40500
```

### Configuration Record

```rhodesia
record: config = {
    learning_rate: 0.01,
    max_epochs:    1000,
    batch_size:    32,
    verbose:       true
}

fun train(record: cfg) -> float64 {
    float64: loss = 1.0
    int: epoch = 0
    while epoch < cfg.max_epochs {
        loss = loss * (1.0 - cfg.learning_rate)
        epoch = epoch + 1
    }
    if cfg.verbose {
        println("Final loss:", loss)
    }
    return loss
}

float64: final_loss = train(config)
```

---

## Match Statement Examples

### HTTP Response Handler

```rhodesia
fun handle_response(int: status, str: body) -> str {
    match status {
        200 -> {
            return body
        }
        404 -> {
            return "Resource not found"
        }
        401 -> {
            return "Authentication required"
        }
        500 -> {
            println("Server error, retrying...")
            return ""
        }
        _ -> {
            println("Unhandled status:", status)
            return ""
        }
    }
    return ""
}

println(handle_response(200, "Hello!"))  // Hello!
println(handle_response(404, ""))        // Resource not found
```

### Command Dispatcher

```rhodesia
fun dispatch(str: cmd, float64: arg) -> float64 {
    match cmd {
        "sqrt"  -> { return math.sqrt(arg) }
        "abs"   -> { return math.abs(arg) }
        "log"   -> { return math.log(arg) }
        "exp"   -> { return math.exp(arg) }
        "neg"   -> { return -arg }
        _ -> {
            println("Unknown command:", cmd)
            return 0.0
        }
    }
    return 0.0
}

println(dispatch("sqrt", 16.0))   // 4
println(dispatch("log",  2.718))  // ~1
println(dispatch("neg",  5.0))    // -5
```

### Finite State Machine — Traffic Light

```rhodesia
fun tick(str: light) -> str {
    match light {
        "red"    -> { return "green" }
        "green"  -> { return "yellow" }
        "yellow" -> { return "red" }
        _        -> { return "red" }
    }
    return "red"
}

str: current = "red"
int: cycle = 0
while cycle < 9 {
    println("Cycle", cycle, ":", current)
    current = tick(current)
    cycle = cycle + 1
}
```

### Match with Record Fields

```rhodesia
record: request = { method: "POST", path: "/users", auth: true }

fun route(record: req) -> str {
    if not req.auth {
        return "401 Unauthorized"
    }

    match req.method {
        "GET"    -> { return "200 " }
        "POST"   -> { return "201 Created" }
        "DELETE" -> { return "204 No Content" }
        _ -> {
            return "405 Method Not Allowed"
        }
    }
    return ""
}

println(route(request))  // 201 Created
```

---

## Combined Examples

### Data Pipeline

```rhodesia
// Process a batch of sensor records and return statistics as a tuple
fun process_batch(arr: records) -> tuple {
    set:     seen_ids = {}
    float64: total    = 0.0
    int:     valid    = 0

    for r in records {
        match r.valid {
            true -> {
                if not set.contains(seen_ids, r.id) {
                    set.add(seen_ids, r.id)
                    total = total + r.value
                    valid = valid + 1
                }
            }
            _ -> {
                println("Skipping invalid record")
            }
        }
    }

    float64: avg = 0.0
    if valid > 0 {
        avg = total / valid
    }

    return (valid, avg, set.size(seen_ids))
}
```

### Lookup Table with Match

```rhodesia
// Map category codes to descriptive labels
fun category_label(int: code) -> record {
    match code {
        1 -> { return { name: "Critical", priority: 1, color: "red" } }
        2 -> { return { name: "High",     priority: 2, color: "orange" } }
        3 -> { return { name: "Medium",   priority: 3, color: "yellow" } }
        4 -> { return { name: "Low",      priority: 4, color: "green" } }
        _ -> { return { name: "Unknown",  priority: 99, color: "grey" } }
    }
    return { name: "Unknown", priority: 99, color: "grey" }
}

record: cat = category_label(2)
println(cat.name, "— priority:", cat.priority, "— color:", cat.color)
// High — priority: 2 — color: orange
```

---

## Next Steps

- [Collections Reference](../language/collections.md) — Set, Tuple, Record type reference
- [Match Reference](../language/match.md) — Match statement reference
- [Data Structures](data-structures.md) — Vector and matrix examples
- [Standard Library](../standard-library/builtins.md) — Built-in functions
