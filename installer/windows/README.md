# Rhodesia Windows Installer

Builds a Windows installer for the [Rhodesia programming language](../../README.md)
using [Inno Setup 6](https://jrsoftware.org/isinfo.php).

The installer bundles the pre-compiled `rhodesia.exe` from the
[latest GitHub release](https://github.com/giulicrenna/rhodesia-lang/releases/latest).
No C++ toolchain is required on the build machine.

## What it produces

`installer\windows\Output\rhodesia-setup-0.1.0.exe` — a standard Windows installer
that, when run by an end user:

- Installs `rhodesia.exe`, the stdlib (`libs\math\`) and metadata to
  `C:\Program Files\Rhodesia\` (configurable).
- Adds `C:\Program Files\Rhodesia\bin` to the system PATH.
- Sets `RHODESIA_LIB_PATH=C:\Program Files\Rhodesia\libs` so the
  interpreter can locate the stdlib modules.
- Registers an uninstaller in **Settings → Apps**.

## Build requirements

Build machine:

| Tool | Notes |
|------|-------|
| Windows 10/11 | 64-bit |
| PowerShell 5+ | Built into Windows 10/11 |
| Inno Setup 6 | https://jrsoftware.org/isinfo.php — adds `ISCC.exe` to `Program Files` |
| Internet access | The script downloads `rhodesia_win64.exe` from the latest GitHub release |

## Build

From the repo root:

```powershell
cd installer\windows
.\build-installer.ps1
```

The script downloads `rhodesia_win64.exe` from the latest GitHub release,
stages the support files (`LICENSE`, `README.md`, `libs\math\*`) from the
local checkout, and invokes `ISCC.exe` on `rhodesia.iss`. The final
installer lands in `installer\windows\Output\rhodesia-setup-0.1.0.exe`
(size mirrors the released binary — currently ~6 MB compressed).

### Options

```powershell
.\build-installer.ps1 -Clean                # wipe staging/ first
.\build-installer.ps1 -InnoSetupPath <path>  # explicit ISCC.exe
```

## End-user install

Double-click `rhodesia-setup-0.1.0.exe`, approve the UAC prompt, accept
the MIT license, optionally tick **Create desktop icon**, and click
**Install**. The installer asks for admin rights because it writes to
`Program Files` and edits the system PATH.

### Verify in a new PowerShell window

```powershell
rhodesia -h
rhodesia -e "println(1 + 2)"
```

The second command should print `3` and exit.

## Uninstall

- **Settings → Apps → Installed apps → Rhodesia → Uninstall**, or
- Start Menu → **Rhodesia → Uninstall Rhodesia**.

The uninstaller removes `C:\Program Files\Rhodesia\`, strips the
`bin` entry from PATH, and removes the Start Menu / Desktop shortcuts.

> **Note:** `RHODESIA_LIB_PATH` is set during install but the Inno Setup
> `[Environment]` directive only auto-cleans PATH-style variables. Remove
> the lingering user/system env var manually if you want a fully clean
> slate:
> ```powershell
> [System.Environment]::SetEnvironmentVariable('RHODESIA_LIB_PATH', $null, 'User')
> ```

## Layout

```
installer/windows/
├── rhodesia.iss          # Inno Setup template (paths relative to this dir)
├── build-installer.ps1   # Orchestrator: download -> package
├── README.md             # This file
├── staging/              # Downloaded binary + staged support files (gitignored)
└── Output/               # Generated installer (gitignored)
```

## Troubleshooting

**Download fails — "Could not download rhodesia_win64.exe".**
The latest release on GitHub does not have a Windows binary published
under that name. Either publish one with the expected name, or update
the `$AssetName` variable at the top of `build-installer.ps1` to match
the asset you uploaded.

**Downloaded file is only a few KB.**
The script aborts with a clear error if the downloaded asset is
suspiciously small. The release either has no Windows binary or
uploaded a corrupt file — re-publish and retry.

**`ISCC.exe not found`.**
Install Inno Setup 6 (default path: `C:\Program Files (x86)\Inno Setup 6\ISCC.exe`).
If you installed it elsewhere, pass `-InnoSetupPath "D:\Tools\ISCC.exe"`.

**PATH update not visible in current terminal.**
PATH changes propagate to new processes only. Open a fresh PowerShell
window (or run `$env:PATH = [System.Environment]::GetEnvironmentVariable('Path','Machine') + ';' + $env:PATH`).

**`rhodesia` not found even after opening a new PowerShell.**
Windows caches the user's environment at logon. The installer writes to
`HKCU\Environment\Path` and broadcasts `WM_SETTINGCHANGE`, but explorer.exe
may not refresh its cached env in time (or at all on some Windows builds).
Two options:

1. **Sign out and back in** — the canonical fix. New logon re-reads PATH
   from the registry. This is what Python, Git for Windows, Node.js, etc.
   require after their installers add to PATH.
2. **Refresh the current session manually:**
   ```powershell
   $env:PATH = [System.Environment]::GetEnvironmentVariable('Path','User') + ';' + [System.Environment]::GetEnvironmentVariable('Path','Machine')
   ```

**Build outputs `rhodesia.exe` but ISCC says it's missing.**
Make sure you're running `build-installer.ps1` from the `installer\windows\`
directory (or via its absolute path). The `.iss` resolves `Source:`
paths relative to its own location.
