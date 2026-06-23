const vscode = require('vscode');
const { builtinFunctions, moduleDescriptors } = require('./builtinDescriptors');
const { getAvailableLibPaths, parseIncludedFile, parseDocumentText } = require('./doxygenParser');
const { parseKeywords } = require('./cppSourceParser');

// Auto-extract keywords from Token.hpp. Falls back to a minimal set so the
// provider still works if the C++ source can't be read.
const AUTO_KEYWORDS = parseKeywords() || [
    'if', 'else', 'for', 'while', 'fun', 'return', 'int', 'float64',
    'vec', 'mat', 'string', 'bool', 'void'
];

// Subset used for the fun-signature preselect (only type-position keywords).
const TYPE_KEYWORDS = new Set([
    'int', 'int8', 'int16', 'int32', 'uint8', 'uint16', 'uint32', 'uint64',
    'byte', 'float64', 'complex', 'bool', 'vec', 'mat', 'str', 'map', 'arr',
    'set', 'tuple', 'record', 'enum', 'function', 'void', 'null'
]);

// Pre-compute available include paths at activation time
const availableLibPaths = getAvailableLibPaths();

class RhodesiaAutocompleteProvider {
    constructor() {
        this.keywords = AUTO_KEYWORDS;
        this.types = AUTO_KEYWORDS;

        this.builtinFunctions = builtinFunctions;
        this.moduleDescriptors = moduleDescriptors;
        this.moduleNames = Object.keys(moduleDescriptors);

        // Cache for parsed include files within a session
        this._includeCache = new Map();
        // Cache for the current document's own symbols (keyed by fsPath).
        this._localDocCache = new Map();

        // Pre-build static completion items once. They are independent of cursor
        // position; VS Code will fuzzy-filter them. This avoids rebuilding ~140
        // CompletionItem objects (each with a MarkdownString) on every keystroke.
        this._staticItems = this._buildStaticItems();
    }

    _buildStaticItems() {
        const items = [];
        // Push each keyword/type exactly once as a Keyword item. Type
        // highlighting is applied contextually (see fun-signature preselect
        // below) so the base pool doesn't need a second pass.
        this.keywords.forEach(kw => {
            items.push(new vscode.CompletionItem(kw, vscode.CompletionItemKind.Keyword));
        });
        this.builtinFunctions.forEach(func => {
            const item = new vscode.CompletionItem(func.label, vscode.CompletionItemKind.Function);
            item.detail = func.detail;
            item.documentation = new vscode.MarkdownString(func.documentation);
            items.push(item);
        });
        this.moduleNames.forEach(modName => {
            const item = new vscode.CompletionItem(modName, vscode.CompletionItemKind.Module);
            item.detail = `Module: ${modName}`;
            item.documentation = new vscode.MarkdownString(
                `Built-in module \`${modName}\`. Type \`${modName}.\` to see its members.`
            );
            items.push(item);
        });
        return items;
    }

