const vscode = require('vscode');
const path = require('path');
const fs = require('fs');
const { parseSymbols, getAvailableLibPaths } = require('./doxygenParser');

const LIBS_BASE = path.resolve(__dirname, '../../libs');

const _symbolCache = new Map();

function _getCachedSymbols(filePath) {
    let stat;
    try { stat = fs.statSync(filePath); }
    catch (_) { return null; }
    const cached = _symbolCache.get(filePath);
    if (cached && cached.mtime === stat.mtimeMs) return cached.symbols;
    const symbols = parseSymbols(filePath);
    _symbolCache.set(filePath, { symbols, mtime: stat.mtimeMs });
    return symbols;
}

/**
 * Walk a file tree (bounded to `root` to avoid scanning node_modules) and
 * yield `.rho` files. Excludes anything in node_modules and .git.
 */
async function findRhoFiles(root) {
    const out = [];
    async function walk(dir) {
        let entries;
        try { entries = await vscode.workspace.fs.readDirectory(vscode.Uri.file(dir)); }
        catch (_) { return; }
        for (const [name, type] of entries) {
            if (name === 'node_modules' || name === '.git' || name.startsWith('.')) continue;
            const full = path.join(dir, name);
            if (type === vscode.FileType.Directory) {
                await walk(full);
            } else if (type === vscode.FileType.File && name.endsWith('.rho')) {
                out.push(full);
            }
        }
    }
    await walk(root);
    return out;
}

/**
 * Parse a single .rho file from disk and return its DocumentSymbols.
 * If a query is given, only return symbols whose name contains the query.
 */
async function symbolsForFile(filePath, query = '') {
    const q = query.toLowerCase();
    const { functions, constants } = parseSymbols(filePath);
    const symbols = [];

    for (const fn of functions) {
        if (q && !fn.label.toLowerCase().includes(q)) continue;
        const line = fn._line || 0;
        const range = new vscode.Range(line, 0, line, 0);
        symbols.push(new vscode.SymbolInformation(
            fn.label,
            vscode.SymbolKind.Function,
            path.basename(filePath),
            new vscode.Location(vscode.Uri.file(filePath), range)
        ));
    }
    for (const c of constants) {
        if (q && !c.label.toLowerCase().includes(q)) continue;
        const line = c._line || 0;
        const range = new vscode.Range(line, 0, line, 0);
        symbols.push(new vscode.SymbolInformation(
            c.label,
            vscode.SymbolKind.Constant,
            path.basename(filePath),
            new vscode.Location(vscode.Uri.file(filePath), range)
        ));
    }
    return symbols;
}

/**
 * Workspace symbol provider: searches every `.rho` file in the workspace.
 * Uses mtime-based cache — re-parses only when file changed.
 */
async function provideWorkspaceSymbols(query, token) {
    const folders = vscode.workspace.workspaceFolders || [];
    if (folders.length === 0) return [];

    const all = [];
    for (const folder of folders) {
        const files = await findRhoFiles(folder.uri.fsPath);
        for (const file of files) {
            if (token.isCancellationRequested) return all;
            const syms = _getCachedSymbols(file);
            if (!syms) continue;
            const q = query.toLowerCase();
            for (const fn of syms.functions || []) {
                if (q && !fn.label.toLowerCase().includes(q)) continue;
                const line = fn._line || 0;
                all.push(new vscode.SymbolInformation(
                    fn.label, vscode.SymbolKind.Function,
                    path.basename(file),
                    new vscode.Location(vscode.Uri.file(file), new vscode.Range(line,0,line,0))
                ));
            }
            for (const c of syms.constants || []) {
                if (q && !c.label.toLowerCase().includes(q)) continue;
                const line = c._line || 0;
                all.push(new vscode.SymbolInformation(
                    c.label, vscode.SymbolKind.Constant,
                    path.basename(file),
                    new vscode.Location(vscode.Uri.file(file), new vscode.Range(line,0,line,0))
                ));
            }
        }
    }
    return all;
}

/**
 * Reference provider: find every textual occurrence of `word` across the
 * workspace's .rho files. Matches whole-word only.
 */
async function provideReferences(document, position, context, token) {
    const wordRange = document.getWordRangeAtPosition(position);
    if (!wordRange) return [];
    const word = document.getText(wordRange);
    const re = new RegExp(`\\b${word.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')}\\b`);
    const out = [];
    const folders = vscode.workspace.workspaceFolders || [];
    for (const folder of folders) {
        const files = await findRhoFiles(folder.uri.fsPath);
        for (const file of files) {
            if (token.isCancellationRequested) return out;
            let content;
            try { content = require('fs').readFileSync(file, 'utf8'); }
            catch (_) { continue; }
            const lines = content.split('\n');
            for (let i = 0; i < lines.length; i++) {
                const m = lines[i].match(re);
                if (!m) continue;
                const col = m.index;
                out.push(new vscode.Location(
                    vscode.Uri.file(file),
                    new vscode.Range(i, col, i, col + word.length)
                ));
            }
        }
    }
    return out;
}

module.exports = {
    findRhoFiles,
    symbolsForFile,
    provideWorkspaceSymbols,
    provideReferences
};
