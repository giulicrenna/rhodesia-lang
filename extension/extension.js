const vscode = require('vscode');
const { RhodesiaLinter } = require('./server/linter');
const { RhodesiaAutocompleteProvider } = require('./server/autocomplete');
const { builtinFunctions, moduleDescriptors } = require('./server/builtinDescriptors');
const { parseIncludedFile } = require('./server/doxygenParser');

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
        vscode.languages.registerCompletionItemProvider('rhodesia', autocompleteProvider, '.', ' ')
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
    // ----------------------------------------------------------------
    context.subscriptions.push(
        vscode.languages.registerDocumentSymbolProvider('rhodesia', {
            provideDocumentSymbols(document, token) {
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

                // Module-qualified: module.funcName
                if (moduleQualMatch) {
                    const qualKey = `${moduleQualMatch[1]}.${word}`;
                    const funcDesc = moduleFuncMap.get(qualKey);
                    if (funcDesc && funcDesc._source) {
                        const targetUri = vscode.Uri.file(funcDesc._source);
                        const lineNum = typeof funcDesc._line === 'number' ? funcDesc._line : 0;
                        const range = new vscode.Range(lineNum, 0, lineNum, 0);
                        return new vscode.Location(targetUri, range);
                    }
                }

                // Symbols from included files
                const docText = document.getText();
                const includeRe = /^include\s+(\S+)/gm;
                let incMatch;
                while ((incMatch = includeRe.exec(docText)) !== null) {
                    const { functions, constants } = parseIncludedFile(incMatch[1].trim());
                    for (const sym of [...functions, ...constants]) {
                        if (sym.label === word && sym._source) {
                            const targetUri = vscode.Uri.file(sym._source);
                            const lineNum = typeof sym._line === 'number' ? sym._line : 0;
                            return new vscode.Location(targetUri, new vscode.Range(lineNum, 0, lineNum, 0));
                        }
                    }
                }

                // User-defined functions in the current document
                const lines = docText.split('\n');
                for (let i = 0; i < lines.length; i++) {
                    const funMatch = lines[i].match(/\bfun\s+(\w+)\s*\(/);
                    if (funMatch && funMatch[1] === word) {
                        const col = lines[i].indexOf('fun');
                        const range = new vscode.Range(i, col, i, col + lines[i].length - col);
                        return new vscode.Location(document.uri, range);
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
