const fs = require('fs');
const path = require('path');

// Auto-extract keywords + module/function names from the C++ source so the
// extension stays in sync when new keywords/functions are added to the compiler.
// Returns null on any failure — callers should fall back to hand-maintained data.

const REPO_ROOT = path.resolve(__dirname, '../../');
const TOKEN_HPP = path.join(REPO_ROOT, 'include/Token.hpp');
const BUILTINS_HPP = path.join(REPO_ROOT, 'include/Builtins.hpp');
const NETWORK_HPP = path.join(REPO_ROOT, 'include/NetworkModule.hpp');

function safeRead(p) {
    try { return fs.readFileSync(p, 'utf8'); }
    catch (_) { return null; }
}

/**
 * Parse all keywords from the `getKeywords()` literal table in Token.hpp.
 * Returns an array of strings (sorted, deduped).
 */
function parseKeywords() {
    const text = safeRead(TOKEN_HPP);
    if (!text) return null;
    // Match {"name", TokenType::Kw...}, inside the keywords table. Anchoring
    // to `Kw` skips reverse-maps and other non-keyword token tables.
    const re = /\{\s*"(\w+)"\s*,\s*TokenType::Kw\w+/g;
    const set = new Set();
    let m;
    while ((m = re.exec(text))) set.add(m[1]);
    if (set.size === 0) return null;
    return Array.from(set).sort();
}

/**
 * Parse module + function names from Builtins.hpp using the
 * `auto& aliasModule = modules_["<modname>"];` declaration pattern,
 * then `<alias>Module["<funcname>"] = ` for each registration.
 * Also picks up `registerNetworkModule(modules_, moduleConstants_)` for `net`.
 *
 * Returns { moduleName: [funcName, ...], ... } — empty object on failure.
 */
function parseBuiltins() {
    const text = safeRead(BUILTINS_HPP);
    if (!text) return {};
    // Match both `auto& mathModule = modules_["math"];` AND `auto& net = modules_["net"];`
    const moduleDeclRe = /auto&\s+(\w+)\s*=\s*modules_\["(\w+)"\]/g;
    const aliases = {}; // alias -> moduleName
    let m;
    while ((m = moduleDeclRe.exec(text))) aliases[m[1]] = m[2];

    const out = {};
    for (const [alias, modName] of Object.entries(aliases)) {
        out[modName] = new Set();
        // Match both `mathModule["norm"] = ...` AND `net["socket"] = ...`
        const fnRe = new RegExp(`\\b${alias}\\["(\\w+)"\\]\\s*=`, 'g');
        let fm;
        while ((fm = fnRe.exec(text))) out[modName].add(fm[1]);
    }

    // Net module is registered in NetworkModule.hpp via registerNetworkModule().
    const netText = safeRead(NETWORK_HPP);
    if (netText) {
        const seen = out.net || new Set();
        // `auto& net = modules_["net"];` + `net["socket"] = ...` pattern
        const aliases2 = {};
        const r = /auto&\s+(\w+)\s*=\s*modules_\["(\w+)"\]/g;
        let x;
        while ((x = r.exec(netText))) aliases2[x[1]] = x[2];
        for (const [alias, modName] of Object.entries(aliases2)) {
            const fnRe = new RegExp(`\\b${alias}\\["(\\w+)"\\]\\s*=`, 'g');
            let fm;
            while ((fm = fnRe.exec(netText))) seen.add(fm[1]);
        }
        out.net = seen;
    }

    // Convert sets to sorted arrays
    for (const k of Object.keys(out)) out[k] = Array.from(out[k]).sort();
    return out;
}

/**
 * Walk Builtins.hpp + NetworkModule.hpp and return a per-module, per-function
 * line-number map. Used by the definition provider so Ctrl+click lands on
 * the actual registration site instead of line 0.
 *
 * Returns { moduleName: Map<funcName, lineNumber>, ... } — empty object on failure.
 */
function parseBuiltinLines() {
    const out = {};
    const addFrom = (text, fileLabel) => {
        if (!text) return;
        const moduleDeclRe = /auto&\s+(\w+)\s*=\s*modules_\["(\w+)"\]/g;
        const aliases = {};
        let m;
        while ((m = moduleDeclRe.exec(text))) aliases[m[1]] = m[2];
        for (const [alias, modName] of Object.entries(aliases)) {
            const map = out[modName] = out[modName] || new Map();
            const fnRe = new RegExp(`\\b${alias}\\["(\\w+)"\\]\\s*=`, 'g');
            let fm;
            while ((fm = fnRe.exec(text))) {
                // First occurrence wins — preserves the registration site even
                // if the same key is re-registered later in the file.
                if (!map.has(fm[1])) {
                    map.set(fm[1], lineOf(text, fm.index));
                }
            }
        }
    };
    addFrom(safeRead(BUILTINS_HPP), 'Builtins.hpp');
    addFrom(safeRead(NETWORK_HPP), 'NetworkModule.hpp');
    return out;
}

/** Convert a character index to a 0-based line number. */
function lineOf(text, idx) {
    let line = 0;
    for (let i = 0; i < idx && i < text.length; i++) {
        if (text[i] === '\n') line++;
    }
    return line;
}

module.exports = { parseKeywords, parseBuiltins, parseBuiltinLines };