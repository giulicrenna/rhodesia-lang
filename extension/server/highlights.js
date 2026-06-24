const vscode = require('vscode');

function provideDocumentHighlights(document, position, token) {
	const wordRange = document.getWordRangeAtPosition(position);
	if (!wordRange) return [];
	const word = document.getText(wordRange);
	if (!word || word.match(/^\d+$/)) return [];

	const re = new RegExp(`\\b${word.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')}\\b`, 'g');
	const text = document.getText();
	const highlights = [];
	let m;
	while ((m = re.exec(text)) !== null) {
		const start = document.positionAt(m.index);
		const end = document.positionAt(m.index + m[0].length);
		highlights.push(new vscode.DocumentHighlight(
			new vscode.Range(start, end),
			m.index === document.offsetAt(position)
				? vscode.DocumentHighlightKind.Write
				: vscode.DocumentHighlightKind.Read
		));
	}
	return highlights;
}

module.exports = { provideDocumentHighlights };