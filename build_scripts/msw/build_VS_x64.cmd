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
set PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64;%PATH%
Echo VS Community Detected
:Start

cd ..
cd ..

cd xlDo
msbuild.exe -m:10 xlDo.sln -p:Configuration="Release" -p:Platform="x64"
if %ERRORLEVEL% NEQ 0 goto error
cd ..

mkdir build_scripts\msw\xlDo
copy xlDo\x64\Release\*.pdb build_scripts\msw\xlDo

cd xFade
msbuild.exe -m:10 xFade.sln -p:Configuration="Release" -p:Platform="x64"
if %ERRORLEVEL% NEQ 0 goto error
cd ..

mkdir build_scripts\msw\xFade
copy xFade\x64\Release\*.pdb build_scripts\msw\xFade

cd xCapture
msbuild.exe -m:10 xCapture.sln -p:Configuration="Release" -p:Platform="x64"
if %ERRORLEVEL% NEQ 0 goto error
cd ..

mkdir build_scripts\msw\xCapture
copy xCapture\x64\Release\*.pdb build_scripts\msw\xCapture

cd xScanner
msbuild.exe -m:10 xScanner.sln -p:Configuration="Release" -p:Platform="x64"
if %ERRORLEVEL% NEQ 0 goto error
cd ..

mkdir build_scripts\msw\xScanner
copy xScanner\x64\Release\*.pdb build_scripts\msw\xScanner

cd xSchedule
msbuild.exe -m:10 xSchedule.sln -p:Configuration="Release" -p:Platform="x64"
if %ERRORLEVEL% NEQ 0 goto error
cd ..

mkdir build_scripts\msw\xSchedule
copy xSchedule\x64\Release\*.pdb build_scripts\msw\xSchedule

cd xSchedule
cd xSMSDaemon
msbuild.exe -m:10 xSMSDaemon.sln -p:Configuration="Release" -p:Platform="x64"
if %ERRORLEVEL% NEQ 0 goto error
cd ..
cd ..

mkdir build_scripts\msw\xSMSDaemon
copy xSchedule\xSMSDaemon\x64\Release\*.pdb build_scripts\msw\xSMSDaemon

cd xSchedule
cd RemoteFalcon
msbuild.exe -m:10 RemoteFalcon.sln -p:Configuration="Release" -p:Platform="x64"
if %ERRORLEVEL% NEQ 0 goto error
cd ..
cd ..

mkdir build_scripts\msw\RemoteFalcon
copy xScedhule\RemoteFalcon\x64\Release\*.pdb build_scripts\msw\RemoteFalcon

cd xLights
msbuild.exe -m:10 xLights.sln -p:Configuration="Release" -p:Platform="x64"
if %ERRORLEVEL% NEQ 0 goto error
cd ..

mkdir build_scripts\msw\xLights
copy xLights\x64\Release\*.pdb build_scripts\msw\xLights

cd build_scripts
cd msw

goto exit

:error

@echo Error compiling x64
pause
exit 1

:exit