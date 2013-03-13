; -- xLights.iss --
; Demonstrates copying 3 files and creating an icon.

; SEE THE DOCUMENTATION FOR DETAILS ON CREATING .ISS SCRIPT FILES!

[Setup]
AppName=xLights-Nutcracker
AppVersion=3.0.6
DefaultDirName={pf}\xLights
DefaultGroupName=xLights
UninstallDisplayIcon={app}\xLights.exe
Compression=lzma2
SolidCompression=yes
OutputDir=output

[Files]
Source: "bin/xLights.exe"; DestDir: "{app}"
Source: "bin/wxmsw294u_gcc_custom.dll"; DestDir: "{app}"
Source: "License.txt"; DestDir: "{app}";
Source: "Readme.txt";  DestDir: "{app}"; Flags: isreadme

[Icons]
Name: "{group}\xLights"; Filename: "{app}\xLights.exe"
