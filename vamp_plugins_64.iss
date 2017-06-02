; -- Audacity Vamp Plugins_64.iss --
; Demonstrates copying 3 files and creating an icon.

; http://www.jrsoftware.org/isinfo.php

;  SEE THE DOCUMENTATION FOR DETAILS ON CREATING .ISS SCRIPT FILES!

; These files should be installed in C:\Program Files\Vamp Plugins
; There is a fair chance they wont work if they are installed anywhere else
; These were compiled with Visual Studio 2017 and have dependencies on the runtime

#define MyTitleName "Audacity Vamp Plugins for 64 Bit" 

[Setup]
; Tell Windows Explorer to reload the environment
ChangesEnvironment=yes
AppName=Audacity Vamp Plugins for 64 Bit
AppVersion=1.7
DefaultDirName="{pf64}\Vamp Plugins"

DefaultGroupName="Vamp Plugins"
;UninstallDisplayIcon={app}\"Audacity Vamp"/ Plugins.exe
Compression=lzma2
SolidCompression=yes
OutputDir=output
OutputBaseFilename=Vamp_Plugin64

[Files]

Source: "bin64/Vamp/Vamp Plugins/*.*"; DestDir: "{pf64}/Vamp Plugins"    ; Flags: recursesubdirs

[Icons]
Name: "{group}\Audacity Vamp Plugins"; Filename: "{app}\Vamp_Plugins32.exe"
