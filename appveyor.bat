echo on

set FROMSDK=10.0.16299.0
set TOSDK=10.0.14393.0

dir "\program files (x86)\windows kits\10\bin\"

dir
set "xlightsdir=%cd%"

rem Install wxWidgets
7z x -o\projects\wxWidgets wxWidgets-3.1.3.zip
if %ERRORLEVEL% NEQ 0 exit 1

rem Complete edits
rem dir \projects\wxWidgets

sed -i '#define wxUSE_STD_STRING_CONV_IN_WXSTRING wxUSE_STL/#define wxUSE_STD_STRING_CONV_IN_WXSTRING 1/g' \projects\wxWidgets\include\wx\msw\setup.h
if %ERRORLEVEL% NEQ 0 exit 1

sed -i '#define wxUSE_STD_CONTAINERS 0/#define wxUSE_STD_CONTAINERS 1/g' \projects\wxWidgets\include\wx\msw\setup.h
if %ERRORLEVEL% NEQ 0 exit 1

rem Setup Inno Setup
is.exe /LOG=inno.log /SUPPRESSMSGBOXES /SP- /NORESTART /NOCLOSEAPPLICATIONS /DIR=\projects\inno /NOICONS /SILENT
if %ERRORLEVEL% NEQ 0 exit 1

type inno.log

dir \projects\inno

rem =========================================== 64 BIT MSVC ===========================================
:x64ReleaseVS

rem prepare the header files for wxWidgets
cd ..\wxWidgets\build\msw
msbuild /m wx_custom_build.vcxproj /p:PlatformToolset=v%PLATFORMTOOLSET% /p:Configuration="Release"
if %ERRORLEVEL% NEQ 0 exit 1

sed -i "s/%FROMSDK%/%TOSDK%/g" wx_custom_build.vcxproj
sed -i "s/%FROMSDK%/%TOSDK%/g" wx_wxpng.vcxproj
sed -i "s/%FROMSDK%/%TOSDK%/g" wx_wxjpeg.vcxproj
sed -i "s/%FROMSDK%/%TOSDK%/g" wx_wxzlib.vcxproj
sed -i "s/%FROMSDK%/%TOSDK%/g" wx_wxtiff.vcxproj
sed -i "s/%FROMSDK%/%TOSDK%/g" wx_wxexpat.vcxproj
sed -i "s/%FROMSDK%/%TOSDK%/g" wx_wxscintilla.vcxproj
sed -i "s/%FROMSDK%/%TOSDK%/g" wx_wxregex.vcxproj
sed -i "s/%FROMSDK%/%TOSDK%/g" wx_base.vcxproj
sed -i "s/%FROMSDK%/%TOSDK%/g" wx_net.vcxproj
sed -i "s/%FROMSDK%/%TOSDK%/g" wx_core.vcxproj
sed -i "s/%FROMSDK%/%TOSDK%/g" wx_xml.vcxproj
sed -i "s/%FROMSDK%/%TOSDK%/g" wx_gl.vcxproj
sed -i "s/%FROMSDK%/%TOSDK%/g" wx_html.vcxproj
sed -i "s/%FROMSDK%/%TOSDK%/g" wx_media.vcxproj
sed -i "s/%FROMSDK%/%TOSDK%/g" wx_adv.vcxproj
sed -i "s/%FROMSDK%/%TOSDK%/g" wx_webview.vcxproj
sed -i "s/%FROMSDK%/%TOSDK%/g" wx_qa.vcxproj
sed -i "s/%FROMSDK%/%TOSDK%/g" wx_stc.vcxproj
sed -i "s/%FROMSDK%/%TOSDK%/g" wx_xrc.vcxproj
sed -i "s/%FROMSDK%/%TOSDK%/g" wx_richtext.vcxproj
sed -i "s/%FROMSDK%/%TOSDK%/g" wx_aui.vcxproj
sed -i "s/%FROMSDK%/%TOSDK%/g" wx_ribbon.vcxproj
sed -i "s/%FROMSDK%/%TOSDK%/g" wx_propgrid.vcxproj

rem Build wxWidgets
msbuild /m wx_vc15.sln /p:PlatformToolset=v%PLATFORMTOOLSET%
if %ERRORLEVEL% NEQ 0 exit 1

cd %xlightsdir%

cd xLights

sed -i "s/%FROMSDK%/%TOSDK%/g" Xlights.vcxproj

cd ..
cd xSchedule
sed -i "s/%FROMSDK%/%TOSDK%/g" xSchedule.vcxproj
if %ERRORLEVEL% NEQ 0 exit 1

cd xSMSDaemon
sed -i "s/%FROMSDK%/%TOSDK%/g" xSMSDaemon.vcxproj
if %ERRORLEVEL% NEQ 0 exit 1

cd ..
cd ..
cd xFade
sed -i "s/%FROMSDK%/%TOSDK%/g" xFade.vcxproj
if %ERRORLEVEL% NEQ 0 exit 1

cd ..
cd xCapture
sed -i "s/%FROMSDK%/%TOSDK%/g" xFade.vcxproj
if %ERRORLEVEL% NEQ 0 exit 1

cd ..
cd build_scripts
cd msw
call PackageWindowsRelease_VS-x64Only.cmd

exit 0
