set cwd=%CD%

IF NOT EXIST "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\amd64" GOTO Preview
set PATH=C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\amd64;%PATH%
Echo VS Professional Detected
GOTO Start

:Preview
IF NOT EXIST "C:\Program Files\Microsoft Visual Studio\2022\Preview\MSBuild\Current\Bin\amd64" GOTO Community
set PATH=C:\Program Files\Microsoft Visual Studio\2022\Preview\MSBuild\Current\Bin\amd64;%PATH%
Echo VS Preview Detected
GOTO Start

:Community
IF NOT EXIST "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64" GOTO Start
set PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64;%PATH%
Echo VS Community Detected
:Start

cd ..
cd ..

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
msbuild.exe -restore -m:10 xLights.sln -p:Configuration="Release" -p:Platform="x64"
if %ERRORLEVEL% NEQ 0 goto error

%cwd%\prepmap x64\Release\xLights.map ..\bin64\xLights.map
if %ERRORLEVEL% NEQ 0 goto error
cd ..

mkdir build_scripts\msw\xLights
copy xLights\x64\Release\*.pdb build_scripts\msw\xLights

cd fseq_convert

cmake -S. -Bcmake_vs -G"Visual Studio 17 2022"
cmake --build cmake_vs --config Release
if %ERRORLEVEL% NEQ 0 goto error

cd ..

cd build_scripts
cd msw

goto exit

:error

@echo Error compiling x64
pause
exit 1

:exit
