; Rhodesia Language - Inno Setup installer script
; Run with: ISCC.exe rhodesia.iss   (from the installer/ directory)
; Produces: installer/Output/rhodesia-setup-0.1.0.exe

#define MyAppName "Rhodesia"
#define MyAppVersion "0.1.2"
#define MyAppPublisher "Giuliano Crenna"
#define MyAppPublisherURL "https://github.com/giulicrenna/rhodesia-lang"
#define MyAppExeName "rhodesia.exe"

[Setup]
AppId={{8E5A2B1F-7D3C-4A1B-9F2E-6C8B5D4E3A2F}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppPublisherURL}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
LicenseFile=staging\src\LICENSE
OutputDir=Output
OutputBaseFilename=rhodesia-setup-{#MyAppVersion}
Compression=lzma2
SolidCompression=yes
PrivilegesRequired=admin
PrivilegesRequiredOverridesAllowed=dialog
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
UninstallDisplayIcon={app}\bin\{#MyAppExeName}
Uninstallable=yes
VersionInfoVersion={#MyAppVersion}.0
VersionInfoCompany={#MyAppPublisher}
VersionInfoDescription={#MyAppName} Programming Language Installer

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "staging\src\build\{#MyAppExeName}"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "staging\src\libs\math\*"; DestDir: "{app}\libs\math"; Flags: recursesubdirs
Source: "staging\src\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "staging\src\README.md"; DestDir: "{app}"; Flags: ignoreversion

[Dirs]
Name: "{app}\libs"; Permissions: users-modify
Name: "{app}\bin"; Permissions: users-modify

[Icons]
Name: "{group}\Rhodesia REPL"; Filename: "{app}\bin\{#MyAppExeName}"
Name: "{group}\Rhodesia Documentation"; Filename: "{app}\README.md"
Name: "{commondesktop}\Rhodesia REPL"; Filename: "{app}\bin\{#MyAppExeName}"; Tasks: desktopicon
Name: "{group}\Uninstall Rhodesia"; Filename: "{uninstallexe}"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Registry]
Root: HKCU; Subkey: "Environment"; ValueType: expandsz; ValueName: "Path"; \
    ValueData: "{olddata};{app}\bin"; Check: NeedsAddPath('{app}\bin')
Root: HKCU; Subkey: "Environment"; ValueType: expandsz; ValueName: "RHODESIA_LIB_PATH"; \
    ValueData: "{app}\libs"; Flags: uninsdeletevalue

[Run]
Filename: "{app}\README.md"; Description: "View Rhodesia README"; Flags: nowait postinstall skipifsilent shellexec

[Code]
function NeedsAddPath(Param: string): Boolean;
var
  OrigPath: string;
  ParamExpanded: string;
begin
  ParamExpanded := ExpandConstant(Param);
  if not RegQueryStringValue(HKEY_CURRENT_USER, 'Environment', 'Path', OrigPath) then
  begin
    Result := True;
    exit;
  end;
  Result := Pos(';' + ParamExpanded + ';', ';' + OrigPath + ';') = 0;
end;

procedure BroadcastEnvChange();
begin
  PostMessage(HWND_BROADCAST, $001A, 0, 0);
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
    BroadcastEnvChange;
end;