; -- Audacity Vamp Plugins.iss --
; Demonstrates copying 3 files and creating an icon.

;  SEE THE DOCUMENTATION FOR DETAILS ON CREATING .ISS SCRIPT FILES!

; These files should be installed in C:\Program Files (x86)\Vamp Plugins
; There is a fair chance they wont work if they are installed anywhere else

#define MyTitleName "Audacity Vamp Plugins" 

[Setup]
; Tell Windows Explorer to reload the environment
ChangesEnvironment=yes
AppName=Audacity Vamp Plugins
AppVersion=1.7
DefaultDirName="{pf32}\Vamp Plugins"

DefaultGroupName="Vamp Plugins"
;UninstallDisplayIcon={app}\"Audacity Vamp"/ Plugins.exe
Compression=lzma2
SolidCompression=yes
OutputDir=output
OutputBaseFilename=Vamp_Plugin32

[Files]

Source: "bin\Vamp\Vamp Plugins/*.*"; DestDir: "{pf32}/Vamp Plugins"    ; Flags: recursesubdirs

[Icons]
Name: "{group}\Audacity Vamp Plugins"; Filename: "{app}\Vamp_Plugins32.exe"
