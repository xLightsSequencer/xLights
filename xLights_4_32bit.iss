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

[Setup]
;; (not yet implemented) SignTool=mystandard
; Tell Windows Explorer to reload the environment
ChangesEnvironment=yes
; setting to DisableDirPage no makes it so users can change the installation directory
DisableDirPage=no   
AppName=xLights
AppVersion=2017.42
DefaultDirName={pf32}\xLights
DefaultGroupName=xLights
SetupIconFile=include\xlights.ico

UninstallDisplayIcon={app}\xLights.exe
Compression=lzma2
SolidCompression=yes
OutputDir=output
OutputBaseFilename=xLights32_2017_x

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "Do you want to create desktop icon?"; Flags: checkablealone

[Files]
Source: "bin/xLights.exe"; DestDir: "{app}"
Source: "bin/xlights.map"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "bin/xlights.windows.properties"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "include\xLights.ico"; DestDir: "{app}"
Source: "include\xLights_nutcracker.ico"; DestDir: "{app}"

; xSchedule
Source: "bin/xSchedule.exe"; DestDir: "{app}"
Source: "bin/xSchedule.map"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "bin/xschedule.windows.properties"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "bin/xScheduleWeb\*.*"; DestDir: "{app}/xScheduleWeb"; Flags: replacesameversion recursesubdirs
Source: "include\xSchedule.ico"; DestDir: "{app}"; Flags: "ignoreversion"


; xLightsBatchRender
Source: "bin/xLightsBatchRenderer.exe"; DestDir: "{app}"
Source: "include\xLightsBatchRenderer.ico"; DestDir: "{app}"

Source: "C:\wxWidgets-3.1.0\lib\gcc_dll\wxmsw310u_gcc_custom.dll";    DestDir: "{app}"; Flags: "ignoreversion"
Source: "C:\wxWidgets-3.1.0\lib\gcc_dll\wxmsw310u_gl_gcc_custom.dll"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "bin/libgcc_s_sjlj-1.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "bin\libgcc_s_dw2-1.dll";  DestDir: "{app}";  Flags: "ignoreversion"
Source: "bin\libstdc++-6.dll";     DestDir: "{app}";  Flags: "ignoreversion"
Source: "bin\libwinpthread-1.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "bin/liblog4cpp.dll"; DestDir: "{app}";  Flags: "ignoreversion"

; avlib - video and audio
Source: "bin/avcodec-57.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "bin/avformat-57.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "bin/avutil-55.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "bin/swresample-2.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "bin/swscale-4.dll"; DestDir: "{app}";  Flags: "ignoreversion"
; SDL - audio playing
Source: "bin/SDL2.dll"; DestDir: "{app}";  Flags: "ignoreversion"

; Added files for doing Papagayo effects
Source: "bin/extended_dictionary"; DestDir: "{app}"
Source: "bin/phoneme_mapping";     DestDir: "{app}"
Source: "bin/standard_dictionary"; DestDir: "{app}"
Source: "bin/user_dictionary";     DestDir: "{app}"

; Path editor
Source: "bin/PathEditor.exe";      DestDir: "{app}"

; readmes and licenses
Source: "License.txt"; DestDir: "{app}";
Source: "README.txt";  DestDir: "{app}"; Flags: isreadme

; Color Curves
Source: "colorcurves\*.*"; DestDir: "{app}/colorcurves"   ; Flags: replacesameversion recursesubdirs

; Value Curves
Source: "valuecurves\*.*"; DestDir: "{app}/valuecurves"   ; Flags: replacesameversion recursesubdirs

; controllers
; Source: "controllers\*.*"; DestDir: "{app}/controllers"   ; Flags: replacesameversion recursesubdirs

[Icons] 
Name: "{group}\xLights"; Filename: "{app}\xLights.EXE"; WorkingDir: "{app}"
Name: "{group}\xSchedule"; Filename: "{app}\xSchedule.EXE"; WorkingDir: "{app}"
Name: "{commondesktop}\xLights"; Filename: "{app}\xLights.EXE";   WorkingDir: "{app}"; Tasks: desktopicon ;   IconFilename: "{app}\xLights.ico";
Name: "{commondesktop}\xSchedule"; Filename: "{app}\xSchedule.EXE";   WorkingDir: "{app}"; Tasks: desktopicon ;   IconFilename: "{app}\xSchedule.ico";
Name: "{group}\xLightsBatchRender"; Filename: "{app}\xLightsBatchRenderer.EXE"; WorkingDir: "{app}"
Name: "{commondesktop}\xLightsBatchRender"; Filename: "{app}\xLightsBatchRenderer.EXE"; WorkingDir: "{app}"; Tasks: desktopicon ; IconFilename: "{app}\xLightsBatchRenderer.ico";

[Run]
Filename: "{app}\xLights.exe"; Description: "Launch application"; Flags: postinstall nowait skipifsilent 

[Registry]
Root: HKCU; Subkey: "Software\Xlights"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\xSchedule"; Flags: uninsdeletekey
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
