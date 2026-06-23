const vscode = require('vscode');

/**
 * Minimal Rhodesia formatter.
 * - Replaces tabs with spaces (4 wide).
 * - Trims trailing whitespace per line.
 * - Collapses runs of blank lines to a single blank line.
 * - Indents lines by brace depth, with a one-level bump for lines that open
 *   a block (line ends with `{`).
 *
 * This is a regex-based formatter, not an AST-aware one. It preserves comments
 * and string contents as opaque. Good enough for a first pass.
 */
class RhodesiaFormatter {
    constructor() {
        this.indentSize = 4;
    }

    _config(document) {
        const size = vscode.workspace.getConfiguration('rhodesia', document.uri)
            .get('formatter.indentSize', 4);
        return Math.max(1, Math.min(16, size | 0));
    }

    format(document, range) {
        const indentSize = this._config(document);
        const fullText = document.getText();
        const startOffset = document.offsetAt(range.start);
        const endOffset = document.offsetAt(range.end);
        const isFull = startOffset === 0 && endOffset === fullText.length;

        // Expand to whole lines so brace tracking works across the selection.
        const startLine = range.start.line;
        const endLine = range.end.line;
        const startPos = new vscode.Position(startLine, 0);
        const endPos = new vscode.Position(endLine, document.lineAt(endLine).text.length);
        const block = document.getText(new vscode.Range(startPos, endPos));
        const formatted = this._formatBlock(block, indentSize);

        return [vscode.TextEdit.replace(new vscode.Range(startPos, endPos), formatted)];
    }

    _formatBlock(text, indentSize) {
        const lines = text.split('\n');
        const out = [];
        let depth = 0;
        let lastBlank = false;

        for (let raw of lines) {
            let line = raw.replace(/\t/g, ' '.repeat(indentSize)).replace(/[ \t]+$/g, '');

            // Track opening/closing braces at the start of the line
            const trimmed = line.trim();
            const isBlank = trimmed.length === 0;

            if (!isBlank) {
                // Closing brace at the very start: dedent before printing
                let leading = '';
                while (line.length > 0 && line[0] === ' ') leading += ' ';
                const closeCount = (trimmed.match(/^\}/g) || []).length;
                const effectiveDepth = Math.max(0, depth - closeCount);
                line = ' '.repeat(effectiveDepth * indentSize) + trimmed;

                // Bump depth for lines that open a block (end with `{`)
                if (line.endsWith('{')) depth += (line.match(/\{/g) || []).length;
                if (closeCount > 0) depth = Math.max(0, depth);
            }

            if (isBlank) {
                if (!lastBlank && out.length > 0) out.push('');
                lastBlank = true;
            } else {
                out.push(line);
                lastBlank = false;
            }
        }

        // Strip trailing blank lines
        while (out.length > 0 && out[out.length - 1] === '') out.pop();

        // Preserve leading newlines (when formatting a non-full range starting
        // mid-file, keep one blank if the original started blank).
        const leadingBlanks = lines.findIndex(l => l.trim().length > 0);
        const leading = leadingBlanks > 0 ? lines.slice(0, leadingBlanks) : [];

        return (leading.length ? leading.join('\n') + '\n' : '') + out.join('\n') + '\n';
    }
}

module.exports = { RhodesiaFormatter };
