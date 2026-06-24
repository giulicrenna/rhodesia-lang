const vscode = require('vscode');

function provideInlineValues(document, viewport, context, token) {
	const lines = document.getText().split('\n');
	const values = [];

	for (let i = viewport.start.line; i <= viewport.end.line && i < lines.length; i++) {
		const line = lines[i];
		// Look for variable assignments that we can evaluate statically
		const assign = line.match(/^\s*(?:let|const|int|float64|vec|mat|string|bool)\s*:\s*([A-Za-z_]\w*)\s*=\s*(.+?)\s*$/);
		if (assign) {
			const name = assign[1];
			const expr = assign[2].trim();
			const range = new vscode.Range(i, line.indexOf(name), i, line.indexOf(name) + name.length);
			values.push(new vscode.InlineValueText(range, `${name} = ${expr}`));
		}
	}
	return values;
}

module.exports = { provideInlineValues };