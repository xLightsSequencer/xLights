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
ArchitecturesAllowed=x64
; "ArchitecturesInstallIn64BitMode=x64" requests that the install be
; done in "64-bit mode" on x64, meaning it should use the native
; 64-bit Program Files directory and the 64-bit view of the registry.
ArchitecturesInstallIn64BitMode=x64

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
Source: "../../bin64/xLights.exe"; DestDir: "{app}"
Source: "../../bin/xlights.windows.properties"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../bin/special.options"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../include\xlights64.ico"; DestDir: "{app}"
Source: "../../include\xLights_nutcracker.ico"; DestDir: "{app}"

; xSchedule
Source: "../../bin64/xSchedule.exe"; DestDir: "{app}"
Source: "../../bin/xschedule.windows.properties"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../bin/xScheduleWeb\*.*"; DestDir: "{app}/xScheduleWeb"; Flags: ignoreversion recursesubdirs
Source: "../../include\xSchedule64.ico"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../documentation/xSchedule API Documentation.txt"; DestDir: "{app}"

; xCapture
Source: "../../bin64/xCapture.exe"; DestDir: "{app}"
Source: "../../bin/xcapture.windows.properties"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../include\xcapture64.ico"; DestDir: "{app}"; Flags: "ignoreversion"

; xScanner
Source: "../../bin64/xScanner.exe"; DestDir: "{app}"
Source: "../../bin/xScanner.windows.properties"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../include/xScanner64.ico"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../xScanner/MacLookup.txt"; DestDir: "{app}"; Flags: "ignoreversion"

; xFade
Source: "../../bin64/xFade.exe"; DestDir: "{app}"
Source: "../../bin/xfade.windows.properties"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../include/xfade64.ico"; DestDir: "{app}"; Flags: "ignoreversion"

; xlDo
Source: "../../bin64/xlDo.exe"; DestDir: "{app}"
Source: "../../documentation/xlDo Commands.txt"; DestDir: "{app}"

; xSMSDaemon
Source: "../../bin64/xSMSDaemon.dll"; DestDir: "{app}"
Source: "../../xSchedule\xSMSDaemon\Blacklist.txt"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../xSchedule\xSMSDaemon\Whitelist.txt"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../xSchedule\xSMSDaemon\PhoneBlacklist.txt"; DestDir: "{app}"; Flags: "ignoreversion"

; RemoteFalcon
Source: "../../bin64/RemoteFalcon.dll"; DestDir: "{app}"

; wxWidgets
Source: "../../bin64/wxmsw314u_gl_gcc_custom.dll"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../bin64/wxbase314u_gcc_custom.dll"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../bin64/wxbase314u_net_gcc_custom.dll"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../bin64/wxbase314u_xml_gcc_custom.dll"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../bin64/wxmsw314u_aui_gcc_custom.dll"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../bin64/wxmsw314u_core_gcc_custom.dll"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../bin64/wxmsw314u_html_gcc_custom.dll"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../bin64/wxmsw314u_propgrid_gcc_custom.dll"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../bin64/wxmsw314u_qa_gcc_custom.dll"; DestDir: "{app}"; Flags: "ignoreversion"

; Take these from the default mingw install directory
Source: "../../bin64/libgcc_s_seh-1.dll";  DestDir: "{app}";  Flags: "ignoreversion"
Source: "../../bin64/libstdc++-6.dll";     DestDir: "{app}";  Flags: "ignoreversion"
Source: "../../bin64/libwinpthread-1.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "../../bin64/liblog4cpp.dll"; DestDir: "{app}";  Flags: "ignoreversion"

; avlib - video and audio
Source: "../../bin64/avcodec-59.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "../../bin64/avformat-59.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "../../bin64/avutil-57.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "../../bin64/swresample-4.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "../../bin64/swscale-6.dll"; DestDir: "{app}";  Flags: "ignoreversion"

; SDL - audio playing
Source: "../../bin64/SDL2.dll"; DestDir: "{app}";  Flags: "ignoreversion"

