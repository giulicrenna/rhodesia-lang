# Module Alias Examples

This folder contains examples demonstrating Rhodesia's module alias system and conflict resolution.

## Examples

### module_a.rho and module_b.rho
Example modules that demonstrate:
- Module creation
- Function definitions
- Exportable symbols

### test_alias.rho
Demonstrates basic module alias usage:
- Importing modules with aliases
- Using aliased functions
- Basic alias patterns

### test_conflict_resolution.rho
Showcases conflict resolution strategies:
- Handling name conflicts
- Using aliases to resolve conflicts
- Importing specific symbols
- Managing namespace collisions

### test_import_all.rho
Demonstrates importing all symbols from a module:
- Wildcard imports
- Importing all functions
- Importing all variables
- Using imported symbols

### test_mixed.rho
Shows mixed import styles:
- Combining selective and wildcard imports
- Using aliases with selective imports
- Complex import patterns
- Best practices for module usage

### test_module.rho
Tests module functionality:
- Module loading
- Symbol resolution
- Import verification
- Module testing patterns

## Key Concepts Covered

- Module alias syntax (`as` keyword)
- Conflict resolution strategies
- Selective vs wildcard imports
- Namespace management
- Import best practices
- Module testing

## Usage

```bash
# Run basic alias example
./rhodesia examples/modules/alias/test_alias.rho

# Run conflict resolution example
./rhodesia examples/modules/alias/test_conflict_resolution.rho
```

## Related Examples

- [Simple Modules](../simple/README.md) - Basic module usage
- [Modules Overview](../README.md) - Module system overview
