const vscode = require('vscode');
const { getAvailableLibPaths } = require('./doxygenParser');

const DEBOUNCE_MS = 150;

// Pre-compute the set of valid include paths once at activation
const VALID_INCLUDES = new Set(getAvailableLibPaths());

class RhodesiaLinter {
    constructor() {
        this.diagnosticCollection = vscode.languages.createDiagnosticCollection('rhodesia');
        this._timers = new Map(); // uri.fsPath -> timeoutId
    }

    activate(context) {
        if (vscode.window.activeTextEditor
            && vscode.window.activeTextEditor.document.languageId === 'rhodesia') {
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
                    this._scheduleValidation(event.document);
                }
            })
        );

        // Re-validate when settings change
        context.subscriptions.push(
            vscode.workspace.onDidChangeConfiguration(e => {
                if (e.affectsConfiguration('rhodesia.enableLinting')
                    || e.affectsConfiguration('rhodesia.maxNumberOfProblems')) {
                    if (vscode.window.activeTextEditor) {
                        this._scheduleValidation(vscode.window.activeTextEditor.document);
                    }
                }
            })
        );
    }

    _scheduleValidation(document) {
        const key = document.uri.fsPath;
        if (this._timers.has(key)) clearTimeout(this._timers.get(key));
        this._timers.set(key, setTimeout(() => {
            this._timers.delete(key);
            this.validateTextDocument(document);
        }, DEBOUNCE_MS));
    }

    validateTextDocument(document) {
        const config = vscode.workspace.getConfiguration('rhodesia');
        if (!config.get('enableLinting', true)) {
            this.diagnosticCollection.set(document.uri, []);
            return;
        }

        const maxProblems = config.get('maxNumberOfProblems', 100);
        const diagnostics = [];
        this._validateBraces(document, diagnostics);
        this._validateFunDeclarations(document, diagnostics);
        this._validateStrings(document, diagnostics);
        this._validateIncludes(document, diagnostics);
        this._validateTypeAssign(document, diagnostics);

        this.diagnosticCollection.set(
            document.uri,
            diagnostics.slice(0, maxProblems)
        );
    }

    _validateBraces(document, diagnostics) {
        const text = document.getText();
        let braceDepth = 0;
        let parenDepth = 0;
        const len = text.length;

        for (let i = 0; i < len; i++) {
            const ch = text[i];
            // Skip line comments
            if (ch === '/' && text[i + 1] === '/') {
                const nl = text.indexOf('\n', i);
                i = nl === -1 ? len : nl;
                continue;
            }
            // Skip block comments
            if (ch === '/' && text[i + 1] === '*') {
                const end = text.indexOf('*/', i + 2);
                i = end === -1 ? len - 1 : end + 1;
                continue;
            }
            // Skip strings
            if (ch === '"') {
                const strEnd = this._findStringEnd(text, i);
                i = strEnd;
                continue;
            }
            if (ch === '{') braceDepth++;
            else if (ch === '}') {
                braceDepth--;
                if (braceDepth < 0) {
                    const pos = document.positionAt(i);
                    diagnostics.push(new vscode.Diagnostic(
                        new vscode.Range(pos, pos.translate(0, 1)),
                        'Unexpected closing brace',
                        vscode.DiagnosticSeverity.Error
                    ));
                    braceDepth = 0;
                }
            } else if (ch === '(') parenDepth++;
            else if (ch === ')') parenDepth--;
        }

        if (braceDepth > 0) {
            const lastLine = document.lineCount - 1;
            const lastCol = document.lineAt(lastLine).text.length;
            diagnostics.push(new vscode.Diagnostic(
                new vscode.Range(lastLine, lastCol, lastLine, lastCol),
                `Missing ${braceDepth} closing brace${braceDepth > 1 ? 's' : ''}`,
                vscode.DiagnosticSeverity.Error
            ));
        }
    }

    _findStringEnd(text, startIdx) {
        // startIdx points at the opening "
        for (let i = startIdx + 1; i < text.length; i++) {
            if (text[i] === '\\') { i++; continue; }
            if (text[i] === '\n') return i - 1; // unterminated, stop at newline
            if (text[i] === '"') return i;
        }
        return text.length - 1;
    }

    _validateFunDeclarations(document, diagnostics) {
        const text = document.getText();
        const lines = text.split('\n');

        for (let i = 0; i < lines.length; i++) {
            const line = lines[i];
            const funDeclMatch = line.match(/\bfun\s+(\w+)\s*\(/);
            if (!funDeclMatch) continue;

            // Look for the opening brace, allowing multi-line signatures
            let bodyFound = false;
            for (let j = i; j < Math.min(i + 10, lines.length); j++) {
                if (lines[j].includes('{')) { bodyFound = true; break; }
                if (lines[j].match(/\bfun\s+\w+\s*\([^)]*\)\s*->\s*[^\s{]+\s*$/)) {
                    // signature complete, no body — error
                    break;
                }
            }

            if (!bodyFound) {
                const funIdx = line.indexOf('fun');
                const endCol = funIdx + 3 + 1 + funDeclMatch[1].length + 1;
                diagnostics.push(new vscode.Diagnostic(
                    new vscode.Range(i, funIdx, i, endCol),
                    `Function '${funDeclMatch[1]}' is missing a return type (-> type)`,
                    vscode.DiagnosticSeverity.Warning
                ));
                continue;
            }

            if (!line.includes('->')) {
                // Check if "->" appears on a continuation line
                let hasReturn = false;
                for (let j = i; j < Math.min(i + 10, lines.length); j++) {
                    if (lines[j].includes('->')) { hasReturn = true; break; }
                    if (lines[j].includes('{')) break;
                }
                if (!hasReturn) {
                    const funIdx = line.indexOf('fun');
                    const endCol = funIdx + 3 + 1 + funDeclMatch[1].length + 1;
                    diagnostics.push(new vscode.Diagnostic(
                        new vscode.Range(i, funIdx, i, endCol),
                        `Function '${funDeclMatch[1]}' is missing a return type (-> type)`,
                        vscode.DiagnosticSeverity.Warning
                    ));
                }
            }
        }
    }

    _validateStrings(document, diagnostics) {
        const text = document.getText();
        let inString = false;
        let stringStart = -1;

        for (let i = 0; i < text.length; i++) {
            const ch = text[i];
            if (ch === '/' && text[i + 1] === '/') {
                const nl = text.indexOf('\n', i);
                if (nl === -1) break;
                i = nl;
                continue;
            }
            if (ch === '/' && text[i + 1] === '*') {
                const end = text.indexOf('*/', i + 2);
                if (end === -1) break;
                i = end + 1;
                continue;
            }
            if (ch === '\\' && inString) { i++; continue; }
            if (ch === '"') {
                if (!inString) {
                    inString = true;
                    stringStart = i;
                } else {
                    inString = false;
                }
            }
        }

        if (inString) {
            const pos = document.positionAt(stringStart);
            const endPos = pos.translate(0, 1);
            diagnostics.push(new vscode.Diagnostic(
                new vscode.Range(pos, endPos),
                'Unterminated string literal',
                vscode.DiagnosticSeverity.Error
            ));
        }
    }

    _validateIncludes(document, diagnostics) {
        if (VALID_INCLUDES.size === 0) return; // no libs/ folder found
        const text = document.getText();
        const lines = text.split('\n');
        const includeRe = /^\s*include\s+(\S+)/;

        for (let i = 0; i < lines.length; i++) {
            const m = lines[i].match(includeRe);
            if (!m) continue;
            const path = m[1];
            if (!VALID_INCLUDES.has(path)) {
                const col = lines[i].indexOf(path);
                const start = new vscode.Position(i, col);
                const end = new vscode.Position(i, col + path.length);
                diagnostics.push(new vscode.Diagnostic(
                    new vscode.Range(start, end),
                    `Unknown include path '${path}'`,
                    vscode.DiagnosticSeverity.Warning
                ));
            }
        }
    }

    _validateTypeAssign(document, diagnostics) {
        const lines = document.getText().split('\n');
        for (let i = 0; i < lines.length; i++) {
            const line = lines[i];
            if (line.trim().startsWith('//')) continue;
            const intAssignMatch = line.match(/\bint\s*:\s*\w+\s*=\s*([^/\n]+)/);
            if (intAssignMatch && /\d+\.\d+/.test(intAssignMatch[1].trim())) {
                const matchIdx = intAssignMatch.index || 0;
                diagnostics.push(new vscode.Diagnostic(
                    new vscode.Range(i, matchIdx, i, line.length),
                    'Cannot assign float value to int variable',
                    vscode.DiagnosticSeverity.Warning
                ));
            }
        }
    }

    dispose() {
        for (const t of this._timers.values()) clearTimeout(t);
        this._timers.clear();
        this.diagnosticCollection.clear();
        this.diagnosticCollection.dispose();
    }
}

module.exports = {
    RhodesiaLinter
};
