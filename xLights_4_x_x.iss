; -- xLights_opengl.iss --
; File used for building xLights.exe

;  SEE THE DOCUMENTATION FOR DETAILS ON CREATING .ISS SCRIPT FILES!
; mar 3,2015: added new line for bin/xlights.map
; mar 3,2016:  added Source: "bin/avcodec-57.dll"; DestDir: "{pf32}";  Flags: "ignoreversion"
; mar 3,2016:  added Source: "bin/avformat-57.dll"; DestDir: "{pf32}";  Flags: "ignoreversion"
; mar 3,2016:  added Source: "bin/avutil-55.dll"; DestDir: "{pf32}";  Flags: "ignoreversion"
; mar 3,2016:  added Source: "bin/swresample-2.dll"; DestDir: "{pf32}";  Flags: "ignoreversion"
; mar 3,2016:  added Source: "bin/swscale-4.dll"; DestDir: "{pf32}";  Flags: "ignoreversion"
; mar 3,2016:  added Source: "bin/SDL2.dll"; DestDir: "{pf32}";  Flags: "ignoreversion"
; mar 3,2016: (Removed, No longer needed as of 2016.10) Source: "bin/ffmpeg.exe"; DestDir: "{pf32}"; Flags: "ignoreversion"
; mar 7, 2016 added DisableDirPage=no . This always prompt for an installation directory


#define MyTitleName "xLights" 

[Setup]
;; (not yet implemented) SignTool=mystandard
; Tell Windows Explorer to reload the environment
ChangesEnvironment=yes
; setting to DisableDirPage no makes it so users can change the installation directory
DisableDirPage=no   
AppName=xLights
AppVersion=2017.10
DefaultDirName={pf32}\xLights
DefaultGroupName=xLights
SetupIconFile=include\xlights.ico

UninstallDisplayIcon={pf32}\xLights.exe
Compression=lzma2
SolidCompression=yes
OutputDir=output
OutputBaseFilename=xLights_Nutcracker_2017_x


[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "Do you want to create desktop icon?"; Flags: checkablealone


[Files]
Source: "bin/xLights.exe"; DestDir: "{pf32}"
Source: "bin/xSchedule.exe"; DestDir: "{pf32}"

Source: "C:\wxWidgets-3.1.0\lib\gcc_dll\wxmsw310u_gcc_custom.dll";    DestDir: "{pf32}"; Flags: "ignoreversion"
Source: "C:\wxWidgets-3.1.0\lib\gcc_dll\wxmsw310u_gl_gcc_custom.dll"; DestDir: "{pf32}"; Flags: "ignoreversion"
;Source: "bin\glut32.dll";          DestDir: "{pf32}";  Flags: "ignoreversion"
Source: "bin\libgcc_s_dw2-1.dll";  DestDir: "{pf32}";  Flags: "ignoreversion"
Source: "bin\libstdc++-6.dll";     DestDir: "{pf32}";  Flags: "ignoreversion"
Source: "bin\libwinpthread-1.dll"; DestDir: "{pf32}";  Flags: "ignoreversion"
Source: "bin/libgcc_s_sjlj-1.dll"; DestDir: "{pf32}";  Flags: "ignoreversion"

Source: "bin/liblog4cpp.dll"; DestDir: "{pf32}";  Flags: "ignoreversion"
Source: "bin/xlights.windows.properties"; DestDir: "{pf32}";


;Source: "lib/windows/avcodec.lib"; DestDir: "{pf32}";  Flags: "ignoreversion"
;Source: "lib/windows/avformat.lib"; DestDir: "{pf32}";  Flags: "ignoreversion"
;Source: "lib/windows/avutil.lib"; DestDir: "{pf32}";  Flags: "ignoreversion"

Source: "bin/avcodec-57.dll"; DestDir: "{pf32}";  Flags: "ignoreversion"
Source: "bin/avformat-57.dll"; DestDir: "{pf32}";  Flags: "ignoreversion"
Source: "bin/avutil-55.dll"; DestDir: "{pf32}";  Flags: "ignoreversion"
Source: "bin/swresample-2.dll"; DestDir: "{pf32}";  Flags: "ignoreversion"
Source: "bin/swscale-4.dll"; DestDir: "{pf32}";  Flags: "ignoreversion"
Source: "bin/SDL2.dll"; DestDir: "{pf32}";  Flags: "ignoreversion"

; No longer needed as of 2016.10. Source: "bin/ffmpeg.exe";          DestDir: "{pf32}"; Flags: "ignoreversion"
Source: "bin/xlights.map";         DestDir: "{pf32}"; Flags: "ignoreversion"
; Added files for doing Papagayo effects
Source: "bin/extended_dictionary"; DestDir: "{pf32}"
Source: "bin/phoneme_mapping";     DestDir: "{pf32}"
Source: "bin/standard_dictionary"; DestDir: "{pf32}"
Source: "bin/user_dictionary";     DestDir: "{pf32}"
;

; xschedule
Source: "bin/xschedule.windows.properties";     DestDir: "{pf32}"
Source: "bin/xSchedule.map";     DestDir: "{pf32}"
Source: "bin/xScheduleWeb\*.*";     DestDir: "{pf32}/xScheduleWeb"; Flags: replacesameversion recursesubdirs

Source: "bin/gawk.exe";            DestDir: "{pf32}"
Source: "bin/coroface.awk";        DestDir: "{pf32}"
Source: "bin/corofaces.bat";       DestDir: "{pf32}"
Source: "bin/picturefaces.awk";    DestDir: "{pf32}"
Source: "bin/modelxls.awk";        DestDir: "{pf32}"
Source: "bin/modelxls.bat";        DestDir: "{pf32}"
Source: "bin/merge_xml.awk";       DestDir: "{pf32}"
Source: "bin/PathEditor.exe";      DestDir: "{pf32}"
Source: "include\xLights.ico";     DestDir: "{pf32}"
Source: "include\xSchedule.ico";     DestDir: "{pf32}"
;mar4 Source: "include\xLights_orig.ico"; DestDir: "{pf32}"
Source: "include\xLights_nutcracker.ico"; DestDir: "{pf32}"
Source: "C:\Users\Sean_Dell\Documents\xLights\songs/*.*"; DestDir: "{pf32}/songs"    ; Flags: replacesameversion recursesubdirs
;Source: "piano/*.*"; DestDir: "{pf32}/piano" 
Source: "License.txt"; DestDir: "{pf32}";
Source: "README.txt";  DestDir: "{pf32}"; Flags: isreadme
;Source: "README.corofaces.txt";  DestDir: "{pf32}"; Flags: isreadme
Source: "colorcurves\*.*"; DestDir: "{pf32}/colorcurves"   ; Flags: replacesameversion recursesubdirs

[Icons] 
Name: "{commondesktop}\xLights4"; Filename: "{pf32}\xLights.EXE";   WorkingDir: "{pf32}"; Tasks: desktopicon ;   IconFilename: "{pf32}\xLights.ico";
Name: "{commondesktop}\xSchedule"; Filename: "{pf32}\xSchedule.EXE";   WorkingDir: "{pf32}"; Tasks: desktopicon ;   IconFilename: "{pf32}\xSchedule.ico";



[Run]
Filename: "{pf32}\xLights.exe"; Description: "Launch application"; Flags: postinstall nowait skipifsilent 


[Registry]
; set PATH. if it is already there dont add path to our installation. we are doing this so user can run ffmpeg from a cmd prompt
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; ValueType: expandsz; ValueName: "Path"; ValueData: "{olddata};{pf32}\xLights"; Check: NeedsAddPath ('C:\Program Files (x86)\xLights')


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
