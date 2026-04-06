# Control Flow

Rhodesia provides several control flow constructs to manage program execution flow.

## Conditional Statements

### If-Else

```rhodesia
if condition {
    // code executed if condition is true
} else if other_condition {
    // code executed if other_condition is true
} else {
    // code executed if no conditions are true
}
```

#### Examples

```rhodesia
int: x = 15

if x > 20 {
    println("x is greater than 20")
} else if x > 10 {
    println("x is between 11 and 20")
} else {
    println("x is 10 or less")
}

// Simple if
if x % 2 == 0 {
    println("x is even")
}
```

## Loops

### For Loops

#### Range Iteration

```rhodesia
// Iterate from 0 to n-1
for i in range(5) {
    println(i)  // 0, 1, 2, 3, 4
}

// Iterate from start to end-1
for i in range(2, 8) {
    println(i)  // 2, 3, 4, 5, 6, 7
}
```

#### Vector Iteration

```rhodesia
vec: data = [10.5, 20.3, 15.7, 8.9]

for value in data {
    println(value)
}
```

#### Indexed Iteration

```rhodesia
vec: values = [100, 200, 300]

for i in range(size(values)) {
    float64: val = values[i]
    println("Index", i, ": value", val)
}
```

### While Loops

```rhodesia
while condition {
    // code executed while condition is true
    // must modify condition to avoid infinite loops
}
```

#### Examples

```rhodesia
// Countdown
int: count = 5
while count > 0 {
    println(count)
    count = count - 1
}
println("Launch!")

// Accumulator
int: sum = 0
int: i = 1
while i <= 10 {
    sum = sum + i
    i = i + 1
}
println("Sum from 1 to 10:", sum)
```

## Loop Control

### Break

The `break` statement exits the loop immediately:

```rhodesia
for i in range(100) {
    if i == 5 {
        break  // Exit loop when i == 5
    }
    println(i)  // Prints 0, 1, 2, 3, 4
}
```

### Continue

The `continue` statement skips to the next iteration:

```rhodesia
for i in range(10) {
    if i % 2 == 0 {
        continue  // Skip even numbers
    }
    println(i)  // Prints 1, 3, 5, 7, 9
}
```

## Nested Control Structures

Control structures can be nested:

```rhodesia
for i in range(3) {
    for j in range(3) {
        if i == j {
            println("Diagonal element at", i, j)
        } else if i < j {
            println("Upper triangle at", i, j)
        } else {
            println("Lower triangle at", i, j)
        }
    }
}
```

## Examples

### FizzBuzz

```rhodesia
for i in range(1, 21) {
    if i % 15 == 0 {
        println("FizzBuzz")
    } else if i % 3 == 0 {
        println("Fizz")
    } else if i % 5 == 0 {
        println("Buzz")
    } else {
        println(i)
    }
}
```

### Finding First Multiple

```rhodesia
int: found = 0
for n in range(1, 100) {
    if n % 7 == 0 {
        println("First multiple of 7:", n)
        found = n
        break
    }
}
```

### Factorial with While

```rhodesia
int: n = 10
int: factorial = 1
int: i = 1
while i <= n {
    factorial = factorial * i
    i = i + 1
}
println("Factorial of", n, "is", factorial)
```

## Next Steps

- [Match Statement](match.md) - Pattern matching over a single value
- [Functions](functions.md) - Learn about function definition and calling
- [Examples](../examples/basics.md) - See practical control flow examples
- [Language Syntax](syntax.md) - Complete language reference
