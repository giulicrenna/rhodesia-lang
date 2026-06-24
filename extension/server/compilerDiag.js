const vscode = require('vscode');
const { exec } = require('child_process');

const RHODESIA_BIN = 'C:\\Program Files\\Rhodesia\\bin\\rhodesia.exe';

/**
 * Run rhodesia.exe on the given file and parse errors into VS Code diagnostics.
 * Errors expected format:
 *   filename:line:col: error|warning: message
 *   filename:line:col: error: ...  (single char column for now)
 */
function runCompiler(document) {
	return new Promise((resolve) => {
		const filePath = document.uri.fsPath;
		exec(`"${RHODESIA_BIN}" "${filePath}" 2>&1`, { timeout: 10000 }, (err, stdout, stderr) => {
			const output = stdout || stderr || '';
			const lines = output.split('\n').filter(Boolean);
			const diagnostics = [];

			const re = /^(?:.*?):(\d+):(\d+):\s*(error|warning):\s*(.+)$/;
			for (const line of lines) {
				const m = line.match(re);
				if (!m) continue;
				const lineNum = parseInt(m[1], 10) - 1;
				const colNum = Math.max(0, parseInt(m[2], 10) - 1);
				const severity = m[3] === 'error' ? vscode.DiagnosticSeverity.Error : vscode.DiagnosticSeverity.Warning;
				const message = m[4];

				const range = new vscode.Range(
					new vscode.Position(lineNum, colNum),
					new vscode.Position(lineNum, colNum + 1)
				);
				diagnostics.push(new vscode.Diagnostic(range, message, severity));
			}

			resolve(diagnostics);
		});
	});
}

module.exports = { runCompiler };