; -- Audacity Vamp Plugins.iss --
; Demonstrates copying 3 files and creating an icon.

;  SEE THE DOCUMENTATION FOR DETAILS ON CREATING .ISS SCRIPT FILES!

#define MyTitleName "Audacity Vamp Plugins" 

[Setup]
; Tell Windows Explorer to reload the environment
ChangesEnvironment=yes
AppName=Audacity Vamp Plugins
AppVersion=1.7
DefaultDirName={pf}\Vamp Plugins
DefaultGroupName=Vamp Plugins
UninstallDisplayIcon={app}\Audacity Vamp Plugins.exe
Compression=lzma2
SolidCompression=yes
OutputDir=output
OutputBaseFilename=Vamp_Plugin

[Files]

Source: "*.*"; DestDir: "{app}"    ; Flags: recursesubdirs
Source: "README.txt";  DestDir: "{app}"; Flags: isreadme



[Icons]
Name: "{group}\Audacity Vamp Plugins"; Filename: "{app}\Vamp_Plugins.exe"

