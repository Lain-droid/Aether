; Inno Setup script for Aether (x64)
#define MyAppName "Aether"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "Aether Team"
#define MyAppExeName "Aether.exe"

#ifndef SourceDir
#define SourceDir "..\\..\\artifacts\\frontend"
#endif

[Setup]
AppName=Aether
AppVersion=1.0.0
AppPublisher=Aether Team
AppPublisherURL=https://github.com/Lain-droid/Aether
AppSupportURL=https://github.com/Lain-droid/Aether/issues
AppUpdatesURL=https://github.com/Lain-droid/Aether/releases
DefaultDirName={autopf}\Aether
DefaultGroupName=Aether
OutputDir=artifacts\installer
OutputBaseFilename=Aether-Setup
SetupIconFile=scripts\installer\icon.ico
LicenseFile=scripts\installer\license.txt
Compression=lzma
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=lowest
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "artifacts\frontend\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Aether"; Filename: "{app}\Aether.Frontend.exe"
Name: "{group}\{cm:UninstallProgram,Aether}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\Aether"; Filename: "{app}\Aether.Frontend.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\Aether.Frontend.exe"; Description: "{cm:LaunchProgram,Aether}"; Flags: nowait postinstall skipifsilent

[Code]
function InitializeSetup(): Boolean;
begin
  Result := True;
end;
