const fs = require('fs');
const path = require('path');

const LIBS_BASE = path.resolve(__dirname, '../../libs');

const MODULE_PATHS = {
    math: path.resolve(LIBS_BASE, 'math')
};

const MODULE_SUBDIRS = {
    math: [
        'core',
        'linear_algebra',
        'numerical',
        'statistics'
    ]
};

const FILES_BY_SUBDIR = {
    core: ['core.rho', 'trigonometry.rho'],
    linear_algebra: ['vectors.rho', 'matrices.rho', 'decompositions.rho'],
    numerical: ['root_finding.rho', 'optimization.rho', 'integration.rho'],
    statistics: ['descriptive.rho', 'probability.rho', 'regression.rho']
};

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
    return null;
}

/**
 * Parse a .rho file and return { functions, constants }.
 * - functions: array of descriptor objects (with _source, _line)
 * - constants: array of { label, type, value, documentation, _source, _line }
 */
function parseSymbols(filePath) {
    if (!fs.existsSync(filePath)) return { functions: [], constants: [] };

    const content = fs.readFileSync(filePath, 'utf8');
    const lines = content.split('\n');
    const functions = [];
    const constants = [];

    for (let i = 0; i < lines.length; i++) {
        const line = lines[i];

        // ---- Functions: fun name(...) -> type ----
        const funMatch = line.match(/\bfun\s+(\w+)\s*\(([^)]*)\)\s*->\s*([^\s{]+)/);
        if (funMatch) {
            const label = funMatch[1];
            const paramText = funMatch[2];
            const returnType = funMatch[3];
            const params = parseParams(paramText);
            const doc = findPrecedingDoc(lines, i) || { summary: '', paramDocs: {}, returns: '' };
            const signature = buildSignature(label, params, returnType);

            // Build parameter info using parsed param names from signature
            const paramInfos = params.map(param => {
                // param might be "type: name" or "name: type" — extract last word as name
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
                _source: filePath,
                _line: i,
                _return: doc.returns
            });
            continue;
        }

        // ---- Constants/Variables: type: NAME = value (ALL_CAPS = constant) ----
        const constMatch = line.match(/^(float64|int|bool|string|vec|mat)\s*:\s*([A-Za-z_]\w*)\s*=\s*(.+)/);
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
                _source: filePath,
                _line: i
            });
        }
    }

    return { functions, constants };
}

/**
 * Backwards-compatible wrapper: parse only functions from a file (like old parseFile).
 */
function parseFile(filePath) {
    return parseSymbols(filePath).functions;
}

/**
 * Walk the libs/ directory and return all available include paths.
 * e.g. "math/core/core", "math/linear_algebra/matrices"
 */
function getAvailableLibPaths() {
    const paths = [];
    if (!fs.existsSync(LIBS_BASE)) return paths;

    function walk(dir, prefix) {
        let entries;
        try { entries = fs.readdirSync(dir, { withFileTypes: true }); }
        catch (_) { return; }
        for (const entry of entries) {
            const rel = prefix ? `${prefix}/${entry.name}` : entry.name;
            if (entry.isDirectory()) {
                walk(path.join(dir, entry.name), rel);
            } else if (entry.name.endsWith('.rho')) {
                paths.push(rel.replace(/\.rho$/, ''));
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

function parseModuleFunctions(moduleName) {
    const basePath = MODULE_PATHS[moduleName];
    if (!basePath) return [];

    const functions = [];
    MODULE_SUBDIRS[moduleName].forEach(subdir => {
        const files = FILES_BY_SUBDIR[subdir] || [];
        files.forEach(file => {
            functions.push(...parseFile(path.join(basePath, subdir, file)));
        });
    });
    return functions;
}

function buildModuleDescriptors(baseDescriptors) {
    const updatedDescriptors = { ...baseDescriptors };
    Object.keys(MODULE_PATHS).forEach(moduleName => {
        const existing = updatedDescriptors[moduleName] || {};
        const parsedFunctions = parseModuleFunctions(moduleName);
        updatedDescriptors[moduleName] = {
            ...existing,
            functions: parsedFunctions.length > 0 ? parsedFunctions : existing.functions || []
        };
    });
    return updatedDescriptors;
}

function buildBuiltinFunctions(baseFunctions) {
    const mathFunctions = parseModuleFunctions('math');
    if (mathFunctions.length === 0) return baseFunctions;

    const libraryLabels = new Set(mathFunctions.map(f => f.label));
    const filteredBase = baseFunctions.filter(f => !libraryLabels.has(f.label));
    return [...filteredBase, ...mathFunctions];
}

module.exports = {
    buildModuleDescriptors,
    buildBuiltinFunctions,
    getAvailableLibPaths,
    parseIncludedFile,
    parseSymbols
};
