const fs = require('fs');
const path = require('path');
const { findPep8Doc } = require('./docstringParser');

const LIBS_BASE = path.resolve(__dirname, '../../libs');

function normalizeWhitespace(text) {
    return text.replace(/\s+/g, ' ').trim();
}

function parseParams(paramLine) {
    if (!paramLine.trim()) return [];
    return paramLine.split(',').map(param => param.trim()).filter(Boolean);
}

/**
 * Clean a raw doxygen block line: remove /**, * prefix, and closing * /
 */
function cleanDoxyLine(line) {
    return line
        .replace(/^\s*\/\*+\s*/, '')   // Remove opening /** or /*
        .replace(/\s*\*+\/\s*$/, '')   // Remove closing */
        .replace(/^\s*\*\s?/, '')       // Remove leading * in body lines
        .trim();
}

function parseDoxygenBlock(block) {
    const lines = block
        .split('\n')
        .map(cleanDoxyLine)
        .filter(Boolean);

    const summaryParts = [];
    const paramDocs = {};   // { paramName -> description }
    let returns = '';

    lines.forEach(line => {
        if (line.startsWith('@brief')) {
            summaryParts.push(line.replace(/^@brief\s*/, ''));
        } else if (line.startsWith('@details')) {
            summaryParts.push(line.replace(/^@details\s*/, ''));
        } else if (line.startsWith('@param')) {
            // "@param name description" or "@param name"
            const m = line.replace(/^@param\s*/, '').match(/^(\w+)\s*(.*)/);
            if (m) paramDocs[m[1]] = m[2].trim();
        } else if (line.startsWith('@return')) {
            returns = line.replace(/^@returns?\s*/, '').trim();
        }
        // @file, @brief (already handled), other @ tags → skip
    });

    return {
        summary: normalizeWhitespace(summaryParts.join(' ')),
        paramDocs,
        returns: normalizeWhitespace(returns)
    };
}

function buildSignature(label, params, returnType) {
    const paramText = params.length ? params.join(', ') : '';
    return returnType ? `${label}(${paramText}) -> ${returnType}` : `${label}(${paramText})`;
}

/**
 * Find the doxygen block immediately preceding line i in the lines array.
 * Returns the parsed doc object or null.
 */
function findPrecedingDoc(lines, i) {
    for (let j = i - 1; j >= 0; j--) {
        const trimmed = lines[j].trim();
        if (trimmed === '') continue;
        if (trimmed.endsWith('*/')) {
            const blockLines = [];
            for (let k = j; k >= 0; k--) {
                blockLines.unshift(lines[k]);
                if (lines[k].trim().startsWith('/**')) {
                    return parseDoxygenBlock(blockLines.join('\n'));
                }
            }
        }
        break; // First non-empty line is not */, no doc
    }
    // Fallback to PEP8-style docstring
    const pep8 = findPep8Doc(lines, i);
    if (pep8) {
        return { summary: pep8, paramDocs: {}, returns: '' };
    }
    return null;
}

/**
 * Parse a .rho file and return { functions, constants, structs, enums }.
 * - functions: array of descriptor objects (with _source, _line)
 * - constants: array of { label, type, value, documentation, _source, _line }
 * - structs:   array of { label, fields, _source, _line }
 * - enums:     array of { label, members, _source, _line }
 */
function parseSymbols(filePath) {
    if (!fs.existsSync(filePath)) return { functions: [], constants: [], structs: [], enums: [] };
    return parseDocumentText(fs.readFileSync(filePath, 'utf8'), filePath);
}

/**
 * In-memory variant of parseSymbols — takes raw text + a virtual source path
 * so the caller can pass the current document without touching disk.
 */
