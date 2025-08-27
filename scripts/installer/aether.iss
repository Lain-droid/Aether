; Inno Setup script for Aether (x64)
#define MyAppName "Aether"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "Aether Team"
#define MyAppExeName "Aether.exe"

[Setup]
AppId={{C8C0B1F6-2A7A-4E31-A5F7-7F3A9C6E9A77}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={pf64}\Aether
DefaultGroupName=Aether
LicenseFile=license.txt
OutputDir=..\..\artifacts\installer
OutputBaseFilename=Aether-Setup-x64
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
Compression=lzma
SolidCompression=yes
DisableDirPage=no
DisableProgramGroupPage=yes
SetupIconFile=icon.ico

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"; Flags: unchecked

[Files]
; Frontend publish output copied into {#SourceDir}
Source: "{#SourceDir}\\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\\{#MyAppExeName}"; Description: "Launch {#MyAppName}"; Flags: nowait postinstall skipifsilent
