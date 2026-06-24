const vscode = require('vscode');
const fs = require('fs');
const path = require('path');

async function findRhoFiles(root) {
	const out = [];
	async function walk(dir) {
		let entries;
		try { entries = await vscode.workspace.fs.readDirectory(vscode.Uri.file(dir)); }
		catch (_) { return; }
		for (const [name, type] of entries) {
			if (name === 'node_modules' || name === '.git' || name.startsWith('.')) continue;
			const full = path.join(dir, name);
			if (type === vscode.FileType.Directory) await walk(full);
			else if (type === vscode.FileType.File && name.endsWith('.rho')) out.push(full);
		}
	}
	await walk(root);
	return out;
}

class RhodesiaCallHierarchyProvider {
	async prepareCallHierarchy(document, position, token) {
		const wordRange = document.getWordRangeAtPosition(position);
		if (!wordRange) return null;
		const name = document.getText(wordRange);
		return new vscode.CallHierarchyItem(
			vscode.SymbolKind.Function, name, '',
			document.uri, wordRange, wordRange
		);
	}

	async provideCallHierarchyIncomingCalls(item, token) {
		const name = item.name;
		const re = new RegExp(`\\b${name.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')}\\b`);
		const calls = [];
		const folders = vscode.workspace.workspaceFolders || [];
		for (const folder of folders) {
			const files = await findRhoFiles(folder.uri.fsPath);
			for (const file of files) {
				if (token.isCancellationRequested) return calls;
				let content;
				try { content = fs.readFileSync(file, 'utf8'); }
				catch (_) { continue; }
				const lines = content.split('\n');
				for (let i = 0; i < lines.length; i++) {
					const m = lines[i].match(re);
					if (!m) continue;
					if (lines[i].match(new RegExp(`\\bfun\\s+${name}\\b`))) continue;
					const uri = vscode.Uri.file(file);
					const range = new vscode.Range(i, m.index, i, m.index + name.length);
					calls.push(new vscode.CallHierarchyIncomingCall(
						new vscode.CallHierarchyItem(
							vscode.SymbolKind.Function, name, path.basename(file), uri, range, range
						), [range]
					));
				}
			}
		}
		return calls;
	}

	async provideCallHierarchyOutgoingCalls(item, token) {
		const name = item.name;
		const filePath = item.uri.fsPath;
		let content;
		try { content = fs.readFileSync(filePath, 'utf8'); }
		catch (_) { return []; }

		const lines = content.split('\n');
		let inBody = false, braceDepth = 0;
		const called = new Set();

		for (let i = 0; i < lines.length; i++) {
			if (lines[i].match(new RegExp(`\\bfun\\s+${name}\\s*\\(`))) {
				inBody = true;
				braceDepth = (lines[i].match(/\{/g) || []).length - (lines[i].match(/\}/g) || []).length;
				continue;
			}
			if (!inBody) continue;
			braceDepth += (lines[i].match(/\{/g) || []).length;
			braceDepth -= (lines[i].match(/\}/g) || []).length;
			if (braceDepth <= 0) break;

			const callRe = /\b([A-Za-z_]\w*)\s*\(/g;
			let m;
			while ((m = callRe.exec(lines[i])) !== null) {
				if (['if', 'for', 'while', 'fun', 'return'].includes(m[1])) continue;
				called.add(m[1]);
			}
		}

		const calls = [];
		for (const fn of called) {
			const re = new RegExp(`\\bfun\\s+${fn}\\b`);
			const folders = vscode.workspace.workspaceFolders || [];
			for (const folder of folders) {
				const files = await findRhoFiles(folder.uri.fsPath);
				for (const file of files) {
					if (token.isCancellationRequested) return calls;
					let c;
					try { c = fs.readFileSync(file, 'utf8'); }
					catch (_) { continue; }
					const cl = c.split('\n');
					for (let i = 0; i < cl.length; i++) {
						if (cl[i].match(re)) {
							const uri = vscode.Uri.file(file);
							const range = new vscode.Range(i, 0, i, cl[i].length);
							calls.push(new vscode.CallHierarchyOutgoingCall(
								new vscode.CallHierarchyItem(
									vscode.SymbolKind.Function, fn, path.basename(file), uri, range, range
								), [range]
							));
							break;
						}
					}
				}
			}
		}
		return calls;
	}
}

module.exports = { RhodesiaCallHierarchyProvider };