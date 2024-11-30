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

#define Bits 32

[Setup]
;; (not yet implemented) SignTool=mystandard
; Tell Windows Explorer to reload the environment
ChangesEnvironment=yes
; setting to DisableDirPage no makes it so users can change the installation directory
DisableDirPage=no   
AppName={#MyTitleName}
AppVersion={#Year}.{#Version}{#Other}
DefaultDirName={commonpf32}\{#MyTitleName}{#Other}
DefaultGroupName={#MyTitleName}{#Other}
SetupIconFile=..\..\include\{#MyTitleName}.ico
ChangesAssociations = yes
UninstallDisplayIcon={app}\{#MyTitleName}.exe
Compression=lzma2
SolidCompression=yes
OutputDir=output
OutputBaseFilename={#MyTitleName}{#Bits}_{#Year}_{#Version}{#Other}

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "Do you want to create desktop icon?"; Flags: checkablealone

[Files]
Source: "../../bin/xLights.exe"; DestDir: "{app}"
Source: "../../bin/xlights.windows.properties"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../bin/special.options"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../include/xlights.ico"; DestDir: "{app}"
Source: "../../include/xLights_nutcracker.ico"; DestDir: "{app}"

; xSchedule
Source: "../../bin/xSchedule.exe"; DestDir: "{app}"
Source: "../../bin/xschedule.windows.properties"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../bin/xScheduleWeb\*.*"; DestDir: "{app}/xScheduleWeb"; Flags: ignoreversion recursesubdirs
Source: "../../include/xSchedule.ico"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../documentation/xSchedule API Documentation.txt"; DestDir: "{app}"

; xCapture
Source: "../../bin/xCapture.exe"; DestDir: "{app}"
Source: "../../bin/xcapture.windows.properties"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../include/xcapture.ico"; DestDir: "{app}"; Flags: "ignoreversion"

; xScanner
Source: "../../bin/xScanner.exe"; DestDir: "{app}"
Source: "../../bin/xScanner.windows.properties"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../include/xScanner.ico"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../xScanner/MacLookup.txt"; DestDir: "{app}"; Flags: "ignoreversion"

; xFade
Source: "../../bin/xFade.exe"; DestDir: "{app}"
Source: "../../bin/xfade.windows.properties"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../include/xfade.ico"; DestDir: "{app}"; Flags: "ignoreversion"

Source: "../../bin/xlDo.exe"; DestDir: "{app}"
Source: "../../documentation/xlDo Commands.txt"; DestDir: "{app}"

; xSMSDaemon
Source: "../../bin/xSMSDaemon.dll"; DestDir: "{app}"
Source: "../../xSchedule/xSMSDaemon/Blacklist.txt"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../xSchedule/xSMSDaemon/Whitelist.txt"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../xSchedule/xSMSDaemon/PhoneBlacklist.txt"; DestDir: "{app}"; Flags: "ignoreversion"

; RemoteFalcon
Source: "../../bin/RemoteFalcon.dll"; DestDir: "{app}"

; wxWidgets
Source: "../../bin/wxmsw314u_gl_gcc_custom.dll"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../bin/wxbase314u_gcc_custom.dll"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../bin/wxbase314u_net_gcc_custom.dll"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../bin/wxbase314u_xml_gcc_custom.dll"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../bin/wxmsw314u_aui_gcc_custom.dll"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../bin/wxmsw314u_core_gcc_custom.dll"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../bin/wxmsw314u_html_gcc_custom.dll"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../bin/wxmsw314u_propgrid_gcc_custom.dll"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../bin/wxmsw314u_qa_gcc_custom.dll"; DestDir: "{app}"; Flags: "ignoreversion"

; Use the default installed mingw 32 bit files
Source: "../../bin/libgcc_s_dw2-1.dll";  DestDir: "{app}";  Flags: "ignoreversion"
Source: "../../bin/libstdc++-6.dll";     DestDir: "{app}";  Flags: "ignoreversion"
Source: "../../bin/libwinpthread-1.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "../../bin/liblog4cpp.dll"; DestDir: "{app}";  Flags: "ignoreversion"

; avlib - video and audio
Source: "../../bin/avcodec-58.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "../../bin/avformat-58.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "../../bin/avutil-56.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "../../bin/swresample-3.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "../../bin/swscale-5.dll"; DestDir: "{app}";  Flags: "ignoreversion"

; SDL - audio playing
Source: "../../bin/SDL2.dll"; DestDir: "{app}";  Flags: "ignoreversion"

; libcurl
Source: "../../bin/libcurl.dll"; DestDir: "{app}";  Flags: "ignoreversion"

; hidapi
Source: "../../bin/hidapi.dll"; DestDir: "{app}";  Flags: "ignoreversion"

; Added files for doing Papagayo effects
Source: "../../bin/extended_dictionary"; DestDir: "{app}"
Source: "../../bin/phoneme_mapping";     DestDir: "{app}"
Source: "../../bin/standard_dictionary"; DestDir: "{app}"
Source: "../../bin/german_dictionary";   DestDir: "{app}"
Source: "../../bin/user_dictionary";     DestDir: "{app}"

; readmes and licenses
Source: "../../License.txt"; DestDir: "{app}";
Source: "../../README.txt";  DestDir: "{app}"; Flags: isreadme

; Color Curves
Source: "../../colorcurves/*.*"; DestDir: "{app}/colorcurves"   ; Flags: ignoreversion recursesubdirs

; Value Curves
Source: "../../valuecurves/*.*"; DestDir: "{app}/valuecurves"   ; Flags: ignoreversion recursesubdirs

; Mesh Objects
Source: "../../meshobjects/*.*"; DestDir: "{app}/meshobjects"   ; Flags: ignoreversion recursesubdirs

; MHPreset Objects
Source: "../../meshobjects/*.*"; DestDir: "{app}/meshobjects"   ; Flags: ignoreversion recursesubdirs

; Controllers
Source: "../../controllers/*.*"; DestDir: "{app}/controllers"   ; Flags: ignoreversion recursesubdirs

; Palettes
Source: "../../palettes/*.*"; DestDir: "{app}/palettes"   ; Flags: ignoreversion recursesubdirs

; Scripts
;Source: "../../scripts/*.*"; DestDir: "{app}/scripts"   ; Flags: ignoreversion recursesubdirs

[Icons] 
Name: "{group}\xLights"; Filename: "{app}\xLights.EXE"; WorkingDir: "{app}"
Name: "{group}\xSchedule"; Filename: "{app}\xSchedule.EXE"; WorkingDir: "{app}"
Name: "{group}\xScanner"; Filename: "{app}\xScanner.EXE"; WorkingDir: "{app}"
Name: "{commondesktop}\xLights"; Filename: "{app}\xLights.EXE";   WorkingDir: "{app}"; Tasks: desktopicon ;   IconFilename: "{app}\xLights.ico";
Name: "{commondesktop}\xSchedule"; Filename: "{app}\xSchedule.EXE";   WorkingDir: "{app}"; Tasks: desktopicon ;   IconFilename: "{app}\xSchedule.ico";
Name: "{commondesktop}\xScanner"; Filename: "{app}\xScanner.EXE"; WorkingDir: "{app}"; Tasks: desktopicon ;   IconFilename: "{app}\xScanner.ico";

[Run]
Filename: "{app}\xLights.exe"; Description: "Launch application"; Flags: postinstall nowait skipifsilent 

[Registry]
Root: HKCU; Subkey: "Software\Xlights"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\xSchedule"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\xCapture"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\xScanner"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\xFade"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\xSMSDaemon"; Flags: uninsdeletekey

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

