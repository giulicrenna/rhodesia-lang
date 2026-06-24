const vscode = require('vscode');

function provideLinkedEditingRanges(document, position, token) {
	const line = document.lineAt(position).text;
	const ch = line[position.character];

	// Only handle { and } linked editing
	if (ch === '{' || ch === '}') {
		// Find matching brace
		const text = document.getText();
		const offset = document.offsetAt(position);
		const target = ch === '{' ? '}' : '{';
		const dir = ch === '{' ? 1 : -1;
		let depth = 0;
		let match = -1;

		for (let i = offset + dir; i >= 0 && i < text.length; i += dir) {
			if (text[i] === ch) depth++;
			else if (text[i] === target) {
				if (depth === 0) { match = i; break; }
				depth--;
			}
		}

		if (match !== -1) {
			const matchPos = document.positionAt(match);
			return [new vscode.Range(position, position.translate(0, 1)),
					new vscode.Range(matchPos, matchPos.translate(0, 1))];
		}
	}
	return null;
}

module.exports = { provideLinkedEditingRanges };