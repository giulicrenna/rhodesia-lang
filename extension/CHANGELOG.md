# Changelog

All notable changes to the Rhodesia VS Code extension.

## [Unreleased]

### Fixed
- Indentation pattern now triggers on opening braces only (line ending with `{`).
- Grammar no longer hardcodes builtin function names (was drifting from `builtinDescriptors.js`).
- Removed dead TypeScript scaffolding (`tsconfig.json`, `out/`, broken npm scripts, unused devDependencies).
- README links to non-existent docs replaced with valid paths.
- Aligned `engines.vscode` (package.json) and README to `^1.79.0`.
- Linter no longer re-validates on every keystroke (debounced 150ms).
- Brace and string validation now respects comments and string literals.
- String and paren tracking no longer trips on `//` or `/* */` comments.

### Added
- Linter checks: unterminated string literals, `fun` without body, unknown `include` path.
- Linter honors `rhodesia.maxNumberOfProblems` and `rhodesia.enableLinting`.
- Document symbol provider caches by `document.version` (invalidates on edit).
- Completion provider pre-builds static items (keywords/types/builtins/modules) once.
- Document formatter (`server/formatter.js`): trims trailing whitespace, normalises tabs, indents by brace depth. Configurable via `rhodesia.formatter.indentSize`.
- Snippets (`snippets/rho.json`): `fun`, `if`, `ifelse`, `for`, `while`, `return`, `include`, `print`, `println`, `vec`, `mat`, `main`.
- Semantic tokens: function/variable/parameter/type/namespace/constant with `declaration` and `readonly` modifiers.
- Workspace symbol provider: search across all `.rho` files in the workspace.
- Reference provider: find all textual occurrences of a symbol across the workspace.
- Rename provider: rename a symbol across all `.rho` files in the workspace.
- CodeLens: shows "N references" above every `fun` declaration.
- Inlay hints: shows inferred type (`vec`, `mat`, `int`, `float64`, `string`, `bool`) after `let` declarations without an explicit type.
- Marketplace metadata: `publisher`, `galleryBanner`, `qna`, `markdown`.

## [0.1.0] - 2025

### Added
- Syntax highlighting for `.rho` files (TextMate grammar).
- Autocomplete for keywords, types, builtins, and module-qualified names.
- Hover documentation for builtins, module functions, module constants, and variables.
- Signature help for plain and module-qualified calls.
- Go-to-definition for user functions, module functions, and included files.
- Document symbols (outline view).
- Linter for unbalanced braces, missing return types, and int/float type mismatches.
- Quick fix: add `-> void` return type to `fun` declarations.
