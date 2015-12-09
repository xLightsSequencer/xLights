; -- xLights_opengl.iss --
; File used for building xLights.exe

;  SEE THE DOCUMENTATION FOR DETAILS ON CREATING .ISS SCRIPT FILES!
; mar 3,2015: added new line for bin/xlights.map

#define MyTitleName "xLights" 

[Setup]
; Tell Windows Explorer to reload the environment
ChangesEnvironment=yes
AppName=xLights-Nutcracker
AppVersion=4.2.25
DefaultDirName={pf}\xLights
DefaultGroupName=xLights
SetupIconFile=include\xLights.ico

UninstallDisplayIcon={app}\xLights.exe
Compression=lzma2
SolidCompression=yes
OutputDir=output
OutputBaseFilename=xLights_Nutcracker_4_x_x


[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "Do you want to create desktop icon?"; Flags: checkablealone


[Files]
Source: "bin/xLights.exe"; DestDir: "{app}"
Source: "C:\wxWidgets-3.0.2\lib\gcc_dll\wxmsw30u_gcc_custom.dll"; DestDir: "{app}"
Source: "C:\wxWidgets-3.0.2\lib\gcc_dll\wxmsw30u_gl_gcc_custom.dll"; DestDir: "{app}"
;Source: "C:\Users\Sean_Dell\wxwidgets-3.0.2\lib\gcc_dll\wxmsw30ud_gl_gcc_custom.dll"; DestDir: "{app}"
Source: "C:\Users\Sean_Dell\Documents\xLights\bin\glut32.dll"; DestDir: "{app}"
Source: "C:\Users\Sean_Dell\Documents\xLights\bin\libmpg123-0.dll"; DestDir: "{app}"
Source: "bin/ffmpeg.exe";       DestDir: "{app}"
Source: "bin/xlights.map";      DestDir: "{app}"
; Added files for doing Papagayo effects
Source: "bin/extended_dictionary";      DestDir: "{app}"
Source: "bin/phoneme_mapping";      DestDir: "{app}"
Source: "bin/standard_dictionary";      DestDir: "{app}"
Source: "bin/user_dictionary";      DestDir: "{app}"
;
Source: "bin/libgcc_s_sjlj-1.dll"; DestDir: "{app}"
Source: "bin/gawk.exe";         DestDir: "{app}"
Source: "bin/coroface.awk";     DestDir: "{app}"
Source: "bin/corofaces.bat";    DestDir: "{app}"
Source: "bin/picturefaces.awk"; DestDir: "{app}"
Source: "bin/modelxls.awk";     DestDir: "{app}"
Source: "bin/modelxls.bat";     DestDir: "{app}"
Source: "bin/merge_xml.awk";    DestDir: "{app}"
Source: "bin/PathEditor.exe";   DestDir: "{app}"
Source: "include\xLights.ico";  DestDir: "{app}"
Source: "include\xLights_orig.ico"; DestDir: "{app}"
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