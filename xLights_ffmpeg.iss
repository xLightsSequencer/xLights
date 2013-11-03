; -- xLights_ffmpeg.iss --
; Demonstrates copying 3 files and creating an icon.

;  SEE THE DOCUMENTATION FOR DETAILS ON CREATING .ISS SCRIPT FILES!

#define MyTitleName "xLights" 

[Setup]
; Tell Windows Explorer to reload the environment
ChangesEnvironment=yes
AppName=xLights-Nutcracker
AppVersion=3.2.4
DefaultDirName={pf}\xLights
DefaultGroupName=xLights
UninstallDisplayIcon={app}\xLights.exe
Compression=lzma2
SolidCompression=yes
OutputDir=output


[Files]
Source: "bin/xLights.exe"; DestDir: "{app}"
Source: "bin/ffmpeg.exe"; DestDir: "{app}"
Source: "bin/PathEditor.exe"; DestDir: "{app}"
;Source: "c:\wxWidgets\lib\gcc_dll\wxmsw294u_gcc_custom.dll"; DestDir: "{app}"
Source: "C:\wxWidgets-2.9.5\lib\gcc_dll\wxmsw295u_gcc_custom.dll"; DestDir: "{app}"
Source: "License.txt"; DestDir: "{app}";
Source: "Readme.txt";  DestDir: "{app}"; Flags: isreadme



[Icons]
Name: "{group}\xLights"; Filename: "{app}\xLights.exe"

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