const vscode = require('vscode');

function provideSelectionRanges(document, positions, token) {
	const text = document.getText();
	const lines = text.split('\n');
	const pairs = [];

	// Find all brace pairs: (), {}, []
	let braceStack = [];
	for (let i = 0; i < lines.length; i++) {
		const line = lines[i];
		if (line.trim().startsWith('//')) continue;
		for (let col = 0; col < line.length; col++) {
			const ch = line[col];
			const pos = new vscode.Position(i, col);
			if (ch === '(' || ch === '{' || ch === '[') {
				braceStack.push({ ch, pos, start: pos });
			} else if (ch === ')' || ch === '}' || ch === ']') {
				const open = braceStack.pop();
				if (open && (
					(open.ch === '(' && ch === ')') ||
					(open.ch === '{' && ch === '}') ||
					(open.ch === '[' && ch === ']')
				)) {
					pairs.push({ start: open.start, end: pos });
				}
			}
		}
	}

	return positions.map(pos => {
		const selections = [];

		// Word range first
		const wordRange = document.getWordRangeAtPosition(pos);
		if (wordRange) selections.push(new vscode.SelectionRange(wordRange));

		// Then enclosing brace pair
		for (const p of pairs) {
			if (pos.isAfterOrEqual(p.start) && pos.isBeforeOrEqual(p.end)) {
				const range = new vscode.Range(p.start, p.end.translate(0, 1));
				selections.push(new vscode.SelectionRange(range, selections[0] || null));
			}
		}

		// Then line
		const lineRange = document.lineAt(pos.line).range;
		selections.push(new vscode.SelectionRange(lineRange, selections[selections.length - 1] || null));

		return new vscode.SelectionRange(
			selections[selections.length - 1].range,
			selections.length > 1 ? selections[selections.length - 2] : undefined
		);
	});
}

module.exports = { provideSelectionRanges };