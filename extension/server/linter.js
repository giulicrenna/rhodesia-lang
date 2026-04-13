const vscode = require('vscode');

class RhodesiaLinter {
    constructor() {
        this.diagnosticCollection = vscode.languages.createDiagnosticCollection('rhodesia');
    }

    activate(context) {
        context.subscriptions.push(
            vscode.languages.registerCodeActionsProvider('rhodesia', new RhodesiaCodeActionProvider(), {
                providedCodeActionKinds: [vscode.CodeActionKind.QuickFix]
            })
        );

        if (vscode.window.activeTextEditor) {
            this.validateTextDocument(vscode.window.activeTextEditor.document);
        }

        context.subscriptions.push(
            vscode.window.onDidChangeActiveTextEditor(editor => {
                if (editor && editor.document.languageId === 'rhodesia') {
                    this.validateTextDocument(editor.document);
                }
            })
        );

        context.subscriptions.push(
            vscode.workspace.onDidChangeTextDocument(event => {
                if (event.document.languageId === 'rhodesia') {
                    this.validateTextDocument(event.document);
                }
            })
        );
    }

    validateTextDocument(document) {
        const diagnostics = [];
        this.validateSyntax(document, diagnostics);
        this.validateTypes(document, diagnostics);
        this.diagnosticCollection.set(document.uri, diagnostics);
    }

    validateSyntax(document, diagnostics) {
        const lines = document.getText().split('\n');
        let braceDepth = 0;

        for (let i = 0; i < lines.length; i++) {
            const line = lines[i];

            // Skip comment lines
            if (line.trim().startsWith('//')) continue;

            // Track brace balance across the document
            for (let j = 0; j < line.length; j++) {
                if (line[j] === '{') {
                    braceDepth++;
                } else if (line[j] === '}') {
                    braceDepth--;
                    if (braceDepth < 0) {
                        diagnostics.push(new vscode.Diagnostic(
                            new vscode.Range(i, j, i, j + 1),
                            'Unexpected closing brace',
                            vscode.DiagnosticSeverity.Error
                        ));
                        braceDepth = 0;
                    }
                }
            }

            // Check for function declarations without return type annotation
            // Valid Rhodesia syntax: fun name(...) -> type { ... }
            const funDeclMatch = line.match(/\bfun\s+(\w+)\s*\([^)]*\)/);
            if (funDeclMatch && !line.includes('->')) {
                const funIdx = line.indexOf('fun');
                diagnostics.push(new vscode.Diagnostic(
                    new vscode.Range(i, funIdx, i, funIdx + 3 + 1 + funDeclMatch[1].length),
                    `Function '${funDeclMatch[1]}' is missing a return type (-> type)`,
                    vscode.DiagnosticSeverity.Warning
                ));
            }
        }

        // Report unclosed braces at end of file
        if (braceDepth > 0) {
            const lastLine = document.lineCount - 1;
            diagnostics.push(new vscode.Diagnostic(
                new vscode.Range(lastLine, 0, lastLine, 0),
                `Missing ${braceDepth} closing brace${braceDepth > 1 ? 's' : ''}`,
                vscode.DiagnosticSeverity.Error
            ));
        }
    }

    validateTypes(document, diagnostics) {
        const lines = document.getText().split('\n');

        for (let i = 0; i < lines.length; i++) {
            const line = lines[i];
            if (line.trim().startsWith('//')) continue;

            // Detect float literal assigned to an int variable: int: name = 1.5
            const intAssignMatch = line.match(/\bint\s*:\s*\w+\s*=\s*([^/\n]+)/);
            if (intAssignMatch) {
                const value = intAssignMatch[1].trim();
                if (/\d+\.\d+/.test(value)) {
                    const matchIdx = intAssignMatch.index || 0;
                    diagnostics.push(new vscode.Diagnostic(
                        new vscode.Range(i, matchIdx, i, line.length),
                        'Cannot assign float value to int variable',
                        vscode.DiagnosticSeverity.Warning
                    ));
                }
            }
        }
    }

    dispose() {
        this.diagnosticCollection.clear();
        this.diagnosticCollection.dispose();
    }
}

class RhodesiaCodeActionProvider {
    provideCodeActions(document, range, context, token) {
        const codeActions = [];

        for (const diagnostic of context.diagnostics) {
            if (diagnostic.message.includes('missing a return type')) {
                const fix = new vscode.CodeAction('Add return type -> void', vscode.CodeActionKind.QuickFix);
                fix.edit = new vscode.WorkspaceEdit();
                fix.isPreferred = true;
                fix.diagnostics = [diagnostic];

                const lineText = document.lineAt(diagnostic.range.start.line).text;
                // Insert -> void before the opening brace or at end of line
                const insertPos = lineText.indexOf('{') !== -1
                    ? lineText.indexOf('{')
                    : lineText.length;
                const position = new vscode.Position(diagnostic.range.start.line, insertPos);
                fix.edit.insert(document.uri, position, '-> void ');
                codeActions.push(fix);
            }
        }

        return codeActions;
    }
}

module.exports = {
    RhodesiaLinter
};
