# Input/Output Examples

This page demonstrates input/output operations in Rhodesia, including file reading/writing, user input, and data persistence.

## Table of Contents

- [File Operations](#file-operations)
- [Reading Files](#reading-files)
- [Writing Files](#writing-files)
- [User Input](#user-input)
- [File Management](#file-management)
- [Complete Examples](#complete-examples)

## File Operations

### Basic File Opening and Closing

```rhodesia
// Manual file handling (not recommended)
println("=== Manual File Handling ===")

// Open a file for writing
int: file_handle = io.open("example.txt", "w")
if file_handle != -1 {
    println("File opened successfully")

    // Write some content
    int: bytes_written = io.write(file_handle, "Hello from Rhodesia!\n")
    println("Wrote", bytes_written, "bytes")

    // Close the file
    io.close(file_handle)
    println("File closed")
} else {
    println("Failed to open file")
}
```

### Using Blocks (Recommended)

```rhodesia
// Using blocks automatically handle file closing
println("=== Using Blocks (Recommended) ===")

// Writing to a file
using io.open("data.txt", "w") as file {
    io.write(file, "Line 1\n")
    io.write(file, "Line 2\n")
    io.write(file, "Line 3\n")
    println("Data written to file")
}

// Reading from a file
using io.open("data.txt", "r") as file {
    str: content = io.read(file)
    println("File content:")
    println(content)
}
```

## Reading Files

### Reading Entire Files

```rhodesia
println("=== Reading Entire Files ===")

using io.open("sample_data.txt", "r") as file {
    str: all_content = io.read(file)
    println("Complete file content:")
    println(all_content)
    println("Total characters:", string.length(all_content))
}
```

### Reading Specific Amounts

```rhodesia
println("=== Reading Specific Amounts ===")

using io.open("large_file.txt", "r") as file {
    // Read first 100 bytes
    str: first_100 = io.read(file, 100)
    println("First 100 bytes:")
    println(first_100)

    // Read next 50 bytes
    str: next_50 = io.read(file, 50)
    println("Next 50 bytes:")
    println(next_50)
}
```

### Reading Line by Line

```rhodesia
println("=== Reading Line by Line ===")

using io.open("poem.txt", "r") as file {
    int: line_number = 1
    str: line = io.readline(file)

    while string.length(line) > 0 {
        // Remove trailing newline for display
        str: clean_line = string.trim(line)
        println("Line", line_number, ":", clean_line)
        line_number = line_number + 1
        line = io.readline(file)
    }
}
```

## Writing Files

### Writing Text Files

```rhodesia
println("=== Writing Text Files ===")

using io.open("output.txt", "w") as file {
    io.write(file, "Rhodesia Programming Language\n")
    io.write(file, "Input/Output Examples\n")
    io.write(file, "======================\n")
    io.write(file, "\n")
    io.write(file, "This file demonstrates file writing.\n")
}
```

### Appending to Files

```rhodesia
println("=== Appending to Files ===")

// First create a file
using io.open("log.txt", "w") as file {
    io.write(file, "Initial log entry\n")
}

// Then append to it
using io.open("log.txt", "a") as file {
    io.write(file, "Additional log entry\n")
    io.write(file, "Another log entry\n")
}

// Read the complete log
using io.open("log.txt", "r") as file {
    str: log_content = io.read(file)
    println("Log file contents:")
    println(log_content)
}
```

## User Input

### Basic User Input

```rhodesia
println("=== Basic User Input ===")

// Simple input without prompt
println("Please enter your name:")
str: name = io.input()
println("Hello,", name, "!")

// Input with prompt
str: age_str = io.input("Enter your age: ")
println("You entered:", age_str)
```

### Processing User Input

```rhodesia
println("=== Processing User Input ===")

// Get user preferences
str: language = io.input("What's your favorite programming language? ")
str: experience = io.input("How many years of experience? ")

println("\nSummary:")
println("Language:", language)
println("Experience:", experience, "years")

// Simple validation
if string.contains(language, "Rhodesia") {
    println("Excellent choice!")
} else {
    println("You might want to try Rhodesia too!")
}
```

## File Management

### Checking File Existence

```rhodesia
println("=== File Existence Checks ===")

str: filename = "test_file.txt"

// Check if file exists
int: exists = io.exists(filename)
if exists {
    println("File", filename, "exists")
} else {
    println("File", filename, "does not exist")
}
```

### File Size Information

```rhodesia
println("=== File Size Information ===")

using io.open("data.txt", "w") as file {
    io.write(file, "This is some test data for demonstration purposes.\n")
    io.write(file, "It contains multiple lines of text.\n")
    io.write(file, "We can check its size after writing.\n")
}

int: file_exists = io.exists("data.txt")
if file_exists {
    int: size_bytes = io.filesize("data.txt")
    int: lines_count = io.readlines("data.txt")

    println("File information:")
    println("  Size:", size_bytes, "bytes")
    println("  Lines:", lines_count)
}
```

### Deleting Files

```rhodesia
println("=== File Deletion ===")

str: temp_file = "temporary.txt"

// Create a temporary file
using io.open(temp_file, "w") as file {
    io.write(file, "This is a temporary file that will be deleted.\n")
}

println("Created temporary file:", temp_file)

// Check if it exists
if io.exists(temp_file) {
    println("File exists before deletion")
}

// Delete the file
int: deleted = io.remove(temp_file)
if deleted {
    println("File successfully deleted")
} else {
    println("Failed to delete file")
}

// Verify deletion
if !io.exists(temp_file) {
    println("File no longer exists")
}
```

## Complete Examples

### Data Logger

```rhodesia
println("=== Data Logger Example ===")

// Simulate collecting sensor data
fun collect_sensor_data() -> vec {
    // In a real application, this would read from actual sensors
    return [23.5, 45.2, 12.8, 78.9, 34.6]
}

fun log_data(str: filename, vec: data, str: timestamp) -> void {
    using io.open(filename, "a") as file {
        io.write(file, timestamp)
        io.write(file, ": ")

        for i in range(math.size(data)) {
            io.write(file, string.from_float64(data[i]))
            if i < math.size(data) - 1 {
                io.write(file, ", ")
            }
        }
        io.write(file, "\n")
    }
}

// Collect and log data multiple times
for reading in range(5) {
    vec: sensor_data = collect_sensor_data()
    str: timestamp = "2025-12-21 15:30:" + string.from_int(reading * 10)

    log_data("sensor_log.txt", sensor_data, timestamp)
    println("Logged reading", reading + 1)
}

// Display the log
using io.open("sensor_log.txt", "r") as file {
    str: log_content = io.read(file)
    println("\nSensor log contents:")
    println(log_content)
}
```

### Configuration File Handler

```rhodesia
println("=== Configuration File Handler ===")

// Create a configuration file
fun create_config(str: filename) -> void {
    using io.open(filename, "w") as file {
        io.write(file, "# Rhodesia Application Configuration\n")
        io.write(file, "app_name=My Rhodesia App\n")
        io.write(file, "version=1.0.0\n")
        io.write(file, "debug=false\n")
        io.write(file, "max_connections=100\n")
        io.write(file, "timeout=30\n")
    }
}

fun read_config(str: filename) -> map {
    map: config = {}

    using io.open(filename, "r") as file {
        str: line = io.readline(file)
        while string.length(line) > 0 {
            str: clean_line = string.trim(line)

            // Skip comments and empty lines
            if string.length(clean_line) > 0 && !string.starts_with(clean_line, "#") {
                // Parse key=value
                vec: parts = string.split(clean_line, "=")
                if math.size(parts) == 2 {
                    str: key = string.trim(parts[0])
                    str: value = string.trim(parts[1])
                    config[key] = value
                }
            }

            line = io.readline(file)
        }
    }

    return config
}

fun update_config(str: filename, str: key, str: new_value) -> void {
    vec: lines = {}

    // Read all lines
    using io.open(filename, "r") as file {
        str: line = io.readline(file)
        while string.length(line) > 0 {
            str: clean_line = string.trim(line)

            if string.length(clean_line) > 0 && !string.starts_with(clean_line, "#") {
                vec: parts = string.split(clean_line, "=")
                if math.size(parts) == 2 && string.trim(parts[0]) == key {
                    // Update this line
                    lines = math.append(lines, key + "=" + new_value + "\n")
                } else {
                    lines = math.append(lines, line)
                }
            } else {
                lines = math.append(lines, line)
            }

            line = io.readline(file)
        }
    }

    // Write back all lines
    using io.open(filename, "w") as file {
        for line in lines {
            io.write(file, line)
        }
    }
}

// Demonstrate configuration management
str: config_file = "app_config.txt"

create_config(config_file)
println("Configuration file created")

map: config = read_config(config_file)
println("Initial configuration:")
println("  App name:", config["app_name"])
println("  Version:", config["version"])
println("  Debug:", config["debug"])

update_config(config_file, "debug", "true")
println("Updated debug setting to true")

map: updated_config = read_config(config_file)
println("Updated configuration:")
println("  Debug:", updated_config["debug"])
```

## Next Steps

- [Basic Examples](basics.md) - Learn basic Rhodesia syntax
- [Data Structures Examples](data-structures.md) - Work with vectors and matrices
- [Machine Learning Examples](machine-learning.md) - Explore ML algorithms
- [Standard Library](standard-library/functions.md) - Discover built-in functions

## Summary

Rhodesia's IO module provides comprehensive file and input handling capabilities:

- **File Operations**: Open, close, read, write, append
- **User Input**: Command-line input with or without prompts
- **File Management**: Check existence, get size, delete files
- **Using Blocks**: Automatic resource management
- **Error Handling**: Check return values for operation success

Always use `using` blocks when possible for automatic resource cleanup, and check return values to handle errors gracefully.