function parseDocumentText(text, virtualPath) {
    const lines = text.split('\n');
    const functions = [];
    const constants = [];
    const structs = [];
    const enums = [];

    for (let i = 0; i < lines.length; i++) {
        const line = lines[i];

        // ---- Functions: fun name(...) -> type  OR  fun name(...) { ----
        const funMatch = line.match(/\bfun\s+(\w+)\s*\(([^)]*)\)(?:\s*->\s*([^\s{]+))?/);
        if (funMatch) {
            const label = funMatch[1];
            const paramText = funMatch[2];
            const returnType = funMatch[3] || '';
            const params = parseParams(paramText);
            const doc = findPrecedingDoc(lines, i) || { summary: '', paramDocs: {}, returns: '' };
            const signature = buildSignature(label, params, returnType);

            const paramInfos = params.map(param => {
                const nameMatch = param.match(/(\w+)\s*$/);
                const paramName = nameMatch ? nameMatch[1] : param;
                return {
                    label: param,
                    documentation: doc.paramDocs[paramName] || ''
                };
            });

            functions.push({
                label,
                detail: signature,
                documentation: doc.summary || 'No documentation provided.',
                signatures: [{
                    label: signature,
                    documentation: doc.summary || 'No documentation provided.',
                    parameters: paramInfos
                }],
                _source: virtualPath,
                _line: i,
                _return: doc.returns
            });
            continue;
        }

        // ---- Record struct: record: Name = { field: type, ... } ----
        // Must come BEFORE the constant regex so `record: foo = {...}` is
        // recognized as a struct literal, not a `record` constant.
        const structMatch = line.match(/^\s*record\s*:\s*(\w+)\s*=\s*\{/);
        if (structMatch) {
            const label = structMatch[1];
            const fields = [];
            let depth = 0;
            // Capture inline field declarations on the opening line:
            //   record: prod1 = { nombre: "x", precio: 1.20, stock: 100 }
            const inlineFields = line.match(/\{(.+)\}/);
            if (inlineFields) {
                const parts = inlineFields[1].split(',');
                for (const p of parts) {
                    const fm = p.match(/^\s*(\w+)\s*:\s*(.+?)\s*$/);
                    if (fm) fields.push({ name: fm[1], value: fm[2] });
                }
                // Single-line struct — done
                const doc = findPrecedingDoc(lines, i) || { summary: '', paramDocs: {}, returns: '' };
                structs.push({
                    label,
                    fields: fields.map(f => f.name),
                    fieldDefs: fields,
                    documentation: doc.summary || `Record with fields: ${fields.map(f => f.name).join(', ')}`,
                    _source: virtualPath,
                    _line: i
                });
                continue;
            }
            // Multi-line struct: walk lines until the matching closing brace
            for (const ch of line) { if (ch === '{') depth++; else if (ch === '}') depth--; }
            let j = i + 1;
            while (j < lines.length && depth > 0) {
                const fl = lines[j];
                for (const ch of fl) { if (ch === '{') depth++; else if (ch === '}') depth--; }
                if (depth > 0) {
                    const fm = fl.match(/^\s*(\w+)\s*:\s*(.+?)(?:,|\s*$)/);
                    if (fm) fields.push({ name: fm[1], value: fm[2].trim() });
                }
                j++;
            }
            const doc = findPrecedingDoc(lines, i) || { summary: '', paramDocs: {}, returns: '' };
            structs.push({
                label,
                fields: fields.map(f => f.name),
                fieldDefs: fields,
                documentation: doc.summary || `Record with fields: ${fields.map(f => f.name).join(', ')}`,
                _source: virtualPath,
                _line: i
            });
            i = j - 1; // skip past the closing brace
            continue;
        }

        // ---- Enum: enum: Name { Variant1, Variant2, ... } ----
        const enumMatch = line.match(/^\s*enum\s*:\s*(\w+)\s*\{/);
        if (enumMatch) {
            const label = enumMatch[1];
            const members = [];
            // Capture inline variants
            const inlineMembers = line.match(/\{(.+)\}/);
            if (inlineMembers) {
                inlineMembers[1].split(',').forEach(m => {
                    const t = m.trim();
                    if (t) members.push(t);
                });
                const doc = findPrecedingDoc(lines, i) || { summary: '', paramDocs: {}, returns: '' };
                enums.push({
                    label,
                    members,
                    documentation: doc.summary || `Enum with members: ${members.join(', ')}`,
                    _source: virtualPath,
                    _line: i
                });
                continue;
            }
            let depth = 0;
            for (const ch of line) { if (ch === '{') depth++; else if (ch === '}') depth--; }
            let j = i + 1;
            while (j < lines.length && depth > 0) {
                const el = lines[j];
                for (const ch of el) { if (ch === '{') depth++; else if (ch === '}') depth--; }
                if (depth > 0) {
                    const em = el.match(/^\s*(\w+)/);
                    if (em) members.push(em[1]);
                }
                j++;
            }
            const doc = findPrecedingDoc(lines, i) || { summary: '', paramDocs: {}, returns: '' };
            enums.push({
                label,
                members,
                documentation: doc.summary || `Enum with members: ${members.join(', ')}`,
                _source: virtualPath,
                _line: i
            });
            i = j - 1; // skip past the closing brace
            continue;
        }

        // ---- Destructure: (type: a, type: b) = expr   OR   (a, b) = expr ----
        // Must come BEFORE the let/const/type branches so `(mat: L, mat: U) = ...`
        // is not misread as a `(` and skipped.
        const destructureMatch = line.match(/^\s*\(([^)]+)\)\s*=\s*(.+?)\s*$/);
        if (destructureMatch) {
            const expr = destructureMatch[2].trim();
            const doc = findPrecedingDoc(lines, i) || { summary: '', paramDocs: {}, returns: '' };
            const bindings = destructureMatch[1].split(',').map(s => s.trim()).filter(Boolean);
            for (const b of bindings) {
                const tm = b.match(/^(\w+)\s*:\s*([A-Za-z_]\w*)\s*(?:as\s+([A-Za-z_]\w*))?\s*$/);
                const nm = !tm && b.match(/^([A-Za-z_]\w*)\s*(?:as\s+([A-Za-z_]\w*))?\s*$/);
                let type = '', name = '', alias = '';
                if (tm) { type = tm[1]; name = tm[2]; alias = tm[3] || ''; }
                else if (nm) { name = nm[1]; alias = nm[2] || ''; }
                else continue;
                constants.push({
                    label: alias || name,
                    type: type || '(inferred)',
                    value: `<destructured from ${expr}>`,
                    documentation: doc.summary || `Local binding from destructure.`,
                    detail: `${type ? type + ': ' : ''}${name}${alias ? ' as ' + alias : ''} = ${expr}`,
                    kind: 'destructure',
                    _source: virtualPath,
                    _line: i
                });
            }
            continue;
        }

        // ---- let / const declarations: optional `: type`, optional `as alias` ----
        const letMatch = line.match(/^\s*(let|const)\s+([A-Za-z_]\w*)(?:\s*:\s*(\w+))?\s*=\s*(.+?)\s*$/);
        if (letMatch) {
            const kw = letMatch[1];
            const name = letMatch[2];
            const explicitType = letMatch[3] || '';
            const value = letMatch[4].trim();
            const doc = findPrecedingDoc(lines, i) || { summary: '', paramDocs: {}, returns: '' };
            constants.push({
                label: name,
                type: explicitType || 'inferred',
                value,
                documentation: doc.summary || (kw === 'const' ? 'Constant.' : 'Variable.'),
                detail: `${kw} ${name}${explicitType ? ': ' + explicitType : ''} = ${value}`,
                kind: kw === 'const' ? 'const' : 'let',
                _source: virtualPath,
                _line: i
            });
            continue;
        }

        // ---- Constants/Variables: type: NAME = value (ALL_CAPS = constant) ----
        // Anchor relaxed to ^\s* so indented declarations inside fun bodies parse.
        const constMatch = line.match(/^\s*(float64|int|int8|int16|int32|int64|uint|uint8|uint16|uint32|uint64|byte|complex|bool|str|string|vec|mat|map|arr|set|tuple|function|void|null)\s*:\s*([A-Za-z_]\w*)\s*=\s*(.+?)\s*$/);
        if (constMatch) {
            const type = constMatch[1];
            const label = constMatch[2];
            const value = constMatch[3].trim();
            const doc = findPrecedingDoc(lines, i) || { summary: '', paramDocs: {}, returns: '' };
            constants.push({
                label,
                type,
                value,
                documentation: doc.summary || `${type} constant.`,
                detail: `${type}: ${label} = ${value}`,
                kind: 'type',
                _source: virtualPath,
                _line: i
            });
        }
    }

    return { functions, constants, structs, enums };
}

/**
 * Backwards-compatible wrapper: parse only functions from a file (like old parseFile).
 */
function parseFile(filePath) {
    return parseSymbols(filePath).functions;
}

/**
 * Walk the libs/ directory and return all available include paths.
 * Library short names (e.g. "math") are included when the library has an
 * `index.rho` entry-point. All sub-paths are returned too:
 *   "math", "math/core/core", "math/linear_algebra/matrices", ...
 */
function getAvailableLibPaths() {
    const paths = [];
    if (!fs.existsSync(LIBS_BASE)) return paths;

    // First pass: top-level library dirs.
    let entries;
    try { entries = fs.readdirSync(LIBS_BASE, { withFileTypes: true }); }
    catch (_) { return paths; }
    for (const entry of entries) {
        if (!entry.isDirectory()) continue;
        if (entry.name.startsWith('.')) continue;
        const libPath = path.join(LIBS_BASE, entry.name);
        // If the library exposes an index.rho, register the short name.
        if (fs.existsSync(path.join(libPath, 'index.rho'))) {
            paths.push(entry.name);
        }
    }

    // Second pass: full recursive walk for granular includes.
    function walk(dir, prefix) {
        let es;
        try { es = fs.readdirSync(dir, { withFileTypes: true }); }
        catch (_) { return; }
        for (const entry of es) {
            const rel = prefix ? `${prefix}/${entry.name}` : entry.name;
            if (entry.isDirectory()) {
                walk(path.join(dir, entry.name), rel);
            } else if (entry.name.endsWith('.rho')) {
                const p = rel.replace(/\.rho$/, '');
                if (!paths.includes(p)) paths.push(p);
            }
        }
    }
    walk(LIBS_BASE, '');

    return paths;
}

/**
 * Given an include path like "math/core/core", resolve it to the .rho file
 * and return its parsed symbols { functions, constants }.
 */
function parseIncludedFile(includePath) {
    const filePath = path.join(LIBS_BASE, includePath + '.rho');
    return parseSymbols(filePath);
}

/**
 * Parse a single `include ...` line and return the structured form.
 *   { path, selected: null }            -> `include math`           (import all)
 *   { path, selected: 'all' }           -> `include math{}`         (import all)
 *   { path, selected: [{label, alias?}, ...] }  -> `include math{a, b as c}`
 * Returns null if the line isn't an include.
 */
function parseIncludeStatement(line) {
    const m = line.match(/^\s*include\s+([A-Za-z_][\w/]*)(?:\{([^}]*)\})?/);
    if (!m) return null;
    const includePath = m[1];
    if (m[2] === undefined) return { path: includePath, selected: null };
    const inner = m[2].trim();
    if (inner === '') return { path: includePath, selected: 'all' };
    const items = inner.split(',').map(s => s.trim()).filter(Boolean).map(s => {
        const am = s.match(/^([A-Za-z_]\w*)\s*(?:as\s+([A-Za-z_]\w*))?\s*$/);
        if (!am) return null;
        return am[2] ? { label: am[1], alias: am[2] } : { label: am[1] };
    }).filter(Boolean);
    return { path: includePath, selected: items };
}

