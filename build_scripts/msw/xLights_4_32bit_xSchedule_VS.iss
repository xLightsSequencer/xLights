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
UninstallDisplayIcon={app}\{#MyTitleName}.exe
Compression=lzma2
SolidCompression=yes
OutputDir=output
OutputBaseFilename={#MyTitleName}{#Bits}_{#Year}_{#Version}{#Other}

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "Do you want to create desktop icon?"; Flags: checkablealone

[Files]
Source: "../../bin/special.options"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../include\xlights.ico"; DestDir: "{app}"
Source: "../../include\xLights_nutcracker.ico"; DestDir: "{app}"

; xSchedule
Source: "../../xSchedule/Release/xSchedule.exe"; DestDir: "{app}"
Source: "../../bin/xschedule.windows.properties"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../bin/xScheduleWeb\*.*"; DestDir: "{app}/xScheduleWeb"; Flags: ignoreversion recursesubdirs
Source: "../../include\xSchedule.ico"; DestDir: "{app}"; Flags: "ignoreversion"

; xSMSDaemon
Source: "../../xSchedule/xSMSDaemon/Release/xSMSDaemon.dll"; DestDir: "{app}"
Source: "../../xSchedule\xSMSDaemon\Blacklist.txt"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../xSchedule\xSMSDaemon\Whitelist.txt"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "../../xSchedule\xSMSDaemon\PhoneBlacklist.txt"; DestDir: "{app}"; Flags: "ignoreversion"

; RemoteFalcon
Source: "../../xSchedule/RemoteFalcon/Release/RemoteFalcon.dll"; DestDir: "{app}"

; wxWidgets
; Statically linked

; avlib - video and audio
Source: "../../bin/avcodec-58.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "../../bin/avformat-58.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "../../bin/avutil-56.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "../../bin/swresample-3.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "../../bin/swscale-5.dll"; DestDir: "{app}";  Flags: "ignoreversion"

; Controllers
Source: "../../controllers/*.*"; DestDir: "{app}/controllers"   ; Flags: ignoreversion recursesubdirs

; SDL - audio playing
Source: "../../bin/SDL2.dll"; DestDir: "{app}";  Flags: "ignoreversion"

; libcurl
Source: "../../bin/libcurl.dll"; DestDir: "{app}";  Flags: "ignoreversion"

; readmes and licenses
Source: "../../License.txt"; DestDir: "{app}";
Source: "../../README.txt";  DestDir: "{app}"; Flags: isreadme

; VC++ Redistributable
Source: "vcredist/vc_redist.x86.exe"; DestDir: {tmp}; Flags: deleteafterinstall

[Icons] 
Name: "{group}\xSchedule"; Filename: "{app}\xSchedule.EXE"; WorkingDir: "{app}"
Name: "{commondesktop}\xSchedule"; Filename: "{app}\xSchedule.EXE";   WorkingDir: "{app}"; Tasks: desktopicon ;   IconFilename: "{app}\xSchedule.ico";

[Run]
Filename: {tmp}\vc_redist.x86.exe; \
    Parameters: "/q /passive /Q:a /c:""msiexec /q /i vcredist.msi"""; \
    StatusMsg: "Installing VC++ Redistributables..."

Filename: "{app}\xSchedule.exe"; Description: "Launch application"; Flags: postinstall nowait skipifsilent 

[Registry]
Root: HKCU; Subkey: "Software\Xlights"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\xSchedule"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\xSMSDaemon"; Flags: uninsdeletekey
