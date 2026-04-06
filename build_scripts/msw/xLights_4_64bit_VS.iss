; -- xLights_opengl.iss --
; File used for building xLights.exe

;  SEE THE DOCUMENTATION FOR DETAILS ON CREATING .ISS SCRIPT FILES!
; mar 3,2016:  added Source: "bin/avcodec-57.dll"; DestDir: "{app}";  Flags: "ignoreversion"
; mar 3,2016:  added Source: "bin/avformat-57.dll"; DestDir: "{app}";  Flags: "ignoreversion"
; mar 3,2016:  added Source: "bin/avutil-55.dll"; DestDir: "{app}";  Flags: "ignoreversion"
; mar 3,2016:  added Source: "bin/swresample-2.dll"; DestDir: "{app}";  Flags: "ignoreversion"
; mar 3,2016:  added Source: "bin/swscale-4.dll"; DestDir: "{app}";  Flags: "ignoreversion"
; mar 3,2016:  added Source: "bin/SDL2.dll"; DestDir: "{app}";  Flags: "ignoreversion"
; mar 3,2016: (Removed, No longer needed as of 2016.10) Source: "bin/ffmpeg.exe"; DestDir: "{app}"; Flags: "ignoreversion"
; mar 7, 2016 added DisableDirPage=no . This always prompt for an installation directory

#include "xLights_common.iss"

#define Bits 64

[Setup]
;; (not yet implemented) SignTool=mystandard
; Tell Windows Explorer to reload the environment
ChangesEnvironment=yes
; setting to DisableDirPage no makes it so users can change the installation directory
DisableDirPage=no   
; "ArchitecturesAllowed=x64" specifies that Setup cannot run on
; anything but x64.
ArchitecturesAllowed=x64compatible
; "ArchitecturesInstallIn64BitMode=x64" requests that the install be
; done in "64-bit mode" on x64, meaning it should use the native
; 64-bit Program Files directory and the 64-bit view of the registry.
ArchitecturesInstallIn64BitMode=x64compatible

