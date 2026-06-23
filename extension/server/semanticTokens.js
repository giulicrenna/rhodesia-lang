const vscode = require('vscode');

/**
 * Semantic tokens for Rhodesia.
 *
 * Encoded as the standard 5-int-per-token delta format:
 *   [deltaLine, deltaStartChar, length, tokenType, tokenModifiers]
 *
 * The token-type and token-modifier ids MUST align with the legend passed to
 * `registerSemanticTokensProvider` and with the `contributes.semanticTokenTypes`
 * / `semanticTokenModifiers` arrays in package.json.
 */
const TOKEN_TYPES = [
    'function',   // 0
    'variable',   // 1
    'parameter',  // 2
    'type',       // 3
    'namespace',  // 4
    'constant'    // 5
];
const TOKEN_MODIFIERS = [
    'declaration', // 0
    'readonly'     // 1
];

const TYPES = new Set(['int', 'float64', 'vec', 'mat', 'string', 'void', 'bool']);

function provideSemanticTokens(document) {
    const builder = new vscode.SemanticTokensBuilder(
        new vscode.SemanticTokensLegend(TOKEN_TYPES, TOKEN_MODIFIERS)
    );
    const lines = document.getText().split('\n');

    for (let i = 0; i < lines.length; i++) {
        const line = lines[i];
        if (line.trim().startsWith('//')) continue;

        // Function declaration: `fun name(...) -> type {`
        const funDecl = line.match(/\bfun\s+(\w+)\s*\(/);
        if (funDecl) {
            const name = funDecl[1];
            const col = line.indexOf(name, funDecl.index);
            builder.push(i, col, name.length, 0 /*function*/, 1 << 0 /*declaration*/);
        }

        // Function parameters inside the parens: accept "type: name" or "name: type"
        const funSig = line.match(/\bfun\s+\w+\s*\(([^)]*)\)/);
        if (funSig) {
            const openParen = line.indexOf('(', line.indexOf('fun'));
            for (const param of funSig[1].split(',').map(p => p.trim()).filter(Boolean)) {
                const m = param.match(/^(\w+)\s*:\s*(\w+)$/);
                if (!m) continue;
                const [, a, b] = m;
                let typeWord, nameWord;
                if (TYPES.has(a)) { typeWord = a; nameWord = b; }
                else if (TYPES.has(b)) { typeWord = b; nameWord = a; }
                else continue;
                const nameCol = line.indexOf(nameWord, openParen);
                if (nameCol !== -1) {
                    builder.push(i, nameCol, nameWord.length, 2 /*parameter*/, 1 << 0);
                }
                const typeCol = line.indexOf(typeWord, openParen);
                if (typeCol !== -1 && typeCol !== nameCol) {
                    builder.push(i, typeCol, typeWord.length, 3 /*type*/, 0);
                }
            }
        }

        // Variable declaration: "type: name = ..." (skip when in fun signature)
        const varDeclType = line.match(/\b(int|float64|vec|mat|string|bool)\s*:\s*([A-Za-z_]\w*)/);
        if (varDeclType && !funSig) {
            const typeWord = varDeclType[1];
            const nameWord = varDeclType[2];
            const typeCol = line.indexOf(typeWord);
            if (typeCol !== -1) {
                builder.push(i, typeCol, typeWord.length, 3 /*type*/, 0);
            }
            const nameCol = line.indexOf(nameWord, typeCol + typeWord.length);
            if (nameCol !== -1) {
                builder.push(i, nameCol, nameWord.length, 1 /*variable*/, 1 << 0);
            }
        }

        // let / const declaration
        const letDecl = line.match(/\b(let|const)\s+([A-Za-z_]\w*)/);
        if (letDecl) {
            const nameWord = letDecl[2];
            const col = line.indexOf(nameWord, letDecl.index);
            const mod = letDecl[1] === 'const'
                ? (1 << 0) | (1 << 1)               // declaration + readonly
                : (1 << 0);                          // declaration
            builder.push(i, col, nameWord.length, 1 /*variable*/, mod);
        }

        // Module-qualified namespace: "math" in "math.zeros("
        const moduleRef = line.match(/\b(math|stats|numerical|linear_algebra|core)\.(\w+)/);
        if (moduleRef) {
            const modName = moduleRef[1];
            const funcName = moduleRef[2];
            const modCol = line.indexOf(modName);
            builder.push(i, modCol, modName.length, 4 /*namespace*/, 0);
            const funcCol = line.indexOf(funcName, modCol + modName.length + 1);
            if (funcCol !== -1 && line[funcCol + funcName.length] === '(') {
                builder.push(i, funcCol, funcName.length, 0 /*function*/, 0);
            }
        }

        // Return type after `->`
        const retMatch = line.match(/->\s*(\w+)/);
        if (retMatch) {
            const typeWord = retMatch[1];
            if (TYPES.has(typeWord)) {
                const col = line.indexOf(typeWord, line.indexOf('->'));
                if (col !== -1) {
                    builder.push(i, col, typeWord.length, 3 /*type*/, 0);
                }
            }
        }
    }

    return builder.build();
}

module.exports = {
    TOKEN_TYPES,
    TOKEN_MODIFIERS,
    provideSemanticTokens
};
