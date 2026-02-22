; Quanta Programming Language Installer Script (Inno Setup)
; Generates a professional Windows installer for Quanta

#define MyAppName "Quanta Language"
#define MyAppVersion "1.0"
#define MyAppPublisher "Your Name / Organization"
#define MyAppExeName "quanta.exe"

[Setup]
; Basic Information
AppId={{YOUR-UNIQUE-GUID-HERE}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
OutputDir=.\Output
OutputBaseFilename=Quanta_Installer_v{#MyAppVersion}
Compression=lzma2/ultra
SolidCompression=yes

; User Interface Optimizations
; Enable modern Wizard styling
WizardStyle=modern
; If you have custom images, uncomment and point to them
; WizardImageFile=installer_left_banner.bmp
; WizardSmallImageFile=installer_top_right.bmp
; SetupIconFile=quanta_icon.ico

; Only require admin privileges if installing to Program Files
PrivilegesRequired=admin

; Specify that this installer can be run on 64-bit systems
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
; IMPORTANT: Replace "..\build\quanta.exe" with the actual path to your compiled Windows executable
Source: "..\build\quanta.exe"; DestDir: "{app}"; Flags: ignoreversion
; You can include the standard library or docs if you have them:
; Source: "..\lib\*"; DestDir: "{app}\lib"; Flags: ignoreversion recursesubdirs createallsubdirs
; Source: "..\docs\*"; DestDir: "{app}\docs"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName} Documentation"; Filename: "{app}\docs\The_Quanta_Programming_Language.md"
Name: "{group}\Uninstall {#MyAppName}"; Filename: "{uninstallexe}"

[Registry]
; Add the installation directory to the system PATH so users can type `quanta` globally in CMD/PowerShell
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; \
    ValueType: expandsz; ValueName: "Path"; ValueData: "{olddata};{app}"; \
    Check: NeedsAddPath(ExpandConstant('{app}'))

[Code]
// Helper function to check if the app directory is already in the system PATH
function NeedsAddPath(Param: string): boolean;
var
  OrigPath: string;
begin
  if not RegQueryStringValue(HKEY_LOCAL_MACHINE, 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment', 'Path', OrigPath) then
  begin
    Result := True;
    exit;
  end;
  // Look for the path with leading and trailing semicolon
  // Pos() returns > 0 if found
  Result := Pos(';' + Param + ';', ';' + OrigPath + ';') = 0;
end;
