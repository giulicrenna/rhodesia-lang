const vscode = require('vscode');

const HEX_RE = /#([0-9a-fA-F]{3}|[0-9a-fA-F]{6})\b/g;
const RGB_RE = /\brgb\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)/g;

function provideDocumentColors(document, token) {
	const text = document.getText();
	const colors = [];

	// Hex colors
	let m;
	while ((m = HEX_RE.exec(text)) !== null) {
		const pos = document.positionAt(m.index);
		const range = new vscode.Range(pos, pos.translate(0, m[0].length));
		const hex = m[1];
		const r = parseInt(hex.length === 3 ? hex[0]+hex[0] : hex[0]+hex[1], 16);
		const g = parseInt(hex.length === 3 ? hex[1]+hex[1] : hex[2]+hex[3], 16);
		const b = parseInt(hex.length === 3 ? hex[2]+hex[2] : hex[4]+hex[5], 16);
		colors.push(new vscode.ColorInformation(range, new vscode.Color(r/255, g/255, b/255, 1)));
	}

	// rgb() colors
	while ((m = RGB_RE.exec(text)) !== null) {
		const pos = document.positionAt(m.index);
		const range = new vscode.Range(pos, pos.translate(0, m[0].length));
		const r = Math.min(255, Math.max(0, parseInt(m[1])));
		const g = Math.min(255, Math.max(0, parseInt(m[2])));
		const b = Math.min(255, Math.max(0, parseInt(m[3])));
		colors.push(new vscode.ColorInformation(range, new vscode.Color(r/255, g/255, b/255, 1)));
	}

	return colors;
}

function provideColorPresentations(color, context, token) {
	const r = Math.round(color.red * 255);
	const g = Math.round(color.green * 255);
	const b = Math.round(color.blue * 255);
	const hex = `#${r.toString(16).padStart(2, '0')}${g.toString(16).padStart(2, '0')}${b.toString(16).padStart(2, '0')}`;
	return [
		new vscode.ColorPresentation(hex),
		new vscode.ColorPresentation(`rgb(${r}, ${g}, ${b})`)
	];
}

module.exports = { provideDocumentColors, provideColorPresentations };