    /**
     * Parse the include statements from the document text and return
     * the union of { functions, constants } from all included files.
     * Honors both bare (`include math`) and brace-selector forms:
     *   include math_utils{square, cube, normalize}
     *   include test_module{MODULE_PI as PI}
     * Aliased descriptors are cloned with `label` rewritten to the alias.
     */
    _getIncludedSymbols(documentText) {
        const functions = [];
        const constants = [];
        // Match bare path OR path{...} — selector is captured separately so the
        // cache key stays the bare module path.
        const includeRe = /^\s*include\s+([A-Za-z_][\w/]*)(?:\{([^}]*)\})?/gm;
        let match;
        while ((match = includeRe.exec(documentText)) !== null) {
            const includePath = match[1];
            if (!this._includeCache.has(includePath)) {
                this._includeCache.set(includePath, parseIncludedFile(includePath));
            }
            const symbols = this._includeCache.get(includePath);
            const selector = match[2];

            if (selector === undefined) {
                // Bare path — import all
                functions.push(...(symbols.functions || []));
                constants.push(...(symbols.constants || []));
                continue;
            }

            // Brace selector — selective import. Empty `{}` means "all".
            const inner = selector.trim();
            const items = inner === ''
                ? (symbols.functions || []).map(f => ({ label: f.label, alias: '' }))
                    .concat((symbols.constants || []).map(c => ({ label: c.label, alias: '' })))
                : inner.split(',').map(s => s.trim()).filter(Boolean).map(s => {
                    const am = s.match(/^([A-Za-z_]\w*)\s*(?:as\s+([A-Za-z_]\w*))?\s*$/);
                    return am ? { label: am[1], alias: am[2] || '' } : null;
                }).filter(Boolean);

            const funcByLabel = new Map((symbols.functions || []).map(f => [f.label, f]));
            const constByLabel = new Map((symbols.constants || []).map(c => [c.label, c]));
            for (const it of items) {
                const fn = funcByLabel.get(it.label);
                if (fn) {
                    functions.push(it.alias
                        ? { ...fn, label: it.alias, _originalLabel: it.label }
                        : fn);
                    continue;
                }
                const cn = constByLabel.get(it.label);
                if (cn) {
                    constants.push(it.alias
                        ? { ...cn, label: it.alias, _originalLabel: it.label }
                        : cn);
                }
            }
        }
        return { functions, constants };
    }

    /**
     * Parse the current document and return its symbols. Cached by
     * (fsPath, version) so repeated keystrokes don't re-parse.
     */
    _getLocalDocumentSymbols(document) {
        const key = document.uri.fsPath;
        const cached = this._localDocCache.get(key);
        if (cached && cached.version === document.version) return cached.symbols;
        const symbols = parseDocumentText(document.getText(), document.uri.fsPath);
        this._localDocCache.set(key, { version: document.version, symbols });
        return symbols;
    }

    provideCompletionItems(document, position, token, context) {
        const linePrefix = document.lineAt(position).text.substring(0, position.character);

        // Include path completions: line starts with "include "
        const includeMatch = linePrefix.match(/^include\s+(.*)$/);
        if (includeMatch) {
            const typed = includeMatch[1];
            const typedLower = typed.toLowerCase();
            const startChar = linePrefix.length - typed.length;
            return availableLibPaths
                .filter(libPath => libPath.toLowerCase().startsWith(typedLower))
                .map(libPath => {
                    const item = new vscode.CompletionItem(libPath, vscode.CompletionItemKind.Module);
                    item.detail = 'Include path';
                    item.documentation = new vscode.MarkdownString(
                        `Include \`${libPath}\`\n\nImports functions and constants from this library file.`
                    );
                    item.insertText = libPath;
                    item.filterText = libPath;
                    item.range = new vscode.Range(
                        position.line, startChar,
                        position.line, position.character
                    );
                    return item;
                });
        }

        // Module-qualified completions: detect "module." at cursor.
        // Type names (vec, mat, str, ...) alias to their module (vector, matrix, string, ...)
        // so `vec.` and `vector.` both surface the same vector methods.
        const moduleMatch = linePrefix.match(/\b(\w+)\.\s*$/);
        if (moduleMatch) {
            const moduleAliases = { vec: 'vector', mat: 'matrix', str: 'string', map: 'mapping', arr: 'array' };
            const moduleName = moduleAliases[moduleMatch[1]] || moduleMatch[1];
            const mod = this.moduleDescriptors[moduleName];
            if (mod) {
                const items = [];
                if (mod.functions) {
                    mod.functions.forEach(func => {
                        const item = new vscode.CompletionItem(func.label, vscode.CompletionItemKind.Function);
                        item.detail = func.detail;
                        item.documentation = new vscode.MarkdownString(func.documentation);
                        item.insertText = func.label;
                        items.push(item);
                    });
                }
                if (mod.constants) {
                    mod.constants.forEach(constant => {
                        const item = new vscode.CompletionItem(constant.label, vscode.CompletionItemKind.Constant);
                        item.detail = `${moduleName}.${constant.label}: ${constant.detail}`;
                        item.documentation = new vscode.MarkdownString(constant.documentation);
                        item.insertText = constant.label;
                        items.push(item);
                    });
                }
                return items;
            }
        }

        // Global completion: reuse pre-built static items + per-doc include symbols
        const items = this._staticItems.slice();

        const { functions: incFuncs, constants: incConsts } = this._getIncludedSymbols(document.getText());
        const seenLabels = new Set(this.builtinFunctions.map(f => f.label));

        incFuncs.forEach(func => {
            if (seenLabels.has(func.label)) return;
            seenLabels.add(func.label);
            const item = new vscode.CompletionItem(func.label, vscode.CompletionItemKind.Function);
            item.detail = func.detail;
            item.documentation = new vscode.MarkdownString(func.documentation);
            items.push(item);
        });

        incConsts.forEach(constant => {
            if (seenLabels.has(constant.label)) return;
            seenLabels.add(constant.label);
            const item = new vscode.CompletionItem(constant.label, vscode.CompletionItemKind.Constant);
            item.detail = constant.detail;
            item.documentation = new vscode.MarkdownString(
                `**${constant.label}** = \`${constant.value}\`\n\n${constant.documentation}`
            );
            items.push(item);
        });

        // Current document's own symbols (functions, constants/variables, structs, enums)
        const local = this._getLocalDocumentSymbols(document);
        local.functions.forEach(func => {
            if (seenLabels.has(func.label)) return;
            seenLabels.add(func.label);
            const item = new vscode.CompletionItem(func.label, vscode.CompletionItemKind.Function);
            item.detail = func.detail;
            item.documentation = new vscode.MarkdownString(func.documentation);
            items.push(item);
        });
        local.constants.forEach(v => {
            if (seenLabels.has(v.label)) return;
            seenLabels.add(v.label);
            const item = new vscode.CompletionItem(v.label, vscode.CompletionItemKind.Variable);
            item.detail = v.detail;
            item.documentation = new vscode.MarkdownString(
                `Local variable: \`${v.detail}\`\n\n${v.documentation}`
            );
            items.push(item);
        });
        local.structs.forEach(s => {
            if (seenLabels.has(s.label)) return;
            seenLabels.add(s.label);
            const item = new vscode.CompletionItem(s.label, vscode.CompletionItemKind.Struct);
            item.detail = `record ${s.label} { ${s.fields.join(', ')} }`;
            item.documentation = new vscode.MarkdownString(s.documentation);
            items.push(item);
        });
        local.enums.forEach(e => {
            if (seenLabels.has(e.label)) return;
            seenLabels.add(e.label);
            const item = new vscode.CompletionItem(e.label, vscode.CompletionItemKind.Enum);
            item.detail = `enum ${e.label} { ${e.members.join(', ')} }`;
            item.documentation = new vscode.MarkdownString(e.documentation);
            items.push(item);
        });

        // Context: pre-select return types when typing a fun signature
        if (linePrefix.includes('fun ') && !linePrefix.includes('->')) {
            this.types.forEach(type => {
                if (!TYPE_KEYWORDS.has(type)) return;
                const item = new vscode.CompletionItem(type, vscode.CompletionItemKind.Class);
                item.preselect = true;
                items.unshift(item);
            });
        }

        return items;
    }
}

module.exports = {
    RhodesiaAutocompleteProvider
};
