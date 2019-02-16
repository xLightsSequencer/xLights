; -- xLights_opengl.iss --
; File used for building xLights.exe

;  SEE THE DOCUMENTATION FOR DETAILS ON CREATING .ISS SCRIPT FILES!
; mar 3,2015: added new line for bin/xlights.map
; mar 3,2016:  added Source: "bin/avcodec-57.dll"; DestDir: "{app}";  Flags: "ignoreversion"
; mar 3,2016:  added Source: "bin/avformat-57.dll"; DestDir: "{app}";  Flags: "ignoreversion"
; mar 3,2016:  added Source: "bin/avutil-55.dll"; DestDir: "{app}";  Flags: "ignoreversion"
; mar 3,2016:  added Source: "bin/swresample-2.dll"; DestDir: "{app}";  Flags: "ignoreversion"
; mar 3,2016:  added Source: "bin/swscale-4.dll"; DestDir: "{app}";  Flags: "ignoreversion"
; mar 3,2016:  added Source: "bin/SDL2.dll"; DestDir: "{app}";  Flags: "ignoreversion"
; mar 3,2016: (Removed, No longer needed as of 2016.10) Source: "bin/ffmpeg.exe"; DestDir: "{app}"; Flags: "ignoreversion"
; mar 7, 2016 added DisableDirPage=no . This always prompt for an installation directory


#define MyTitleName "xLights" 
#define Year 2019
#define Version 12
#define Bits 32
#define Other ""

