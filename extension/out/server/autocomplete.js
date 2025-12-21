"use strict";
const vscode = require('vscode');
class RhodesiaAutocompleteProvider {
    constructor() {
        this.keywords = [
            'if', 'else', 'for', 'while', 'break', 'continue', 'return', 'in', 'fun'
        ];
        this.types = [
            'int', 'float64', 'vec', 'mat', 'string', 'void'
        ];
        this.builtinFunctions = [
            { label: 'norm', detail: 'norm(vec) -> float64 | norm(mat) -> float64', documentation: 'Compute Euclidean norm of vector or Frobenius norm of matrix' },
            { label: 'dot', detail: 'dot(vec, vec) -> float64', documentation: 'Compute dot product of two vectors' },
            { label: 'transpose', detail: 'transpose(mat) -> mat | transpose(vec) -> mat', documentation: 'Compute transpose of matrix or vector' },
            { label: 'inv', detail: 'inv(mat) -> mat', documentation: 'Compute matrix inverse' },
            { label: 'sum', detail: 'sum(vec) -> float64 | sum(mat) -> float64', documentation: 'Compute sum of all elements' },
            { label: 'mean', detail: 'mean(vec) -> float64 | mean(mat) -> float64', documentation: 'Compute arithmetic mean of all elements' },
            { label: 'zeros', detail: 'zeros(int) -> vec | zeros(int, int) -> mat', documentation: 'Create zero vector or matrix' },
            { label: 'ones', detail: 'ones(int) -> vec | ones(int, int) -> mat', documentation: 'Create vector or matrix filled with ones' },
            { label: 'eye', detail: 'eye(int) -> mat', documentation: 'Create identity matrix' },
            { label: 'range', detail: 'range(int) -> vec | range(int, int) -> vec', documentation: 'Create range vector' },
            { label: 'sqrt', detail: 'sqrt(float64) -> float64 | sqrt(vec) -> vec', documentation: 'Compute square root' },
            { label: 'exp', detail: 'exp(float64) -> float64 | exp(vec) -> vec', documentation: 'Compute exponential' },
            { label: 'log', detail: 'log(float64) -> float64 | log(vec) -> vec', documentation: 'Compute natural logarithm' },
            { label: 'abs', detail: 'abs(float64) -> float64 | abs(vec) -> vec', documentation: 'Compute absolute value' },
            { label: 'sin', detail: 'sin(float64) -> float64', documentation: 'Compute sine' },
            { label: 'cos', detail: 'cos(float64) -> float64', documentation: 'Compute cosine' },
            { label: 'tan', detail: 'tan(float64) -> float64', documentation: 'Compute tangent' },
            { label: 'rows', detail: 'rows(mat) -> int', documentation: 'Get number of rows in matrix' },
            { label: 'cols', detail: 'cols(mat) -> int', documentation: 'Get number of columns in matrix' },
            { label: 'size', detail: 'size(vec) -> int | size(mat) -> int', documentation: 'Get total number of elements' },
            { label: 'print', detail: 'print(...) -> void', documentation: 'Print values without newline' },
            { label: 'println', detail: 'println(...) -> void', documentation: 'Print values with newline' }
        ];
        this.logicalOperators = [
            'and', 'or', 'not'
        ];
    }
    provideCompletionItems(document, position, token, context) {
        const linePrefix = document.lineAt(position).text.substr(0, position.character);
        const items = [];
        // Suggest keywords
        this.keywords.forEach(keyword => {
            items.push(new vscode.CompletionItem(keyword, vscode.CompletionItemKind.Keyword));
        });
        // Suggest types
        this.types.forEach(type => {
            items.push(new vscode.CompletionItem(type, vscode.CompletionItemKind.Class));
        });
        // Suggest built-in functions
        this.builtinFunctions.forEach(func => {
            const item = new vscode.CompletionItem(func.label, vscode.CompletionItemKind.Function);
            item.detail = func.detail;
            item.documentation = func.documentation;
            items.push(item);
        });
        // Suggest logical operators
        this.logicalOperators.forEach(op => {
            items.push(new vscode.CompletionItem(op, vscode.CompletionItemKind.Operator));
        });
        // Context-aware suggestions
        if (linePrefix.includes('fun ') && !linePrefix.includes('->')) {
            // Suggest return types after function declaration
            this.types.forEach(type => {
                const item = new vscode.CompletionItem(type, vscode.CompletionItemKind.Class);
                item.preselect = true;
                items.unshift(item); // Show types first
            });
        }
        if (linePrefix.match(/\\b(int|float64|vec|mat|string|void)\\s*:\\s*\\w*$/)) {
            // Suggest variable names after type declaration
            const varItem = new vscode.CompletionItem('variable_name', vscode.CompletionItemKind.Variable);
            varItem.insertText = 'variable_name = ';
            varItem.range = new vscode.Range(position.line, position.character - 0, position.line, position.character);
            items.unshift(varItem);
        }
        return items;
    }
}
module.exports = {
    RhodesiaAutocompleteProvider
};