/**
 * Given full document text and an alias name, find which include statement
 * introduced it and return { modulePath, originalLabel } or null.
 * Used by hover and definition to resolve `MODULE_PI as PI` style imports.
 */
function resolveIncludeAlias(docText, aliasName) {
    const re = /^\s*include\s+([A-Za-z_][\w/]*)\{([^}]*)\}/gm;
    let m;
    while ((m = re.exec(docText)) !== null) {
        const modulePath = m[1];
        for (const raw of m[2].split(',').map(s => s.trim()).filter(Boolean)) {
            const am = raw.match(/^([A-Za-z_]\w*)\s*(?:as\s+([A-Za-z_]\w*))?\s*$/);
            if (!am) continue;
            if (am[2] && am[2] === aliasName) return { modulePath, originalLabel: am[1] };
        }
    }
    return null;
}

/**
 * Parse every .rho file under libs/<moduleName>/ and return its functions.
 * Walks the filesystem — no hard-coded submodule layout.
 */
function parseModuleFunctions(moduleName) {
    const basePath = path.join(LIBS_BASE, moduleName);
    if (!fs.existsSync(basePath) || !fs.statSync(basePath).isDirectory()) return [];

    const functions = [];
    function walk(dir) {
        let entries;
        try { entries = fs.readdirSync(dir, { withFileTypes: true }); }
        catch (_) { return; }
        for (const entry of entries) {
            const full = path.join(dir, entry.name);
            if (entry.isDirectory()) {
                walk(full);
            } else if (entry.name.endsWith('.rho') && entry.name !== 'index.rho') {
                functions.push(...parseFile(full));
            }
        }
    }
    walk(basePath);
    return functions;
}