[Setup]
;; (not yet implemented) SignTool=mystandard
; Tell Windows Explorer to reload the environment
ChangesEnvironment=yes
; setting to DisableDirPage no makes it so users can change the installation directory
DisableDirPage=no   
AppName={#MyTitleName}
AppVersion={#Year}.{#Version}{#Other}
DefaultDirName={pf32}\{#MyTitleName}{#Other}
DefaultGroupName={#MyTitleName}{#Other}
SetupIconFile=include\{#MyTitleName}.ico

UninstallDisplayIcon={app}\{#MyTitleName}.exe
Compression=lzma2
SolidCompression=yes
OutputDir=output
OutputBaseFilename={#MyTitleName}{#Bits}_{#Year}_{#Version}{#Other}

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "Do you want to create desktop icon?"; Flags: checkablealone

[Files]
Source: "bin/xLights.exe"; DestDir: "{app}"
Source: "bin/xlights.map"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "bin/xlights.windows.properties"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "include\xlights.ico"; DestDir: "{app}"
Source: "include\xLights_nutcracker.ico"; DestDir: "{app}"

; xSchedule
Source: "bin/xSchedule.exe"; DestDir: "{app}"
Source: "bin/xSchedule.map"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "bin/xschedule.windows.properties"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "bin/xScheduleWeb\*.*"; DestDir: "{app}/xScheduleWeb"; Flags: replacesameversion recursesubdirs
Source: "include\xSchedule.ico"; DestDir: "{app}"; Flags: "ignoreversion"

; xCapture
Source: "bin/xCapture.exe"; DestDir: "{app}"
Source: "bin/xCapture.map"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "bin/xcapture.windows.properties"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "include\xcapture.ico"; DestDir: "{app}"; Flags: "ignoreversion"

; xFade
Source: "bin/xFade.exe"; DestDir: "{app}"
Source: "bin/xFade.map"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "bin/xfade.windows.properties"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "include\xfade.ico"; DestDir: "{app}"; Flags: "ignoreversion"

; xSMSDaemon
Source: "bin/xSMSDaemon.exe"; DestDir: "{app}"
Source: "bin/xSMSDaemon.map"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "bin/xsmsdaemon.windows.properties"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "include\xsmsdaemon.ico"; DestDir: "{app}"; Flags: "ignoreversion"

Source: "bin/wxmsw311u_gcc_custom.dll";    DestDir: "{app}"; Flags: "ignoreversion"
Source: "bin/wxmsw311u_gl_gcc_custom.dll"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "bin/libgcc_s_sjlj-1.dll"; DestDir: "{app}";  Flags: "ignoreversion"
; Use the default installed mingw 32 bit files
Source: "C:\Program Files (x86)\mingw-w64\i686-7.2.0-posix-dwarf-rt_v5-rev1\mingw32\bin\libgcc_s_dw2-1.dll";  DestDir: "{app}";  Flags: "ignoreversion"
Source: "C:\Program Files (x86)\mingw-w64\i686-7.2.0-posix-dwarf-rt_v5-rev1\mingw32\bin\libstdc++-6.dll";     DestDir: "{app}";  Flags: "ignoreversion"
Source: "C:\Program Files (x86)\mingw-w64\i686-7.2.0-posix-dwarf-rt_v5-rev1\mingw32\bin\libwinpthread-1.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "bin/liblog4cpp.dll"; DestDir: "{app}";  Flags: "ignoreversion"

; avlib - video and audio
Source: "bin/avcodec-57.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "bin/avformat-57.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "bin/avutil-55.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "bin/swresample-2.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "bin/swscale-4.dll"; DestDir: "{app}";  Flags: "ignoreversion"

; SDL - audio playing
Source: "bin/SDL2.dll"; DestDir: "{app}";  Flags: "ignoreversion"

; libcurl
Source: "bin/libcurl.dll"; DestDir: "{app}";  Flags: "ignoreversion"

; Added files for doing Papagayo effects
Source: "bin/extended_dictionary"; DestDir: "{app}"
Source: "bin/phoneme_mapping";     DestDir: "{app}"
Source: "bin/standard_dictionary"; DestDir: "{app}"
Source: "bin/user_dictionary";     DestDir: "{app}"

; readmes and licenses
Source: "License.txt"; DestDir: "{app}";
Source: "README.txt";  DestDir: "{app}"; Flags: isreadme

; Color Curves
Source: "colorcurves\*.*"; DestDir: "{app}/colorcurves"   ; Flags: replacesameversion recursesubdirs

; Value Curves
Source: "valuecurves\*.*"; DestDir: "{app}/valuecurves"   ; Flags: replacesameversion recursesubdirs

; Palettes
Source: "palettes\*.*"; DestDir: "{app}/palettes"   ; Flags: replacesameversion recursesubdirs

; controllers
; Source: "controllers\*.*"; DestDir: "{app}/controllers"   ; Flags: replacesameversion recursesubdirs

[Icons] 
Name: "{group}\xLights"; Filename: "{app}\xLights.EXE"; WorkingDir: "{app}"
Name: "{group}\xSchedule"; Filename: "{app}\xSchedule.EXE"; WorkingDir: "{app}"
Name: "{commondesktop}\xLights"; Filename: "{app}\xLights.EXE";   WorkingDir: "{app}"; Tasks: desktopicon ;   IconFilename: "{app}\xLights.ico";
Name: "{commondesktop}\xSchedule"; Filename: "{app}\xSchedule.EXE";   WorkingDir: "{app}"; Tasks: desktopicon ;   IconFilename: "{app}\xSchedule.ico";

[Run]
Filename: "{app}\xLights.exe"; Description: "Launch application"; Flags: postinstall nowait skipifsilent 

[Registry]
Root: HKCU; Subkey: "Software\Xlights"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\xSchedule"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\xCapture"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\xFade"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\xSMSDaemon"; Flags: uninsdeletekey
; set PATH. if it is already there dont add path to our installation. we are doing this so user can run ffmpeg from a cmd prompt
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; ValueType: expandsz; ValueName: "Path"; ValueData: "{olddata};{app}\xLights"; Check: NeedsAddPath ('C:\Program Files (x86)\xLights')

[Code]

function NeedsAddPath(Param: string): boolean;
var
  OrigPath: string;
begin
  if not RegQueryStringValue(HKEY_LOCAL_MACHINE,'SYSTEM\CurrentControlSet\Control\Session Manager\Environment', 'Path', OrigPath)
  then begin
    Result := True;
    exit;
  end;
  // look for the path with leading and trailing semicolon
  // Pos() returns 0 if not found
  Result := Pos(';' + UpperCase(Param) + ';', ';' + UpperCase(OrigPath) + ';') = 0;  
  if Result = True then
     Result := Pos(';' + UpperCase(Param) + '\;', ';' + UpperCase(OrigPath) + ';') = 0; 
end;