AppName={#MyTitleName}
AppVersion={#Year}.{#Version}{#Other}
DefaultDirName={commonpf64}\{#MyTitleName}{#Other}
DefaultGroupName={#MyTitleName}{#Other}
SetupIconFile=..\..\include\{#MyTitleName}64.ico
ChangesAssociations = yes
UninstallDisplayIcon={app}\{#MyTitleName}.exe
Compression=lzma2
SolidCompression=yes
OutputDir=output
OutputBaseFilename={#MyTitleName}{#Bits}_{#Year}_{#Version}{#Other}

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "Do you want to create desktop icon?"; Flags: checkablealone

[Files]
Source: "../../xlights/x64/Release/xLights.exe"; DestDir: "{app}"
Source: "../../bin64/xLights.map"; DestDir: "{app}"
Source: "../../bin/special.options"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../include\xlights64.ico"; DestDir: "{app}"
Source: "../../include\xLights_nutcracker.ico"; DestDir: "{app}"

; xlDo
Source: "../../xlDo/x64/Release/xlDo.exe"; DestDir: "{app}"
Source: "../../bin64/xlDo.map"; DestDir: "{app}"
Source: "../../documentation/xlDo Commands.txt"; DestDir: "{app}"

; fseq_convert
Source: "../../fseq_convert/cmake_vs/Release/fseq_convert.exe"; DestDir: "{app}"

; TipOfDay
; Commented out until we have enough content and assuming we want to distribute that content
; Source: "../../TipOfDay/*.*"; DestDir: "{app}/TipOfDay"; Flags: ignoreversion 

; wxWidgets
; Statically linkes

; Take these from the default mingw install directory
Source: "../../bin64/libgcc_s_seh-1.dll";  DestDir: "{app}";  Flags: "ignoreversion"

; avlib - video and audio
Source: "../../bin64/avcodec-60.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "../../bin64/avdevice-60.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "../../bin64/avfilter-9.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "../../bin64/avformat-60.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "../../bin64/avutil-58.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "../../bin64/postproc-57.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "../../bin64/swresample-4.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "../../bin64/swscale-7.dll"; DestDir: "{app}";  Flags: "ignoreversion"

; SDL - audio playing
Source: "../../bin64/SDL2.dll"; DestDir: "{app}";  Flags: "ignoreversion"

; libcurl
Source: "../../bin64/libcurl-x64.dll"; DestDir: "{app}";  Flags: "ignoreversion"

; hidapi
Source: "../../bin64/hidapi.dll"; DestDir: "{app}";  Flags: "ignoreversion"

; Dictionaries (for Papagayo effects)
Source: "../../resources/dictionaries/*"; DestDir: "{app}/dictionaries"; Flags: ignoreversion
Source: "../../bin/german_dictionary";    DestDir: "{app}"

; Vamp dll's
Source: "../../bin64/Vamp/*.dll"; DestDir: "{app}"; Flags: "ignoreversion"

; readmes and licenses
Source: "../../License.txt"; DestDir: "{app}";
Source: "../../README.txt";  DestDir: "{app}"; Flags: isreadme

; Color Curves
Source: "../../resources/colorcurves/*.*"; DestDir: "{app}/colorcurves"   ; Flags: ignoreversion recursesubdirs

; Value Curves
Source: "../../resources/valuecurves/*.*"; DestDir: "{app}/valuecurves"   ; Flags: ignoreversion recursesubdirs

; Mesh Objects
Source: "../../resources/meshobjects/*.*"; DestDir: "{app}/meshobjects"   ; Flags: ignoreversion recursesubdirs

; MHPreset Objects
Source: "../../resources/mhpresets/*.*"; DestDir: "{app}/mhpresets"   ; Flags: ignoreversion recursesubdirs

; Controllers
Source: "../../resources/controllers/*.*"; DestDir: "{app}/controllers"   ; Flags: ignoreversion recursesubdirs

; Prompts
Source: "../../resources/prompts/*.*"; DestDir: "{app}/prompts"   ; Flags: ignoreversion recursesubdirs

; Palettes
Source: "../../resources/palettes/*.*"; DestDir: "{app}/palettes"   ; Flags: ignoreversion recursesubdirs

; HTML resources (tailwind etc.)
Source: "../../resources/html/*.*"; DestDir: "{app}/html"   ; Flags: ignoreversion recursesubdirs

; Scripts
;Source: "../../scripts/*.*"; DestDir: "{app}/scripts"   ; Flags: ignoreversion recursesubdirs

; VC++ Redistributable
Source: "vcredist/vc_redist.x64.exe"; DestDir: {tmp}; Flags: deleteafterinstall

[Icons]
Name: "{group}\xLights64"; Filename: "{app}\xLights.EXE"; WorkingDir: "{app}"
Name: "{commondesktop}\xLights64"; Filename: "{app}\xLights.EXE";   WorkingDir: "{app}"; Tasks: desktopicon ;   IconFilename: "{app}\xLights64.ico";

[Run]
Filename: {tmp}\vc_redist.x64.exe; \
    Parameters: "/q /passive /norestart /Q:a /c:""msiexec /q /i vcredist.msi"""; \
    StatusMsg: "Installing VC++ Redistributables..."
	
Filename: "{app}\xLights.exe"; Description: "Launch application"; Flags: postinstall nowait skipifsilent 

[Registry]
Root: HKCU; Subkey: "Software\Xlights"; Flags: uninsdeletekey

Root: HKCR; Subkey: ".xsq";                                ValueData: "xLights.Sequence";           Flags: uninsdeletevalue; ValueType: string;  ValueName: ""
Root: HKCR; Subkey: "xLights.Sequence";                    ValueData: "xLights Sequence";           Flags: uninsdeletekey;   ValueType: string;  ValueName: ""
Root: HKCR; Subkey: "xLights.Sequence\DefaultIcon";        ValueData: "{app}\{#MyTitleName}.exe,0"; Flags: uninsdeletekey;   ValueType: string;  ValueName: ""
Root: HKCR; Subkey: "xLights.Sequence\shell\open\command"; ValueData: """{app}\{#MyTitleName}.exe"" ""%1"""; Flags: uninsdeletekey; ValueType: string; ValueName: ""

Root: HKCR; Subkey: ".xsqz";                                       ValueData: "xLights.PackagedSequence";  Flags: uninsdeletevalue; ValueType: string;  ValueName: ""
Root: HKCR; Subkey: "xLights.PackagedSequence";                    ValueData: "xLights Packaged Sequence";  Flags: uninsdeletekey;   ValueType: string;  ValueName: ""
Root: HKCR; Subkey: "xLights.PackagedSequence\DefaultIcon";        ValueData: "{app}\{#MyTitleName}.exe,11"; Flags: uninsdeletekey;   ValueType: string;  ValueName: ""
Root: HKCR; Subkey: "xLights.PackagedSequence\shell\open\command"; ValueData: """{app}\{#MyTitleName}.exe"" ""%1"""; Flags: uninsdeletekey; ValueType: string; ValueName: ""

; delete old association
Root: HKCR; Subkey: "{#MyTitleName}";                    Flags: deletekey
Root: HKCR; Subkey: "{#MyTitleName}\DefaultIcon";        Flags: deletekey
Root: HKCR; Subkey: "{#MyTitleName}\shell\open\command"; Flags: deletekey

