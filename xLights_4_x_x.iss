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
AppVersion=2016.18
DefaultDirName={pf}\xLights
DefaultGroupName=xLights
SetupIconFile=include\xlights.ico

UninstallDisplayIcon={app}\xLights.exe
Compression=lzma2
SolidCompression=yes
OutputDir=output
OutputBaseFilename=xLights_Nutcracker_2016_x


[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "Do you want to create desktop icon?"; Flags: checkablealone


[Files]
Source: "bin/xLights.exe"; DestDir: "{app}"

Source: "C:\wxWidgets-3.1.0\lib\gcc_dll\wxmsw310u_gcc_custom.dll";    DestDir: "{app}"; Flags: "ignoreversion"
Source: "C:\wxWidgets-3.1.0\lib\gcc_dll\wxmsw310u_gl_gcc_custom.dll"; DestDir: "{app}"; Flags: "ignoreversion"
Source: "bin\glut32.dll";          DestDir: "{app}";  Flags: "ignoreversion"
Source: "bin\libgcc_s_dw2-1.dll";  DestDir: "{app}";  Flags: "ignoreversion"
Source: "bin\libstdc++-6.dll";     DestDir: "{app}";  Flags: "ignoreversion"
Source: "bin\libwinpthread-1.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "bin/libgcc_s_sjlj-1.dll"; DestDir: "{app}";  Flags: "ignoreversion"

Source: "bin/liblog4cpp.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "bin/xlights.windows.properties"; DestDir: "{app}";


;Source: "lib/windows/avcodec.lib"; DestDir: "{app}";  Flags: "ignoreversion"
;Source: "lib/windows/avformat.lib"; DestDir: "{app}";  Flags: "ignoreversion"
;Source: "lib/windows/avutil.lib"; DestDir: "{app}";  Flags: "ignoreversion"

Source: "bin/avcodec-57.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "bin/avformat-57.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "bin/avutil-55.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "bin/swresample-2.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "bin/swscale-4.dll"; DestDir: "{app}";  Flags: "ignoreversion"
Source: "bin/SDL2.dll"; DestDir: "{app}";  Flags: "ignoreversion"

; No longer needed as of 2016.10. Source: "bin/ffmpeg.exe";          DestDir: "{app}"; Flags: "ignoreversion"
Source: "bin/xlights.map";         DestDir: "{app}"; Flags: "ignoreversion"
; Added files for doing Papagayo effects
Source: "bin/extended_dictionary"; DestDir: "{app}"
Source: "bin/phoneme_mapping";     DestDir: "{app}"
Source: "bin/standard_dictionary"; DestDir: "{app}"
Source: "bin/user_dictionary";     DestDir: "{app}"
;

Source: "bin/gawk.exe";            DestDir: "{app}"
Source: "bin/coroface.awk";        DestDir: "{app}"
Source: "bin/corofaces.bat";       DestDir: "{app}"
Source: "bin/picturefaces.awk";    DestDir: "{app}"
Source: "bin/modelxls.awk";        DestDir: "{app}"
Source: "bin/modelxls.bat";        DestDir: "{app}"
Source: "bin/merge_xml.awk";       DestDir: "{app}"
Source: "bin/PathEditor.exe";      DestDir: "{app}"
Source: "include\xLights.ico";     DestDir: "{app}"
;mar4 Source: "include\xLights_orig.ico"; DestDir: "{app}"
Source: "include\xLights_nutcracker.ico"; DestDir: "{app}"
Source: "C:\Users\Sean_Dell\Documents\xLights\songs/*.*"; DestDir: "{app}/songs"    ; Flags: replacesameversion recursesubdirs
Source: "piano/*.*"; DestDir: "{app}/piano" 
Source: "License.txt"; DestDir: "{app}";
Source: "README.txt";  DestDir: "{app}"; Flags: isreadme
;Source: "README.corofaces.txt";  DestDir: "{app}"; Flags: isreadme

[Icons] 
Name: "{commondesktop}\xLights4"; Filename: "{app}\xLights.EXE";   WorkingDir: "{app}"; Tasks: desktopicon ;   IconFilename: "{app}\xLights.ico";



[Run]
Filename: "{app}\xLights.exe"; Description: "Launch application"; Flags: postinstall nowait skipifsilent 


[Registry]
; set PATH. if it is already there dont add path to our installation. we are doing this so user can run ffmpeg from a cmd prompt
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; ValueType: expandsz; ValueName: "Path"; ValueData: "{olddata};{pf}\xLights"; Check: NeedsAddPath ('C:\Program Files (x86)\xLights')


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
