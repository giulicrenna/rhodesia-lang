"use strict";
const vscode = require('vscode');
const { builtinFunctions, moduleDescriptors } = require('./builtinDescriptors');
const { getAvailableLibPaths, parseIncludedFile } = require('./doxygenParser');
// Pre-compute available include paths at activation time
const availableLibPaths = getAvailableLibPaths();
class RhodesiaAutocompleteProvider {
    constructor() {
        this.keywords = [
            'if', 'else', 'for', 'while', 'break', 'continue', 'return', 'in', 'fun', 'include'
        ];
        this.types = [
            'int', 'float64', 'vec', 'mat', 'string', 'void', 'bool'
        ];
        this.builtinFunctions = builtinFunctions;
        this.moduleDescriptors = moduleDescriptors;
        this.logicalOperators = [
            'and', 'or', 'not'
        ];
        this.moduleNames = Object.keys(moduleDescriptors);
        // Cache for parsed include files within a session
        this._includeCache = new Map();
    }
    /**
     * Parse the include statements from the document text and return
     * the union of { functions, constants } from all included files.
     */
    _getIncludedSymbols(documentText) {
        const functions = [];
        const constants = [];
        const includeRe = /^include\s+(\S+)/gm;
        let match;
        while ((match = includeRe.exec(documentText)) !== null) {
            const includePath = match[1].trim();
            if (!this._includeCache.has(includePath)) {
                this._includeCache.set(includePath, parseIncludedFile(includePath));
            }
            const symbols = this._includeCache.get(includePath);
            functions.push(...(symbols.functions || []));
            constants.push(...(symbols.constants || []));
        }
        return { functions, constants };
    }
    provideCompletionItems(document, position, token, context) {
        const linePrefix = document.lineAt(position).text.substring(0, position.character);
        const items = [];
        // ----------------------------------------------------------------
        // Include path completions: line starts with "include "
        // e.g. `include math/core/` → suggest matching paths
        // ----------------------------------------------------------------
        const includeMatch = linePrefix.match(/^include\s+(.*)$/);
        if (includeMatch) {
            const typed = includeMatch[1];
            const typedLower = typed.toLowerCase();
            availableLibPaths.forEach(libPath => {
                if (libPath.toLowerCase().startsWith(typedLower)) {
                    const item = new vscode.CompletionItem(libPath, vscode.CompletionItemKind.Module);
                    item.detail = 'Include path';
                    item.documentation = new vscode.MarkdownString(`Include \`${libPath}\`\n\nImports functions and constants from this library file.`);
                    item.insertText = libPath;
                    // filterText uses the full path so VS Code fuzzy-matches the entire
                    // "math/numer" query against "math/numerical/..." correctly,
                    // even when the filter has only seen the last word segment.
                    item.filterText = libPath;
                    // Replace from the start of the typed partial path
                    const startChar = linePrefix.length - typed.length;
                    item.range = new vscode.Range(position.line, startChar, position.line, position.character);
                    items.push(item);
                }
            });
            return items;
        }
        // ----------------------------------------------------------------
        // Module-qualified completions: detect "module." at cursor
        // ----------------------------------------------------------------
        const moduleMatch = linePrefix.match(/\b(\w+)\.\s*$/);
        if (moduleMatch) {
            const moduleName = moduleMatch[1];
            const mod = this.moduleDescriptors[moduleName];
            if (mod) {
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
        // ----------------------------------------------------------------
        // Global completions: keywords, types, builtins, modules, operators
        // ----------------------------------------------------------------
        this.keywords.forEach(keyword => {
            items.push(new vscode.CompletionItem(keyword, vscode.CompletionItemKind.Keyword));
        });
        this.types.forEach(type => {
            items.push(new vscode.CompletionItem(type, vscode.CompletionItemKind.Class));
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
            item.documentation = new vscode.MarkdownString(`Built-in module \`${modName}\`. Type \`${modName}.\` to see its functions.`);
            items.push(item);
        });
        this.logicalOperators.forEach(op => {
            items.push(new vscode.CompletionItem(op, vscode.CompletionItemKind.Operator));
        });
        // ----------------------------------------------------------------
        // Symbols from included files (functions and constants)
        // ----------------------------------------------------------------
        const documentText = document.getText();
        const { functions: incFuncs, constants: incConsts } = this._getIncludedSymbols(documentText);
        const seenLabels = new Set(this.builtinFunctions.map(f => f.label));
        incFuncs.forEach(func => {
            if (seenLabels.has(func.label))
                return; // already in builtins
            seenLabels.add(func.label);
            const item = new vscode.CompletionItem(func.label, vscode.CompletionItemKind.Function);
            item.detail = func.detail;
            item.documentation = new vscode.MarkdownString(func.documentation);
            items.push(item);
        });
        incConsts.forEach(constant => {
            if (seenLabels.has(constant.label))
                return;
            seenLabels.add(constant.label);
            const item = new vscode.CompletionItem(constant.label, vscode.CompletionItemKind.Constant);
            item.detail = constant.detail;
            item.documentation = new vscode.MarkdownString(`**${constant.label}** = \`${constant.value}\`\n\n${constant.documentation}`);
            items.push(item);
        });
        // Context: suggest return types after "fun ... ->"
        if (linePrefix.includes('fun ') && !linePrefix.includes('->')) {
            this.types.forEach(type => {
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
