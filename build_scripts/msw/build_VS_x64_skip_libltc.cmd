set cwd=%CD%
echo %1
rem Ask vswhere for MSBuild rather than naming Visual Studio 2022 install paths.
rem xLights needs the v145 toolset, so a 2022 entry placed ahead of a newer
rem MSBuild produces MSB8020 - the build tools for v145 cannot be found.
set VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist %VSWHERE% goto NoVsWhere
for /f "usebackq tokens=*" %%i in (`%VSWHERE% -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\amd64\MSBuild.exe`) do set MSBUILD_DIR=%%~dpi
if not defined MSBUILD_DIR goto NoVsWhere
set PATH=%MSBUILD_DIR%;%PATH%
echo Using MSBuild from %MSBUILD_DIR%
goto Start

:NoVsWhere
echo vswhere found no MSBuild - relying on whatever is already on PATH

:Start

cd ..
cd ..

rem Stage the dependency bundle before anything compiles. xlDo and fseq_convert
rem both consume it, and both are built below before xLights - whose project
rem file carries the only pre-build fetch, so relying on that leaves them
rem compiling against a bundle that is not there yet.
powershell -NoProfile -ExecutionPolicy Bypass -File ci_scripts\fetch_dependencies.ps1
if %ERRORLEVEL% NEQ 0 goto error

cd TipOfDay
cd Tool
msbuild.exe -m:10 PrepTOD.sln -p:Configuration="Release" -p:Platform="x64"
if %ERRORLEVEL% NEQ 0 goto error
cd ..
cd ..

TipOfDay\Tool\x64\Release\PrepTOD.exe TipOfDay


cd xlDo
msbuild.exe -m:10 xlDo.sln -p:Configuration="Release" -p:Platform="x64"
if %ERRORLEVEL% NEQ 0 goto error

%cwd%\prepmap x64\Release\xlDo.map ..\bin64\xlDo.map
if %ERRORLEVEL% NEQ 0 goto error
cd ..

mkdir build_scripts\msw\xlDo
copy xlDo\x64\Release\*.pdb build_scripts\msw\xlDo

cd xLights
msbuild.exe -restore -m:10 xLights.sln -p:Configuration="Release" -p:Platform="x64" /p:DefineConstants=%1 /p:DisableSpecificWarnings="4244;4267"
if %ERRORLEVEL% NEQ 0 goto error

%cwd%\prepmap x64\Release\xLights.map ..\bin64\xLights.map
if %ERRORLEVEL% NEQ 0 goto error
cd ..

mkdir build_scripts\msw\xLights
copy xLights\x64\Release\*.pdb build_scripts\msw\xLights

cd fseq_convert

cmake -S. -Bcmake_vs
cmake --build cmake_vs --config Release
if %ERRORLEVEL% NEQ 0 goto error

cd ..

cd build_scripts
cd msw

goto exit

:error

@echo Error compiling x64
exit 1

:exit
