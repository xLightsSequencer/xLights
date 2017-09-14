rem I need to rework this so it works for mingw builds as well

echo on

set "xlightsdir=%cd%"

git clone -q --branch=master https://github.com/wxWidgets/wxWidgets.git \projects\wxWidgets
if %ERRORLEVEL% NEQ 0 exit 1

rem prepare the header files for wxWidgets

cd ..\wxWidgets\build\msw
msbuild /m wx_custom_build.vcxproj /p:PlatformToolset=v%PLATFORMTOOLSET% /p:Configuration="Release"
if %ERRORLEVEL% NEQ 0 exit 1

sed -i 's/#   define wxUSE_GRAPHICS_CONTEXT 0/#   define wxUSE_GRAPHICS_CONTEXT 1/g' ..\..\include\wx\msw\setup.h
if %ERRORLEVEL% NEQ 0 exit 1

sed -i 's/bool IsKindOf(const wxClassInfo *info) const/bool __attribute__((optimize("O0"))) IsKindOf(const wxClassInfo *info) const/g' ..\..\include\wx\rtti.h
if %ERRORLEVEL% NEQ 0 exit 1

if [%BUILDTYPE%] EQU [X64DEBUGVS] goto x64DebugVS 
if [%BUILDTYPE%] EQU [X64RELEASEGCC] goto x64ReleaseGCC 
if [%BUILDTYPE%] EQU [X86RELEASEGCC] goto x86ReleaseGCC 

echo "Unknown build type " %BUILDTYPE%

exit 1

:x64DebugVS

rem Build wxWidgets

msbuild /m wx_vc14.sln /p:PlatformToolset=v%PLATFORMTOOLSET%
if %ERRORLEVEL% NEQ 0 exit 1

cd %xlightsdir%

cd xLights

msbuild /m xLights.sln /p:PlatformToolset=v%PLATFORMTOOLSET%
if %ERRORLEVEL% NEQ 0 exit 1

cd ..

cd xSchedule

msbuild /m xSchedule.sln /p:PlatformToolset=v%PLATFORMTOOLSET%
if %ERRORLEVEL% NEQ 0 exit 1

exit 0

:x86ReleaseGCC

set PATH=%PATH%;C:\MinGW\bin

cd %xlightsdir%

7z e cbp2make-stl-rev147-all.tar.7z -o. cbp2make-stl-rev147-all\bin\Release\cbp2make.exe
if %ERRORLEVEL% NEQ 0 exit 1

cd xlights

..\cbp2make.exe -in xLights.cbp -out xLights.cbp.mak --with-deps --keep-outdir --keep-objdir -windows -targets %configuration%,MinGW_Release
if %ERRORLEVEL% NEQ 0 exit 1

type xLights.cbp.mak

mingw32-make -f xLights.cbp.mak CXXFLAGS="-std=gnu++14" WX=\projects\wxWidgets INC=-I\projects\wxWidgets\include -j 1 %configuration%
if %ERRORLEVEL% NEQ 0 exit 1

cd ..\xSchedule

..\cbp2make.exe -in xSchedule.cbp -out xSchedule.cbp.mak --with-deps --keep-outdir --keep-objdir -windows -targets %configuration%
if %ERRORLEVEL% NEQ 0 exit 1

mingw32-make -f xSchedule.cbp.mak CXXFLAGS="-std=gnu++14" WX=\projects\wxWidgets INC=-I\projects\wxWidgets\include -j 10 %configuration%
if %ERRORLEVEL% NEQ 0 exit 1

exit 0

:x64ReleaseGCC

cd %xlightsdir%

7z e cbp2make-stl-rev147-all.tar.7z -o. cbp2make-stl-rev147-all\bin\Release\cbp2make.exe
if %ERRORLEVEL% NEQ 0 exit 1

cbp2make.exe -in xLights/xLights.cbp -out xLights/xLights.cbp.mak --with-deps --keep-outdir --keep-objdir - windows
if %ERRORLEVEL% NEQ 0 exit 1

type xLights\xLights.cbp.mak

dir C:\mingw-w64\i686-5.3.0-posix-dwarf-rt_v4-rev0\MinGW\bin

exit 0
