const path = require('path');
const vscode = require('vscode');
const { RhodesiaLinter } = require('./server/linter');
const { RhodesiaAutocompleteProvider } = require('./server/autocomplete');
const { RhodesiaFormatter } = require('./server/formatter');
const { builtinFunctions, moduleDescriptors } = require('./server/builtinDescriptors');
const { parseIncludedFile, parseDocumentText, parseIncludeStatement, resolveIncludeAlias } = require('./server/doxygenParser');
const { provideSemanticTokens, TOKEN_TYPES, TOKEN_MODIFIERS } = require('./server/semanticTokens');
const { provideWorkspaceSymbols, provideReferences } = require('./server/workspace');
const { provideRenameEdits, provideCodeLenses, provideInlayHints } = require('./server/refactor');
const { parseBuiltinLines } = require('./server/cppSourceParser');

// Pre-compute "module.func" -> line map by parsing Builtins.hpp +
// NetworkModule.hpp so module function go-to-def can jump to the actual
// C++ registration site (not line 0).
const builtinLines = parseBuiltinLines();

// Build a fast lookup map: funcLabel -> descriptor (for global built-ins)
const builtinMap = new Map();
builtinFunctions.forEach(func => {
    builtinMap.set(func.label, func);
});

// Build a fast lookup map: "module.func" -> descriptor (for module functions)
const moduleFuncMap = new Map();
// Build a fast lookup map: "module.CONST" -> descriptor (for module constants)
const moduleConstMap = new Map();
Object.entries(moduleDescriptors).forEach(([modName, mod]) => {
    if (mod.functions) {
        mod.functions.forEach(func => {
            // Auto-stamp _source so go-to-def lands on Builtins.hpp for
            // hand-written descriptors that didn't set one.
            if (!func._source) func._source = path.resolve(__dirname, '..', 'include', 'Builtins.hpp');
            // Stamp the actual C++ line when we have one — falls through to 0
            // for functions that live in .rho libs only.
            const lineMap = builtinLines[modName];
            if (!func._line && lineMap && lineMap.has(func.label)) {
                func._line = lineMap.get(func.label);
            }
            moduleFuncMap.set(`${modName}.${func.label}`, func);
        });
    }
    if (mod.constants) {
        mod.constants.forEach(constant => {
            moduleConstMap.set(`${modName}.${constant.label}`, { ...constant, moduleName: modName });
        });
    }
});

/**
 * Build a SignatureHelp response from a descriptor's signatures array.
 */
function buildSignatureHelp(descriptor, activeParam) {
    if (!descriptor || !descriptor.signatures || descriptor.signatures.length === 0) {
        return null;
    }
    const signatures = descriptor.signatures.map(sig => {
        const si = new vscode.SignatureInformation(sig.label, sig.documentation);
        if (sig.parameters) {
            sig.parameters.forEach(param => {
                si.parameters.push(new vscode.ParameterInformation(param.label, param.documentation));
            });
        }
        return si;
    });
    return {
        signatures,
        activeSignature: 0,
        activeParameter: Math.min(activeParam || 0, signatures[0].parameters.length - 1)
    };
}

