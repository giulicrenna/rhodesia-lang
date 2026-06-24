const vscode = require('vscode');
const { exec, spawn } = require('child_process');
const path = require('path');

const RHODESIA_BIN = 'C:\\Program Files\\Rhodesia\\bin\\rhodesia.exe';

/**
 * Minimal Rhodesia Debug Adapter.
 * Provides: continue, next, stepIn, stepOut, stackTrace, scopes, variables,
 *           setBreakpoints, configurationDone, evaluate, disconnect.
 *
 * Uses the Rhodesia VM's built-in debug hooks via `--debug` flag.
 */
class RhodesiaDebugAdapter {
	constructor() {
		this._debugSession = null;
		this._process = null;
		this._breakpoints = new Map(); // source.path -> line[]
		this._stopped = false;
		this._currentFrame = { id: 1, name: 'main', line: 1, column: 1 };
		this._stackFrames = [];
		this._variables = {};
		this._seq = 0;
		this._pendingRequests = new Map();
	}

	// DAP protocol handlers
	handleMessage(msg) {
		if (!msg || !msg.type) return;
		if (msg.type === 'request') {
			const seq = msg.seq;
			switch (msg.command) {
				case 'initialize': return this._sendResponse(seq, msg.command, {});
				case 'configurationDone': return this._sendResponse(seq, msg.command, {});
				case 'launch':
					this._launch(msg.arguments);
					return this._sendResponse(seq, msg.command, {});
				case 'setBreakpoints':
					return this._setBreakpoints(seq, msg);
				case 'stackTrace':
					return this._sendResponse(seq, msg.command, {
						stackFrames: this._stackFrames.length
							? this._stackFrames
							: [{
								id: 1,
								name: 'main',
								line: 1,
								column: 1,
								source: { path: msg.arguments.source?.path || '' }
							}],
						totalFrames: 1
					});
				case 'scopes':
					return this._sendResponse(seq, msg.command, {
						scopes: [{
							name: 'Locals',
							variablesReference: 1,
							expensive: false
						}]
					});
				case 'variables':
					const ref = msg.arguments.variablesReference;
					const vars = ref === 1 ? Object.entries(this._variables).map(([k, v]) => ({
						name: k, value: String(v), variablesReference: 0
					})) : [];
					return this._sendResponse(seq, msg.command, { variables: vars });
				case 'continue':
					this._stopped = false;
					return this._sendResponse(seq, msg.command, { allThreadsContinued: true });
				case 'next':
					return this._sendResponse(seq, msg.command, {});
				case 'stepIn':
					return this._sendResponse(seq, msg.command, {});
				case 'stepOut':
					return this._sendResponse(seq, msg.command, {});
				case 'pause':
					return this._sendResponse(seq, msg.command, {});
				case 'evaluate':
					const expr = msg.arguments.expression;
					const val = this._variables[expr] !== undefined ? String(this._variables[expr]) : `<eval not supported>`;
					return this._sendResponse(seq, msg.command, {
						result: val,
						variablesReference: 0
					});
				case 'disconnect':
					if (this._process) { this._process.kill(); this._process = null; }
					return this._sendResponse(seq, msg.command, {});
				default:
					return this._sendResponse(seq, msg.command, {});
			}
		}
	}

	_sendResponse(requestSeq, command, body) {
		if (!this._send) return;
		this._send({
			type: 'response',
			request_seq: requestSeq,
			success: true,
			command,
			body
		});
	}

	_setBreakpoints(seq, msg) {
		const args = msg.arguments;
		const sourcePath = args.source?.path;
		if (!sourcePath) return this._sendResponse(seq, 'setBreakpoints', { breakpoints: [] });
		this._breakpoints.set(sourcePath, args.breakpoints || []);
		const bps = (args.breakpoints || []).map(bp => ({
			id: this._seq++,
			verified: true,
			line: bp.line
		}));
		this._sendResponse(seq, 'setBreakpoints', { breakpoints: bps });
	}

	_launch(args) {
		const filePath = args.program;
		if (!filePath) return;
		const breakpoints = [];
		for (const [, bps] of this._breakpoints) {
			for (const bp of bps) breakpoints.push(`-b${bp.line}`);
		}
		const bpArgs = breakpoints.flatMap(b => [b]);

		this._process = spawn(RHODESIA_BIN, ['--debug', ...bpArgs, filePath]);
		let buffer = '';

		this._process.stdout.on('data', (data) => {
			buffer += data.toString();
			this._parseDebugOutput(buffer);
		});
		this._process.stderr.on('data', (data) => {
			buffer += data.toString();
			this._parseDebugOutput(buffer);
		});
		this._process.on('close', () => {
			this._stopped = false;
		});
	}

	_parseDebugOutput(buffer) {
		// Parse VM debug lines: ">> DBG stop at <line>:<col>" or ">> DBG var <name>=<value>"
		const lines = buffer.split('\n');
		for (const line of lines) {
			const stopMatch = line.match(/>> DBG stop at (\d+):(\d+)/);
			if (stopMatch) {
				this._currentFrame = {
					id: 1,
					name: 'main',
					line: parseInt(stopMatch[1], 10),
					column: parseInt(stopMatch[2], 10) || 1
				};
				this._stackFrames = [this._currentFrame];
				this._stopped = true;
				if (this._send) {
					this._send({
						type: 'event',
						event: 'stopped',
						body: { reason: 'breakpoint', threadId: 1 }
					});
				}
				return;
			}
			const varMatch = line.match(/>> DBG var (\w+)=(.+)/);
			if (varMatch) {
				this._variables[varMatch[1]] = varMatch[2].trim();
			}
		}
		// Keep unparsed
	}

	setSend(sendFn) {
		this._send = sendFn;
	}
}

/**
 * Factory: creates a DebugConfigurationProvider and returns the debug adapter.
 */
function createRhodesiaDebugFactory() {
	const adapter = new RhodesiaDebugAdapter();

	return {
		provideDebugConfigurations(folder, token) {
			return [{
				type: 'rhodesia',
				name: 'Rhodesia Debug',
				request: 'launch',
				program: '${file}'
			}];
		},
		resolveDebugConfiguration(folder, config, token) {
			if (!config.program) {
				return vscode.window.showInformationMessage('No program to debug').then(() => undefined);
			}
			return config;
		},
		createDebugAdapterDescriptor(session) {
			// Inline adapter
			return new vscode.DebugAdapterInlineImplementation(adapter);
		},
		adapter
	};
}

module.exports = { createRhodesiaDebugFactory, RhodesiaDebugAdapter };