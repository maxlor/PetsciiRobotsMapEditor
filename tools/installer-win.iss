#define MyAppName "PETSCII Robots Map Editor"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "Benjamin Lutz"
#define MyAppURL "https://github.com/maxlor/PetsciiRobotsMapEditor"
#define MyAppExeName "PetsciiRobotsMapEditor.exe"
#define MyAppAssocName MyAppName + " File"
#define MyAppAssocExt ".petmap"
#define MyAppAssocKey StringChange(MyAppAssocName, " ", "") + MyAppAssocExt
#define BuildDir "C:\Projects\build-PetsciiRobotsMapEditor-Desktop_Qt_5_15_2_MSVC2019_64bit-Release"
#define QtDir "C:\Qt\5.15.2\msvc2019_64"
#define VCRedistDir "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Redist\MSVC\14.28.29910\x64\Microsoft.VC142.CRT"

[Setup]
; NOTE: The value of AppId uniquely identifies this application. Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{B05F4D14-0A6C-47E2-BED9-8192753F870E}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName} {#MyAppVersion}
ChangesAssociations=yes
DisableProgramGroupPage=yes
; Remove the following line to run in administrative install mode (install for all users.)
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
OutputDir=C:\Projects\build-PetsciiRobotsMapEditor-Desktop_Qt_5_15_2_MSVC2019_64bit-Release
OutputBaseFilename=PetsciiRobotsMapEditor-{#MyAppVersion} Setup           
SetupIconFile=C:\Projects\PetsciiRobotsMapEditor\res\robot.ico
Compression=lzma2
SolidCompression=yes
WizardStyle=modern

ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
VersionInfoVersion={#MyAppVersion}
UninstallDisplayIcon={app}\{#MyAppExeName}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#BuildDir}\src\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildDir}\src\NimbusSansNarrow-Bold.otf"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildDir}\src\NimbusSansNarrow-Bold.otf.license"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildDir}\src\tileset.pet"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Svg.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\plugins\imageformats\qsvg.dll"; DestDir: "{app}\imageformats"; Flags: ignoreversion
Source: "{#QtDir}\plugins\platforms\qwindows.dll"; DestDir: "{app}\platforms"; Flags: ignoreversion
Source: "{#QtDir}\plugins\styles\qwindowsvistastyle.dll"; DestDir: "{app}\styles"; Flags: ignoreversion
Source: "{#VCRedistDir}\msvcp140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#VCRedistDir}\vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#VCRedistDir}\vcruntime140_1.dll"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Registry]
;Root: HKA; Subkey: "Software\Classes\{#MyAppAssocExt}\OpenWithProgids"; ValueType: string; ValueName: "{#MyAppAssocKey}"; ValueData: ""; Flags: uninsdeletevalue
;Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}"; ValueType: string; ValueName: ""; ValueData: "{#MyAppAssocName}"; Flags: uninsdeletekey
;Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"
;Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""
;Root: HKA; Subkey: "Software\Classes\Applications\{#MyAppExeName}\SupportedTypes"; ValueType: string; ValueName: ".myp"; ValueData: ""
Root: HKCU; Subkey : "Software\{#MyAppPublisher}"; Flags: dontcreatekey uninsdeletekeyifempty
Root: HKCU; Subkey : "Software\{#MyAppPublisher}\{#MyAppName}"; Flags: dontcreatekey uninsdeletekey

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

