const vscode = require('vscode');
const { RhodesiaLinter } = require('./server/linter');
const { RhodesiaAutocompleteProvider } = require('./server/autocomplete');

function activate(context) {
    console.log('Rhodesia Language Extension is now active!');

    // Register linter
    const linter = new RhodesiaLinter();
    linter.activate(context);
    context.subscriptions.push(linter);

    // Register autocomplete provider
    const autocompleteProvider = new RhodesiaAutocompleteProvider();
    context.subscriptions.push(
        vscode.languages.registerCompletionItemProvider('rhodesia', autocompleteProvider, '.')
    );

    // Register hover provider for type hints
    context.subscriptions.push(
        vscode.languages.registerHoverProvider('rhodesia', {
            provideHover(document, position, token) {
                const range = document.getWordRangeAtPosition(position);
                if (!range) {
                    return null;
                }

                const word = document.getText(range);
                const line = document.lineAt(position).text;

                // Provide type hints for variables
                if (line.match(/\\b(int|float64|vec|mat|string|void)\\s*:\\s*\\w+/)) {
                    const typeMatch = line.match(/\\b(int|float64|vec|mat|string|void)\\s*:/);
                    if (typeMatch) {
                        return new vscode.Hover(`Type: **${typeMatch[1]}**`);
                    }
                }

                // Provide function signatures
                if (word === 'norm') {
                    return new vscode.Hover('**norm(vec) -> float64**\n\nCompute Euclidean norm of vector or Frobenius norm of matrix');
                }

                if (word === 'dot') {
                    return new vscode.Hover('**dot(vec, vec) -> float64**\n\nCompute dot product of two vectors');
                }

                if (word === 'transpose') {
                    return new vscode.Hover('**transpose(mat) -> mat**\n\nCompute transpose of matrix');
                }

                // Add more built-in function documentation
                if (word === 'inv') {
                    return new vscode.Hover('**inv(mat) -> mat**\n\nCompute matrix inverse');
                }

                if (word === 'sum') {
                    return new vscode.Hover('**sum(vec) -> float64**\n\nCompute sum of all elements in vector');
                }

                if (word === 'mean') {
                    return new vscode.Hover('**mean(vec) -> float64**\n\nCompute arithmetic mean of vector elements');
                }

                if (word === 'zeros') {
                    return new vscode.Hover('**zeros(int) -> vec**\n\nCreate zero vector\n\n**zeros(int, int) -> mat**\n\nCreate zero matrix');
                }

                if (word === 'ones') {
                    return new vscode.Hover('**ones(int) -> vec**\n\nCreate vector filled with ones\n\n**ones(int, int) -> mat**\n\nCreate matrix filled with ones');
                }

                if (word === 'eye') {
                    return new vscode.Hover('**eye(int) -> mat**\n\nCreate identity matrix');
                }

                if (word === 'range') {
                    return new vscode.Hover('**range(int) -> vec**\n\nCreate range vector from 0 to n-1\n\n**range(int, int) -> vec**\n\nCreate range vector from start to end-1');
                }

                if (word === 'sqrt') {
                    return new vscode.Hover('**sqrt(float64) -> float64**\n\nCompute square root\n\n**sqrt(vec) -> vec**\n\nCompute square root of each element');
                }

                if (word === 'exp') {
                    return new vscode.Hover('**exp(float64) -> float64**\n\nCompute exponential\n\n**exp(vec) -> vec**\n\nCompute exponential of each element');
                }

                if (word === 'log') {
                    return new vscode.Hover('**log(float64) -> float64**\n\nCompute natural logarithm\n\n**log(vec) -> vec**\n\nCompute natural logarithm of each element');
                }

                if (word === 'abs') {
                    return new vscode.Hover('**abs(float64) -> float64**\n\nCompute absolute value\n\n**abs(vec) -> vec**\n\nCompute absolute value of each element');
                }

                if (word === 'sin') {
                    return new vscode.Hover('**sin(float64) -> float64**\n\nCompute sine');
                }

                if (word === 'cos') {
                    return new vscode.Hover('**cos(float64) -> float64**\n\nCompute cosine');
                }

                if (word === 'tan') {
                    return new vscode.Hover('**tan(float64) -> float64**\n\nCompute tangent');
                }

                if (word === 'rows') {
                    return new vscode.Hover('**rows(mat) -> int**\n\nGet number of rows in matrix');
                }

                if (word === 'cols') {
                    return new vscode.Hover('**cols(mat) -> int**\n\nGet number of columns in matrix');
                }

                if (word === 'size') {
                    return new vscode.Hover('**size(vec) -> int**\n\nGet number of elements in vector\n\n**size(mat) -> int**\n\nGet total number of elements in matrix');
                }

                if (word === 'print') {
                    return new vscode.Hover('**print(...) -> void**\n\nPrint values without newline');
                }

                if (word === 'println') {
                    return new vscode.Hover('**println(...) -> void**\n\nPrint values with newline');
                }

                return null;
            }
        })
    );

    // Register document symbol provider for code navigation
    context.subscriptions.push(
        vscode.languages.registerDocumentSymbolProvider('rhodesia', {
            provideDocumentSymbols(document, token) {
                const symbols = [];
                const text = document.getText();
                const lines = text.split('\n');

                // Find functions
                for (let i = 0; i < lines.length; i++) {
                    const line = lines[i];
                    const funMatch = line.match(/\\bfun\\s+(\\w+)\\s*\\(/);
                    if (funMatch) {
                        const funcName = funMatch[1];
                        const range = new vscode.Range(i, line.indexOf('fun'), i, line.length);
                        const symbol = new vscode.DocumentSymbol(
                            funcName,
                            'Function',
                            vscode.SymbolKind.Function,
                            range,
                            range
                        );
                        symbols.push(symbol);
                    }

                    // Find variables
                    const varMatch = line.match(/\\b(int|float64|vec|mat|string|void)\\s*:\\s*(\\w+)/);
                    if (varMatch) {
                        const varName = varMatch[2];
                        const range = new vscode.Range(i, line.indexOf(varMatch[1]), i, line.length);
                        const symbol = new vscode.DocumentSymbol(
                            varName,
                            `Variable (${varMatch[1]})`,
                            vscode.SymbolKind.Variable,
                            range,
                            range
                        );
                        symbols.push(symbol);
                    }
                }

                return symbols;
            }
        })
    );

    // Register signature help provider for function parameter hints
    context.subscriptions.push(
        vscode.languages.registerSignatureHelpProvider('rhodesia', {
            provideSignatureHelp(document, position, token, context) {
                const linePrefix = document.lineAt(position).text.substr(0, position.character);

                // Check if we're in a function call
                const funcCallMatch = linePrefix.match(/(\\w+)\\s*\(/);
                if (!funcCallMatch) {
                    return null;
                }

                const funcName = funcCallMatch[1];
                const signatures = [];

                // Built-in function signatures
                const builtinSignatures = {
                    'norm': [
                        {
                            label: 'norm(vec) -> float64',
                            documentation: 'Compute Euclidean norm of vector or Frobenius norm of matrix',
                            parameters: [
                                {
                                    label: 'vec',
                                    documentation: 'Input vector or matrix'
                                }
                            ]
                        }
                    ],
                    'dot': [
                        {
                            label: 'dot(vec, vec) -> float64',
                            documentation: 'Compute dot product of two vectors',
                            parameters: [
                                {
                                    label: 'vec1',
                                    documentation: 'First vector'
                                },
                                {
                                    label: 'vec2',
                                    documentation: 'Second vector'
                                }
                            ]
                        }
                    ],
                    'transpose': [
                        {
                            label: 'transpose(mat) -> mat',
                            documentation: 'Compute transpose of matrix',
                            parameters: [
                                {
                                    label: 'mat',
                                    documentation: 'Input matrix'
                                }
                            ]
                        }
                    ],
                    'inv': [
                        {
                            label: 'inv(mat) -> mat',
                            documentation: 'Compute matrix inverse',
                            parameters: [
                                {
                                    label: 'mat',
                                    documentation: 'Input matrix'
                                }
                            ]
                        }
                    ],
                    'sum': [
                        {
                            label: 'sum(vec) -> float64',
                            documentation: 'Compute sum of all elements in vector',
                            parameters: [
                                {
                                    label: 'vec',
                                    documentation: 'Input vector'
                                }
                            ]
                        }
                    ],
                    'mean': [
                        {
                            label: 'mean(vec) -> float64',
                            documentation: 'Compute arithmetic mean of vector elements',
                            parameters: [
                                {
                                    label: 'vec',
                                    documentation: 'Input vector'
                                }
                            ]
                        }
                    ],
                    'zeros': [
                        {
                            label: 'zeros(int) -> vec',
                            documentation: 'Create zero vector',
                            parameters: [
                                {
                                    label: 'size',
                                    documentation: 'Size of vector'
                                }
                            ]
                        },
                        {
                            label: 'zeros(int, int) -> mat',
                            documentation: 'Create zero matrix',
                            parameters: [
                                {
                                    label: 'rows',
                                    documentation: 'Number of rows'
                                },
                                {
                                    label: 'cols',
                                    documentation: 'Number of columns'
                                }
                            ]
                        }
                    ],
                    'ones': [
                        {
                            label: 'ones(int) -> vec',
                            documentation: 'Create vector filled with ones',
                            parameters: [
                                {
                                    label: 'size',
                                    documentation: 'Size of vector'
                                }
                            ]
                        },
                        {
                            label: 'ones(int, int) -> mat',
                            documentation: 'Create matrix filled with ones',
                            parameters: [
                                {
                                    label: 'rows',
                                    documentation: 'Number of rows'
                                },
                                {
                                    label: 'cols',
                                    documentation: 'Number of columns'
                                }
                            ]
                        }
                    ],
                    'eye': [
                        {
                            label: 'eye(int) -> mat',
                            documentation: 'Create identity matrix',
                            parameters: [
                                {
                                    label: 'size',
                                    documentation: 'Size of matrix'
                                }
                            ]
                        }
                    ],
                    'range': [
                        {
                            label: 'range(int) -> vec',
                            documentation: 'Create range vector from 0 to n-1',
                            parameters: [
                                {
                                    label: 'end',
                                    documentation: 'End of range (exclusive)'
                                }
                            ]
                        },
                        {
                            label: 'range(int, int) -> vec',
                            documentation: 'Create range vector from start to end-1',
                            parameters: [
                                {
                                    label: 'start',
                                    documentation: 'Start of range'
                                },
                                {
                                    label: 'end',
                                    documentation: 'End of range (exclusive)'
                                }
                            ]
                        }
                    ]
                };

                if (builtinSignatures[funcName]) {
                    builtinSignatures[funcName].forEach(sig => {
                        const signature = new vscode.SignatureInformation(sig.label, sig.documentation);
                        sig.parameters.forEach(param => {
                            signature.parameters.push(new vscode.ParameterInformation(param.label, param.documentation));
                        });
                        signatures.push(signature);
                    });

                    return {
                        signatures: signatures,
                        activeSignature: 0,
                        activeParameter: Math.min(context.activeParameter, signatures[0].parameters.length - 1)
                    };
                }

                return null;
            }
        }), '(', ',');

    // Register command for showing extension info
    const showInfoCommand = vscode.commands.registerCommand('rhodesia.showInfo', () => {
        vscode.window.showInformationMessage('Rhodesia Language Extension v0.1.0\n\n' +
            'Features:\n' +
            '- Syntax highlighting for .rho files\n' +
            '- Basic linting and error detection\n' +
            '- Autocomplete for keywords, types, and functions\n' +
            '- Type hints on hover\n' +
            '- Code navigation with document symbols');
    });

    context.subscriptions.push(showInfoCommand);
}

function deactivate() {
    console.log('Rhodesia Language Extension is now inactive!');
}

module.exports = {
    activate,
    deactivate
};