; libcurl
Source: "../../bin64/libcurl-x64.dll"; DestDir: "{app}";  Flags: "ignoreversion"

; hidapi
Source: "../../bin64/hidapi.dll"; DestDir: "{app}";  Flags: "ignoreversion"

; Added files for doing Papagayo effects
Source: "../../bin/extended_dictionary"; DestDir: "{app}"
Source: "../../bin/phoneme_mapping";     DestDir: "{app}"
Source: "../../bin/standard_dictionary"; DestDir: "{app}"
Source: "../../bin/german_dictionary";   DestDir: "{app}"
Source: "../../bin/user_dictionary";     DestDir: "{app}"

; Vamp dll's
Source: "../../bin64/Vamp/*.dll"; DestDir: "{app}"; Flags: "ignoreversion"

; readmes and licenses
Source: "../../License.txt"; DestDir: "{app}";
Source: "../../README.txt";  DestDir: "{app}"; Flags: isreadme

; Color Curves
Source: "../../colorcurves/*.*"; DestDir: "{app}/colorcurves"   ; Flags: ignoreversion recursesubdirs

; Value Curves
Source: "../../valuecurves/*.*"; DestDir: "{app}/valuecurves"   ; Flags: ignoreversion recursesubdirs

; Mesh Objects
Source: "../../meshobjects/*.*"; DestDir: "{app}/meshobjects"   ; Flags: ignoreversion recursesubdirs

; Controllers
Source: "../../controllers/*.*"; DestDir: "{app}/controllers"   ; Flags: ignoreversion recursesubdirs

; Palettes
Source: "../../palettes/*.*"; DestDir: "{app}/palettes"   ; Flags: ignoreversion recursesubdirs

; Scripts
;Source: "../../scripts/*.*"; DestDir: "{app}/scripts"   ; Flags: ignoreversion recursesubdirs

[Icons]
Name: "{group}\xLights64"; Filename: "{app}\xLights.EXE"; WorkingDir: "{app}"
Name: "{group}\xSchedule64"; Filename: "{app}\xSchedule.EXE"; WorkingDir: "{app}"
Name: "{group}\xScanner64"; Filename: "{app}\xScanner.EXE"; WorkingDir: "{app}"
Name: "{commondesktop}\xLights64"; Filename: "{app}\xLights.EXE";   WorkingDir: "{app}"; Tasks: desktopicon ;   IconFilename: "{app}\xLights64.ico";
Name: "{commondesktop}\xSchedule64"; Filename: "{app}\xSchedule.EXE"; WorkingDir: "{app}"; Tasks: desktopicon ;   IconFilename: "{app}\xSchedule64.ico";
Name: "{commondesktop}\xScanner64"; Filename: "{app}\xScanner.EXE"; WorkingDir: "{app}"; Tasks: desktopicon ;   IconFilename: "{app}\xScanner64.ico";

[Run]
Filename: "{app}\xLights.exe"; Description: "Launch application"; Flags: postinstall nowait skipifsilent 

[Registry]
Root: HKCU; Subkey: "Software\Xlights"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\xSchedule"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\xCapture"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\xScanner"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\xFade"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\xSMSDaemon"; Flags: uninsdeletekey
Root: HKCR; Subkey: ".xsq";                              ValueData: "{#MyTitleName}";          Flags: uninsdeletevalue; ValueType: string;  ValueName: ""
Root: HKCR; Subkey: "{#MyTitleName}";                    ValueData: "Program {#MyTitleName}";  Flags: uninsdeletekey;   ValueType: string;  ValueName: ""
Root: HKCR; Subkey: "{#MyTitleName}\DefaultIcon";        ValueData: "{app}\{#MyTitleName}.exe,0";                          ValueType: string;  ValueName: ""
Root: HKCR; Subkey: "{#MyTitleName}\shell\open\command"; ValueData: """{app}\{#MyTitleName}.exe"" ""%1""";                 ValueType: string;  ValueName: ""
