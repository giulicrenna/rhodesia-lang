const vscode = require('vscode');
const fs = require('fs');
const path = require('path');
const { findRhoFiles } = require('./workspace');

function escapeRe(s) {
    return s.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
}

/**
 * Rename provider: renames the symbol at the cursor across the workspace.
 * Match is whole-word, case-sensitive. Does NOT distinguish declarations from
 * uses — every textual match is renamed. Good enough for a first cut.
 */
async function provideRenameEdits(document, position, newName, token) {
    const wordRange = document.getWordRangeAtPosition(position);
    if (!wordRange) return null;
    const oldName = document.getText(wordRange);
    if (!/^[A-Za-z_]\w*$/.test(newName)) return null;

    const edit = new vscode.WorkspaceEdit();
    const re = new RegExp(`\\b${escapeRe(oldName)}\\b`);
    const folders = vscode.workspace.workspaceFolders || [];
    for (const folder of folders) {
        const files = await findRhoFiles(folder.uri.fsPath);
        for (const file of files) {
            if (token.isCancellationRequested) return edit;
            let content;
            try { content = fs.readFileSync(file, 'utf8'); }
            catch (_) { continue; }
            const lines = content.split('\n');
            for (let i = 0; i < lines.length; i++) {
                let m;
                re.lastIndex = 0;
                while ((m = re.exec(lines[i])) !== null) {
                    edit.replace(
                        vscode.Uri.file(file),
                        new vscode.Range(i, m.index, i, m.index + oldName.length),
                        newName
                    );
                }
            }
        }
    }
    return edit;
}

/**
 * CodeLens provider: shows "N references" above every `fun` declaration.
 * Reference count is for the current document only — cross-file counting is
 * a future optimisation. O(n) scan on demand, add a cache when
 * keystroke-perf matters.
 */
function provideCodeLenses(document, token) {
    const text = document.getText();
    const lines = text.split('\n');
    const lenses = [];
    const re = /\bfun\s+(\w+)\s*\(/;

    for (let i = 0; i < lines.length; i++) {
        const m = lines[i].match(re);
        if (!m) continue;
        const name = m[1];
        const reUse = new RegExp(`\\b${escapeRe(name)}\\b`, 'g');
        let count = 0;
        for (const line of lines) {
            const ms = line.match(reUse);
            if (ms) count += ms.length;
        }
        // Subtract the declaration itself
        const refs = Math.max(0, count - 1);

        const range = new vscode.Range(i, 0, i, 0);
        const lens = new vscode.CodeLens(range);
        lens.command = {
            title: refs === 0
                ? 'No references'
                : `${refs} reference${refs === 1 ? '' : 's'}`,
            command: 'editor.action.referenceSearch.trigger',
            arguments: [document.uri, new vscode.Position(i, 0)]
        };
        lenses.push(lens);
    }
    return lenses;
}

/**
 * InlayHints provider: show the inferred type of `let name = <expr>` if the
 * right-hand side is recognisable. Very rough heuristic — only handles
 * vector/matrix literals (`[...]`, `[[...]]`) and numeric literals.
 */
function provideInlayHints(document, range, token) {
    const text = document.getText();
    const startLine = range.start.line;
    const endLine = range.end.line;
    const lines = text.split('\n');
    const hints = [];

    for (let i = startLine; i <= endLine && i < lines.length; i++) {
        const line = lines[i];
        // let name = <expr>  or  let name: <type> = <expr>
        const m = line.match(/^\s*let\s+([A-Za-z_]\w*)\s*(?::\s*(\w+))?\s*=\s*(.+?)\s*;?\s*$/);
        if (!m) continue;
        if (m[2]) continue; // type already explicit
        const name = m[1];
        const expr = m[3].trim();
        const col = line.indexOf(name) + name.length;
        let inferred = null;
        if (/^\[\[.*\]\]$/.test(expr)) inferred = 'mat';
        else if (/^\[.*\]$/.test(expr)) inferred = 'vec';
        else if (/^-?\d+$/.test(expr)) inferred = 'int';
        else if (/^-?\d+\.\d+$/.test(expr)) inferred = 'float64';
        else if (/^"(.*)"$/.test(expr)) inferred = 'string';
        else if (/^(true|false)$/.test(expr)) inferred = 'bool';

        if (inferred) {
            hints.push(new vscode.InlayHint(
                new vscode.Position(i, col),
                `: ${inferred}`
            ));
        }
    }
    return hints;
}

module.exports = {
    provideRenameEdits,
    provideCodeLenses,
    provideInlayHints
};
