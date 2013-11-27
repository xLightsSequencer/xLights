; -- xLights.iss --
; Demonstrates copying 3 files and creating an icon.

;  SEE THE DOCUMENTATION FOR DETAILS ON CREATING .ISS SCRIPT FILES!

[Setup]
AppName=xLights-Nutcracker
AppVersion=3.2.9
DefaultDirName={pf}\xLights
DefaultGroupName=xLights
UninstallDisplayIcon={app}\xLights.exe
Compression=lzma2
SolidCompression=yes
OutputDir=output

[Files]
Source: "bin/xLights.exe"; DestDir: "{app}"
Source: "bin/Piano-88KeyShapeMap.txt"; DestDir: "{app}"
Source: "bin/Piano-ExampleFaceMinShapeMap.txt"; DestDir: "{app}"
Source: "bin/Piano-ExampleFaceMinShapes.PNG"; DestDir: "{app}"
Source: "bin/Piano-ExampleKeyEdgeShapes.PNG"; DestDir: "{app}"
Source: "bin/Piano-ExampleKeyTopShapes.PNG"; DestDir: "{app}"
Source: "songs/Piano-TestFace_notes.txt"; DestDir: "{app}"
Source: "songs/Piano-TestMidiNotes.txt"; DestDir: "{app}"
Source: "c:\wxWidgets\lib\gcc_dll\wxmsw294u_gcc_custom.dll"; DestDir: "{app}"
Source: "License.txt"; DestDir: "{app}";
Source: "Readme.txt";  DestDir: "{app}"; Flags: isreadme

[Icons]
Name: "{group}\xLights"; Filename: "{app}\xLights.exe"