function buildModuleDescriptors(baseDescriptors) {
    const updatedDescriptors = { ...baseDescriptors };
    if (!fs.existsSync(LIBS_BASE)) return updatedDescriptors;

    let entries;
    try { entries = fs.readdirSync(LIBS_BASE, { withFileTypes: true }); }
    catch (_) { return updatedDescriptors; }
    for (const entry of entries) {
        if (!entry.isDirectory() || entry.name.startsWith('.')) continue;
        const libName = entry.name;
        const existing = updatedDescriptors[libName] || {};
        const parsedFunctions = parseModuleFunctions(libName);
        updatedDescriptors[libName] = {
            ...existing,
            functions: parsedFunctions.length > 0 ? parsedFunctions : existing.functions || []
        };
    }
    return updatedDescriptors;
}

function buildBuiltinFunctions(baseFunctions) {
    // Merge in any user-library functions for discovered libraries.
    if (!fs.existsSync(LIBS_BASE)) return baseFunctions;

    let entries;
    try { entries = fs.readdirSync(LIBS_BASE, { withFileTypes: true }); }
    catch (_) { return baseFunctions; }

    const allParsed = [];
    for (const entry of entries) {
        if (!entry.isDirectory() || entry.name.startsWith('.')) continue;
        allParsed.push(...parseModuleFunctions(entry.name));
    }
    if (allParsed.length === 0) return baseFunctions;

    const libraryLabels = new Set(allParsed.map(f => f.label));
    const filteredBase = baseFunctions.filter(f => !libraryLabels.has(f.label));
    return [...filteredBase, ...allParsed];
}

module.exports = {
    buildModuleDescriptors,
    buildBuiltinFunctions,
    getAvailableLibPaths,
    parseIncludedFile,
    parseIncludeStatement,
    resolveIncludeAlias,
    parseSymbols,
    parseDocumentText
};
