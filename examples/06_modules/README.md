# Modules Examples

This folder contains examples demonstrating Rhodesia's module system.

## Folder Structure

```
modules/
├── README.md              # This file
├── alias/                 # Module alias examples
└── simple/                # Simple module examples
```

## Examples

### Simple Modules
Basic module usage examples:
- `simple_math.rho` - Simple math functions
- `stats.rho` - Statistical functions
- `main.rho` - Main program using modules
- `test_simple.rho` - Testing simple module usage

### Module Aliases
Advanced module usage with aliases:
- `module_a.rho` and `module_b.rho` - Example modules
- `test_alias.rho` - Basic alias usage
- `test_conflict_resolution.rho` - Conflict resolution
- `test_import_all.rho` - Import all symbols
- `test_mixed.rho` - Mixed import styles
- `test_module.rho` - Module testing

## Key Concepts Covered

- Module creation and organization
- Import statements with `include`
- Selective imports with braces
- Module aliases with `as` keyword
- Conflict resolution strategies
- Importing all symbols from a module
- Mixed import styles

## Usage

```bash
# Run simple module example
./rhodesia examples/modules/simple/main.rho

# Run alias example
./rhodesia examples/modules/alias/test_alias.rho
```

## Related Examples

- [Basic Examples](../basic/README.md) - Fundamental language features
- [Data Structures Examples](../data_structures/README.md) - Vector and matrix operations