function activate(context) {
    console.log('Rhodesia Language Extension is now active!');

    // Register linter
    const linter = new RhodesiaLinter();
    linter.activate(context);
    context.subscriptions.push(linter);

    // Register autocomplete provider
    // Triggers: '.' for module completions, ' ' for include path completions
    // NOTE: '/' is intentionally excluded — it resets VS Code's word-based filter,
    //       causing path completions to be filtered against the last segment only.
    const autocompleteProvider = new RhodesiaAutocompleteProvider();
    context.subscriptions.push(
        vscode.languages.registerCompletionItemProvider('rhodesia', autocompleteProvider, '.', ' ', ',', '(')
    );

    // Register formatter
    const formatter = new RhodesiaFormatter();
    context.subscriptions.push(
        vscode.languages.registerDocumentFormattingEditProvider('rhodesia', {
            provideDocumentFormattingEdits(document, options, token) {
                const full = new vscode.Range(
                    document.positionAt(0),
                    document.positionAt(document.getText().length)
                );
                return formatter.format(document, full);
            }
        })
    );
    context.subscriptions.push(
        vscode.languages.registerDocumentRangeFormattingEditProvider('rhodesia', {
            provideDocumentRangeFormattingEdits(document, range, options, token) {
                return formatter.format(document, range);
            }
        })
    );

    // Register semantic tokens (function/variable/parameter/type/namespace highlighting)
    const semanticLegend = new vscode.SemanticTokensLegend(TOKEN_TYPES, TOKEN_MODIFIERS);
    context.subscriptions.push(
        vscode.languages.registerSemanticTokensProvider(
            'rhodesia',
            { provideSemanticTokens },
            semanticLegend
        )
    );

    // Workspace symbol search
    context.subscriptions.push(
        vscode.languages.registerWorkspaceSymbolProvider({ provideWorkspaceSymbols })
    );

    // Reference provider
    context.subscriptions.push(
        vscode.languages.registerReferenceProvider('rhodesia', { provideReferences })
    );

    // Rename provider
    context.subscriptions.push(
        vscode.languages.registerRenameProvider('rhodesia', {
            provideRenameEdits(document, position, newName, token) {
                return provideRenameEdits(document, position, newName, token);
            }
        })
    );

    // CodeLens — "N references" above each fun declaration
    context.subscriptions.push(
        vscode.languages.registerCodeLensProvider('rhodesia', { provideCodeLenses })
    );

    // Inlay hints — show inferred type of `let` declarations
    context.subscriptions.push(
        vscode.languages.registerInlayHintsProvider('rhodesia', {
            provideInlayHints
        })
    );

    // ----------------------------------------------------------------
    // Hover provider — shows signature and documentation
    // Handles both plain names (sqrt) and module-qualified (math.sqrt)
    // ----------------------------------------------------------------
    context.subscriptions.push(
        vscode.languages.registerHoverProvider('rhodesia', {
            provideHover(document, position, token) {
                const wordRange = document.getWordRangeAtPosition(position);
                if (!wordRange) return null;

                const word = document.getText(wordRange);
                const line = document.lineAt(position).text;

                // Check for module-qualified access: "module.word"
                // Look at the character just before the word range start
                const wordStart = wordRange.start.character;
                const prefix = wordStart >= 2 ? line.substring(0, wordStart) : '';
                const moduleQualMatch = prefix.match(/\b(\w+)\.\s*$/);

                if (moduleQualMatch) {
                    const moduleName = moduleQualMatch[1];
                    const qualKey = `${moduleName}.${word}`;

                    // Check module function
                    const funcDesc = moduleFuncMap.get(qualKey);
                    if (funcDesc) {
                        const md = new vscode.MarkdownString();
                        md.appendCodeblock(funcDesc.detail, 'rhodesia');
                        md.appendText('\n' + funcDesc.documentation);
                        return new vscode.Hover(md);
                    }

                    // Check module constant
                    const constDesc = moduleConstMap.get(qualKey);
                    if (constDesc) {
                        const md = new vscode.MarkdownString();
                        md.appendCodeblock(`${moduleName}.${constDesc.label}: ${constDesc.detail}`, 'rhodesia');
                        md.appendText('\n' + constDesc.documentation);
                        return new vscode.Hover(md);
                    }
                }

                // Type hint for variable declarations: "type: varname"
                if (line.match(/\b(int|float64|vec|mat|string|void|bool)\s*:\s*\w+/)) {
                    const typeMatch = line.match(/\b(int|float64|vec|mat|string|void|bool)\s*:/);
                    if (typeMatch && word === typeMatch[1]) {
                        return new vscode.Hover(new vscode.MarkdownString(`**Type:** \`${typeMatch[1]}\``));
                    }
                }

                // Local symbols: plain functions, variables, structs, enums, and
                // aliased include symbols. parseDocumentText runs once per hover;
                // it does not run on every keystroke so cost is acceptable.
                const docText = document.getText();
                const localSyms = parseDocumentText(docText, document.uri.fsPath);

                // 2a. Local function
                const localFn = localSyms.functions.find(f => f.label === word);
                if (localFn) {
                    const md = new vscode.MarkdownString();
                    md.appendCodeblock(localFn.detail, 'rhodesia');
                    if (localFn._return) md.appendText(`\n**Returns:** \`${localFn._return}\`\n`);
                    if (localFn.documentation) md.appendText('\n' + localFn.documentation);
                    return new vscode.Hover(md);
                }

                // 2b. Local variable (type/let/const/destructure)
                const localVar = localSyms.constants.find(v => v.label === word);
                if (localVar) {
                    const md = new vscode.MarkdownString();
                    const kindLabel =
                        localVar.kind === 'const' ? '**Constant**' :
                        localVar.kind === 'let'   ? '**Variable**' :
                        localVar.kind === 'destructure' ? '**Destructured binding**' :
                                                          '**Variable**';
                    md.appendMarkdown(`${kindLabel}: \`${localVar.detail}\`\n\n`);
                    if (localVar.documentation) md.appendMarkdown(localVar.documentation + '\n');
                    return new vscode.Hover(md);
                }

                // 2c. Local struct
                const localStruct = localSyms.structs.find(s => s.label === word);
                if (localStruct) {
                    const md = new vscode.MarkdownString();
                    md.appendCodeblock(`record ${localStruct.label} { ${localStruct.fields.join(', ')} }`, 'rhodesia');
                    if (localStruct.documentation) md.appendText('\n' + localStruct.documentation);
                    return new vscode.Hover(md);
                }

                // 2d. Local enum
                const localEnum = localSyms.enums.find(e => e.label === word);
                if (localEnum) {
                    const md = new vscode.MarkdownString();
                    md.appendCodeblock(`enum ${localEnum.label} { ${localEnum.members.join(', ')} }`, 'rhodesia');
                    if (localEnum.documentation) md.appendText('\n' + localEnum.documentation);
                    return new vscode.Hover(md);
                }

                // 2e. Aliased include symbol: `PI` in `include test_module{MODULE_PI as PI}`
                const aliased = resolveIncludeAlias(docText, word);
                if (aliased) {
                    const parsed = parseIncludedFile(aliased.modulePath);
                    const sym =
                        (parsed.functions || []).find(s => s.label === aliased.originalLabel) ||
                        (parsed.constants || []).find(s => s.label === aliased.originalLabel);
                    if (sym) {
                        const md = new vscode.MarkdownString();
                        md.appendMarkdown(`**Aliased import** — \`${word}\` → \`${aliased.modulePath}.${aliased.originalLabel}\`\n\n`);
                        md.appendCodeblock(sym.detail, 'rhodesia');
                        if (sym.documentation) md.appendText('\n' + sym.documentation);
                        return new vscode.Hover(md);
                    }
                }

                // Global built-in function lookup
                const descriptor = builtinMap.get(word);
                if (descriptor) {
                    const md = new vscode.MarkdownString();
                    md.appendCodeblock(descriptor.detail, 'rhodesia');
                    md.appendText('\n' + descriptor.documentation);
                    return new vscode.Hover(md);
                }

                // Module name hover: hovering over "math", "stats", etc.
                if (moduleDescriptors[word]) {
                    const mod = moduleDescriptors[word];
                    const funcCount = (mod.functions || []).length;
                    const constCount = (mod.constants || []).length;
                    const md = new vscode.MarkdownString();
                    md.appendMarkdown(`**Module \`${word}\`**\n\n`);
                    if (funcCount > 0) md.appendMarkdown(`${funcCount} function${funcCount !== 1 ? 's' : ''}`);
                    if (constCount > 0) md.appendMarkdown(`, ${constCount} constant${constCount !== 1 ? 's' : ''}`);
                    md.appendMarkdown(`\n\nType \`${word}.\` to see all members.`);
                    return new vscode.Hover(md);
                }

                return null;
            }
        })
    );

    // ----------------------------------------------------------------
    // Document symbol provider — functions and variables
    // Cache keyed by document version: invalidates automatically on edit.
    // ----------------------------------------------------------------
    const symbolCache = new Map(); // fsPath -> { version, symbols }
    context.subscriptions.push(
        vscode.languages.registerDocumentSymbolProvider('rhodesia', {
            provideDocumentSymbols(document, token) {
                const cached = symbolCache.get(document.uri.fsPath);
                if (cached && cached.version === document.version) {
                    return cached.symbols;
                }

                const symbols = [];
                const text = document.getText();
                const lines = text.split('\n');

                for (let i = 0; i < lines.length; i++) {
                    const line = lines[i];

                    // Functions: fun name(...)
                    const funMatch = line.match(/\bfun\s+(\w+)\s*\(/);
                    if (funMatch) {
                        const funcName = funMatch[1];
                        const range = new vscode.Range(i, line.indexOf('fun'), i, line.length);
                        symbols.push(new vscode.DocumentSymbol(
                            funcName, 'Function',
                            vscode.SymbolKind.Function, range, range
                        ));
                    }

                    // Variables: type: name
                    const varMatch = line.match(/\b(int|float64|vec|mat|string|void|bool)\s*:\s*(\w+)/);
                    if (varMatch) {
                        const varName = varMatch[2];
                        const range = new vscode.Range(i, line.indexOf(varMatch[1]), i, line.length);
                        symbols.push(new vscode.DocumentSymbol(
                            varName, `Variable (${varMatch[1]})`,
                            vscode.SymbolKind.Variable, range, range
                        ));
                    }
                }

                symbolCache.set(document.uri.fsPath, {
                    version: document.version,
                    symbols
                });
                return symbols;
            }
        })
    );

    // ----------------------------------------------------------------
    // Signature help provider — parameter hints while typing a call
    // Handles both plain calls (zeros(...)) and module calls (math.zeros(...))
    // ----------------------------------------------------------------
    context.subscriptions.push(
        vscode.languages.registerSignatureHelpProvider('rhodesia', {
            provideSignatureHelp(document, position, token, context) {
                const linePrefix = document.lineAt(position).text.substring(0, position.character);

                // Count commas after the last '(' to determine active parameter
                const lastParen = linePrefix.lastIndexOf('(');
                if (lastParen === -1) return null;
                const afterParen = linePrefix.substring(lastParen + 1);
                const activeParam = afterParen.split(',').length - 1;

                // Match "module.func(" or "func("
                const moduleCallMatch = linePrefix.match(/\b(\w+)\.(\w+)\s*\($/);
                if (moduleCallMatch) {
                    const key = `${moduleCallMatch[1]}.${moduleCallMatch[2]}`;
                    const funcDesc = moduleFuncMap.get(key);
                    if (funcDesc) {
                        // Build minimal signature from detail if no signatures array
                        const syntheticDesc = {
                            signatures: funcDesc.signatures || [{
                                label: funcDesc.detail,
                                documentation: funcDesc.documentation,
                                parameters: []
                            }]
                        };
                        return buildSignatureHelp(syntheticDesc, activeParam);
                    }
                }

                const plainCallMatch = linePrefix.match(/\b(\w+)\s*\($/);
                if (plainCallMatch) {
                    const funcName = plainCallMatch[1];
                    const descriptor = builtinMap.get(funcName);
                    if (descriptor) {
                        return buildSignatureHelp(descriptor, activeParam);
                    }
                }

                return null;
            }
        }, '(', ',')
    );

    // ----------------------------------------------------------------
    // Definition provider — Ctrl+click on module.func or local fun
    // ----------------------------------------------------------------
    context.subscriptions.push(
        vscode.languages.registerDefinitionProvider('rhodesia', {
            provideDefinition(document, position, token) {
                const wordRange = document.getWordRangeAtPosition(position);
                if (!wordRange) return null;

                const word = document.getText(wordRange);
                const line = document.lineAt(position).text;
                const wordStart = wordRange.start.character;
                const prefix = wordStart >= 2 ? line.substring(0, wordStart) : '';
                const moduleQualMatch = prefix.match(/\b(\w+)\.\s*$/);
                // ponytail: detection of cursor on a module-name prefix is split
                // across two regexes — `moduleQualMatch` covers "cursor on func
                // name in `module.func`" and `charAfter === '.'` covers "cursor
                // on `module` itself". A single grammar would be cleaner but
                // VS Code hands us the word range, not the full token.
                const wordEnd = wordRange.end.character;
                const charAfter = wordEnd < line.length ? line[wordEnd] : '';

                // Module-qualified: module.funcName — jumps to Builtins.hpp (or
                // the lib .rho file if it came from one).
                if (moduleQualMatch) {
                    const qualKey = `${moduleQualMatch[1]}.${word}`;
                    const funcDesc = moduleFuncMap.get(qualKey);
                    if (funcDesc && funcDesc._source) {
                        const targetUri = vscode.Uri.file(funcDesc._source);
                        const lineNum = typeof funcDesc._line === 'number' ? funcDesc._line : 0;
                        const range = new vscode.Range(lineNum, 0, lineNum, 0);
                        return new vscode.Location(targetUri, range);
                    }
                    // Module name itself (cursor on "math" not "math.sqrt")
                    const mod = moduleDescriptors[moduleQualMatch[1]];
                    if (mod) {
                        // Point to the math/index.rho if it exists, else the lib root.
                        const idx = path.resolve(__dirname, '..', 'libs', moduleQualMatch[1], 'index.rho');
                        const fallback = path.resolve(__dirname, '..', 'include', 'Builtins.hpp');
                        const fs = require('fs');
                        const target = fs.existsSync(idx) ? idx : fallback;
                        return new vscode.Location(vscode.Uri.file(target), new vscode.Range(0, 0, 0, 0));
                    }
                }

                // Cursor sits on `module` itself (the next char is `.`).
                // The prefix regex above missed this case — fix by inspecting
                // the character right after the word.
                if (charAfter === '.' && moduleDescriptors[word]) {
                    const idx = path.resolve(__dirname, '..', 'libs', word, 'index.rho');
                    const fallback = path.resolve(__dirname, '..', 'include', 'Builtins.hpp');
                    const fs = require('fs');
                    const target = fs.existsSync(idx) ? idx : fallback;
                    return new vscode.Location(vscode.Uri.file(target), new vscode.Range(0, 0, 0, 0));
                }

                const docText = document.getText();

                // 1. Current document's own symbols (functions, vars, structs, enums)
                const local = parseDocumentText(docText, document.uri.fsPath);
                for (const sym of [...local.functions, ...local.constants, ...local.structs, ...local.enums]) {
                    if (sym.label === word && sym._line != null) {
                        return new vscode.Location(document.uri, new vscode.Range(sym._line, 0, sym._line, 0));
                    }
                }

                // 2. Symbols from included files (bare + brace-selector forms)
                const includeRe = /^\s*include\s+([A-Za-z_][\w/]*)(?:\{([^}]*)\})?/gm;
                let incMatch;
                const seenModules = new Set();
                while ((incMatch = includeRe.exec(docText)) !== null) {
                    const includePath = incMatch[1];
                    if (seenModules.has(includePath)) continue;
                    seenModules.add(includePath);
                    const parsed = parseIncludedFile(includePath);

                    // Parse selector (if any). null = bare path, 'all' = {} import all.
                    const inner = incMatch[2];
                    let allowed = null;        // null = no filter
                    let aliasMap = new Map();  // alias -> originalLabel
                    if (inner !== undefined) {
                        const trimmed = inner.trim();
                        if (trimmed === '') {
                            allowed = 'all';
                        } else {
                            allowed = new Set();
                            for (const raw of trimmed.split(',').map(s => s.trim()).filter(Boolean)) {
                                const am = raw.match(/^([A-Za-z_]\w*)\s*(?:as\s+([A-Za-z_]\w*))?\s*$/);
                                if (!am) continue;
                                allowed.add(am[1]);
                                if (am[2]) aliasMap.set(am[2], am[1]);
                            }
                        }
                    }

                    const candidates = [];
                    for (const fn of (parsed.functions || [])) {
                        if (allowed && allowed !== 'all' && !allowed.has(fn.label)) continue;
                        const alias = [...aliasMap.entries()].find(([, orig]) => orig === fn.label);
                        candidates.push(alias ? { ...fn, label: alias[0], _originalLabel: fn.label } : fn);
                    }
                    for (const cn of (parsed.constants || [])) {
                        if (allowed && allowed !== 'all' && !allowed.has(cn.label)) continue;
                        const alias = [...aliasMap.entries()].find(([, orig]) => orig === cn.label);
                        candidates.push(alias ? { ...cn, label: alias[0], _originalLabel: cn.label } : cn);
                    }

                    for (const sym of candidates) {
                        if (sym.label === word && sym._source) {
                            const targetUri = vscode.Uri.file(sym._source);
                            const lineNum = typeof sym._line === 'number' ? sym._line : 0;
                            return new vscode.Location(targetUri, new vscode.Range(lineNum, 0, lineNum, 0));
                        }
                    }
                }

                return null;
            }
        })
    );

    // Info command
    context.subscriptions.push(
        vscode.commands.registerCommand('rhodesia.showInfo', () => {
            vscode.window.showInformationMessage(
                'Rhodesia Language Extension\n\n' +
                'Features: syntax highlighting, linting, autocomplete (with module.func support), ' +
                'hover docs, signature help, and code navigation.'
            );
        })
    );
}

function deactivate() {
    console.log('Rhodesia Language Extension is now inactive!');
}

module.exports = { activate, deactivate };
