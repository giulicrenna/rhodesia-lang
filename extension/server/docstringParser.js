/**
 * PEP8-style docstring parser for Rhodesia.
 * Supports `"""triple double-quoted"""` docstrings immediately
 * before or after function/record/enum declarations.
 */
function findPep8Doc(lines, i) {
	// Look ahead: check lines after the declaration for a docstring
	for (let j = i + 1; j < Math.min(i + 3, lines.length); j++) {
		const match = lines[j].match(/^\s*"""\s*(.*?)\s*"""\s*$/);
		if (match) return match[1];
		// Multi-line: """ opens without closing
		const open = lines[j].match(/^\s*"""/);
		if (open) {
			const parts = [lines[j].replace(/^\s*"""\s*/, '').replace(/\s*"""\s*$/, '')];
			for (let k = j + 1; k < lines.length; k++) {
				const close = lines[k].match(/"""/);
				if (close && k > j) {
					parts.push(lines[k].replace(/\s*"""\s*$/, '').trim());
					return parts.filter(Boolean).join(' ');
				}
				parts.push(lines[k].trim());
			}
		}
	}
	// Look behind: check lines before for a trailing docstring
	for (let j = i - 1; j >= Math.max(0, i - 3); j--) {
		const trimmed = lines[j].trim();
		if (!trimmed) continue;
		const match = trimmed.match(/^\s*"""\s*(.*?)\s*"""\s*$/);
		if (match) return match[1];
		break;
	}
	return null;
}

module.exports = { findPep8Doc };