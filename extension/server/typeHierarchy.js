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

class RhodesiaTypeHierarchyProvider {
	async prepareTypeHierarchy(document, position, token) {
		const wordRange = document.getWordRangeAtPosition(position);
		if (!wordRange) return null;
		const name = document.getText(wordRange);
		return new vscode.TypeHierarchyItem(
			vscode.SymbolKind.Struct, name, '',
			document.uri, wordRange, wordRange
		);
	}

	async provideTypeHierarchySupertypes(item, token) {
		// No explicit inheritance in Rhodesia — return empty
		return [];
	}

	async provideTypeHierarchySubtypes(item, token) {
		const name = item.name;
		const isRecord = `record:\\s*${name}\\s*=`;
		const isEnum = `enum:\\s*${name}\\s*\\{`;
		const subtypes = [];
		const folders = vscode.workspace.workspaceFolders || [];
		for (const folder of folders) {
			const files = await findRhoFiles(folder.uri.fsPath);
			for (const file of files) {
				if (token.isCancellationRequested) return subtypes;
				let content;
				try { content = fs.readFileSync(file, 'utf8'); }
				catch (_) { continue; }
				const lines = content.split('\n');
				for (let i = 0; i < lines.length; i++) {
					const reRecord = new RegExp(`record\\s*:\\s*(\\w+)\\s*=\\s*\\{`);
					const reEnum = new RegExp(`enum\\s*:\\s*(\\w+)\\s*\\{`);
					const m = lines[i].match(reRecord) || lines[i].match(reEnum);
					if (m && m[1] !== name) {
						const uri = vscode.Uri.file(file);
						const range = new vscode.Range(i, 0, i, lines[i].length);
						subtypes.push(new vscode.TypeHierarchyItem(
							vscode.SymbolKind.Struct, m[1], '',
							uri, range, range
						));
					}
				}
			}
		}
		return subtypes;
	}
}

module.exports = { RhodesiaTypeHierarchyProvider };