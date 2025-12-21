# Rhodesia VS Code Extension

![Rhodesia Logo](https://raw.githubusercontent.com/giulicrenna/rhodesia-lang/main/images/RHODESIA.png)

**Comprehensive language support for Rhodesia programming language in Visual Studio Code**

## 🚀 Features

✅ **Syntax Highlighting** - Full syntax coloring for Rhodesia code
✅ **Intelligent Autocomplete** - Context-aware code completion
✅ **Hover Documentation** - Rich type information and function signatures
✅ **Signature Help** - Parameter hints for function calls
✅ **Code Navigation** - Document symbols and go-to-definition
✅ **Linting & Error Detection** - Real-time syntax validation
✅ **Configuration Options** - Customizable extension behavior

## 📦 Installation

### From VS Code Marketplace (Coming Soon)
1. Open VS Code
2. Go to Extensions view (`Ctrl+Shift+X`)
3. Search for "Rhodesia Language Support"
4. Click Install

### From Source
1. Clone this repository
2. Open the `extension` folder in VS Code
3. Run `npm install`
4. Press `F5` to launch the extension in a new VS Code window

## 🎯 Usage

### Basic Features
- **Syntax Highlighting**: Automatic for `.rho` files
- **Autocomplete**: Trigger with `Ctrl+Space`
- **Hover Documentation**: Hover over symbols to see type info
- **Signature Help**: Automatic when typing function calls
- **Code Navigation**: Use `Ctrl+Shift+O` for outline view

### Commands
- **Show Extension Info**: `Ctrl+Shift+P` → "Rhodesia: Show Info"

### Configuration
Add to your VS Code `settings.json`:

```json
{
  "rhodesia.enableLinting": true,
  "rhodesia.enableAutocomplete": true,
  "rhodesia.showTypeHints": true,
  "rhodesia.maxNumberOfProblems": 100
}
```

## 📚 Documentation

- [Full Extension Documentation](EXTENSION_DOCUMENTATION.md)
- [Rhodesia Language Reference](../../documents/Language%20Reference.md)
- [Standard Library Reference](../../documents/Standard%20Library%20Reference.md)

## 🔧 Development

### Requirements
- Node.js 16+
- VS Code 1.75+
- npm or yarn

### Build
```bash
npm install
npm run compile
```

### Test
```bash
# Run the test file
code extension/test_extension.rho

# Test features:
# 1. Verify syntax highlighting
# 2. Test autocomplete (Ctrl+Space)
# 3. Test hover documentation
# 4. Test signature help
# 5. Test code navigation (Ctrl+Shift+O)
```

## 📖 Supported Features

### Syntax Highlighting
- Keywords: `fun`, `return`, `for`, `while`, `if`, `else`, `in`, `break`, `continue`
- Types: `int`, `float64`, `vec`, `mat`, `string`, `void`
- Operators: `+`, `-`, `*`, `/`, `%`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `and`, `or`, `not`
- Literals: Numbers, strings, vectors, matrices
- Comments: `//` and `/* */`

### Autocomplete
- All keywords and types
- Built-in functions with signatures
- Context-aware suggestions

### Hover Documentation
- Variable type information
- Function signatures and descriptions
- All built-in functions documented

### Signature Help
- Parameter hints for all built-in functions
- Multiple signatures for overloaded functions
- Active parameter highlighting

### Code Navigation
- Function and variable symbols in outline
- Go-to-definition support
- Document symbol provider

### Linting
- Syntax error detection
- Type checking
- Quick fixes for common issues

## 📦 Files

```
extension/
├── extension.js          # Main extension code
├── package.json          # Extension manifest
├── server/
│   ├── linter.js         # Linting functionality
│   └── autocomplete.js   # Autocomplete provider
├── syntaxes/
│   └── rhodesia.tmLanguage.json  # Syntax highlighting rules
├── language-configuration.json   # Language configuration
├── EXTENSION_DOCUMENTATION.md   # Complete documentation
├── README.md             # This file
└── test_extension.rho    # Test file
```

## 🤝 Contributing

Contributions are welcome! Please see the main [Rhodesia repository](https://github.com/giulicrenna/rhodesia-lang) for contribution guidelines.

## 📬 Support

For issues, questions, or feature requests:
- [GitHub Issues](https://github.com/giulicrenna/rhodesia-lang/issues)
- [GitHub Discussions](https://github.com/giulicrenna/rhodesia-lang/discussions)

## 📝 License

This extension is licensed under the [MIT License](../../LICENSE).

---

© 2025 Rhodesia Language Project. All rights reserved.
