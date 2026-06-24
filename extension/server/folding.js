const vscode = require('vscode');

function provideFoldingRanges(document, context, token) {
	const text = document.getText();
	const ranges = [];

	// Fold block comments: /* ... */
	const blockRe = /\/\*[\s\S]*?\*\//g;
	let m;
	while ((m = blockRe.exec(text)) !== null) {
		const start = document.positionAt(m.index);
		const end = document.positionAt(m.index + m[0].length);
		ranges.push(new vscode.FoldingRange(
			start.line,
			end.line,
			vscode.FoldingRangeKind.Comment
		));
	}

	// Fold braces { } — track depth
	const lines = text.split('\n');
	const stack = []; // { startLine }
	for (let i = 0; i < lines.length; i++) {
		const trimmed = lines[i].trim();
		// Skip comments
		if (trimmed.startsWith('//') || trimmed.startsWith('/*')) continue;
		for (const ch of lines[i]) {
			if (ch === '{') {
				stack.push({ startLine: i });
			} else if (ch === '}') {
				const open = stack.pop();
				if (open && i - open.startLine > 1) {
					ranges.push(new vscode.FoldingRange(
						open.startLine,
						i,
						vscode.FoldingRangeKind.Region
					));
				}
			}
		}
	}

	// Fold fun declarations without body (single-line)
	// Already covered by brace folding above

	return ranges;
}

module.exports = { provideFoldingRanges };