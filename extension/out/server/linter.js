"use strict";
const vscode = require('vscode');
class RhodesiaLinter {
    constructor() {
        this.diagnosticCollection = vscode.languages.createDiagnosticCollection('rhodesia');
    }
    activate(context) {
        context.subscriptions.push(vscode.languages.registerCodeActionsProvider('rhodesia', new RhodesiaCodeActionProvider(), {
            providedCodeActionKinds: [vscode.CodeActionKind.QuickFix]
        }));
        // Register for document changes
        if (vscode.window.activeTextEditor) {
            this.validateTextDocument(vscode.window.activeTextEditor.document);
        }
        context.subscriptions.push(vscode.window.onDidChangeActiveTextEditor(editor => {
            if (editor && editor.document.languageId === 'rhodesia') {
                this.validateTextDocument(editor.document);
            }
        }));
        context.subscriptions.push(vscode.workspace.onDidChangeTextDocument(event => {
            if (event.document.languageId === 'rhodesia') {
                this.validateTextDocument(event.document);
            }
        }));
    }
    validateTextDocument(document) {
        const diagnostics = [];
        // Basic syntax validation
        this.validateSyntax(document, diagnostics);
        // Type checking
        this.validateTypes(document, diagnostics);
        // Update diagnostics
        this.diagnosticCollection.set(document.uri, diagnostics);
    }
    validateSyntax(document, diagnostics) {
        const text = document.getText();
        const lines = text.split('\n');
        // Check for common syntax errors
        for (let i = 0; i < lines.length; i++) {
            const line = lines[i].trim();
            const lineNumber = i;
            // Check for unmatched braces
            const openBraces = (line.match(/\\{/g) || []).length;
            const closeBraces = (line.match(/\\}/g) || []).length;
            // Check for variable declarations without initialization
            if (line.match(/\\b[a-zA-Z_][a-zA-Z0-9_]*\\s*:\\s*(?!int|float64|vec|mat|string|void)/)) {
                const match = line.match(/\\b([a-zA-Z_][a-zA-Z0-9_]*)\\s*:/);
                if (match) {
                    const range = new vscode.Range(lineNumber, match.index, lineNumber, match.index + match[0].length);
                    diagnostics.push(new vscode.Diagnostic(range, `Variable declaration missing type or initialization`, vscode.DiagnosticSeverity.Error));
                }
            }
            // Check for function declarations
            if (line.match(/\\bfun\\b/) && !line.match(/->\\s*(int|float64|vec|mat|string|void)\\b/)) {
                const match = line.match(/\\bfun\\b/);
                if (match) {
                    const range = new vscode.Range(lineNumber, match.index, lineNumber, match.index + 3);
                    diagnostics.push(new vscode.Diagnostic(range, `Function declaration missing return type`, vscode.DiagnosticSeverity.Error));
                }
            }
        }
    }
    validateTypes(document, diagnostics) {
        const text = document.getText();
        const lines = text.split('\n');
        // Simple type checking - this would be more sophisticated in a real implementation
        for (let i = 0; i < lines.length; i++) {
            const line = lines[i].trim();
            const lineNumber = i;
            // Check for type mismatches in assignments
            if (line.match(/\\b(int|float64|vec|mat|string)\\s*:\\s*\\w+\\s*=\\s*([^;]+)/)) {
                const match = line.match(/\\b(int|float64|vec|mat|string)\\s*:\\s*\\w+\\s*=\\s*([^;]+)/);
                if (match) {
                    const declaredType = match[1];
                    const assignedValue = match[2].trim();
                    // Very basic type checking
                    if (declaredType === 'int' && assignedValue.match(/\\.\\d+/)) {
                        const range = new vscode.Range(lineNumber, match.index, lineNumber, line.length);
                        diagnostics.push(new vscode.Diagnostic(range, `Cannot assign float value to int variable`, vscode.DiagnosticSeverity.Warning));
                    }
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
            if (diagnostic.message.includes('missing type or initialization')) {
                const fix = new vscode.CodeAction('Add type annotation', vscode.CodeActionKind.QuickFix);
                fix.edit = new vscode.WorkspaceEdit();
                fix.isPreferred = true;
                fix.diagnostics = [diagnostic];
                // Simple fix - add : int
                const text = document.getText(diagnostic.range);
                const newText = text.replace(/(\\w+)\\s*:$/, '$1: int = ');
                fix.edit.replace(document.uri, diagnostic.range, newText);
                codeActions.push(fix);
            }
        }
        return codeActions;
    }
}
module.exports = {
    RhodesiaLinter
};
