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
msbuild.exe -m:10 xlDo.sln -p:Configuration="Release" -p:Platform="x86"
if %ERRORLEVEL% NEQ 0 goto error
cd ..

cd xFade
msbuild.exe -m:10 xFade.sln -p:Configuration="Release" -p:Platform="x86"
if %ERRORLEVEL% NEQ 0 goto error
cd ..

cd xCapture
msbuild.exe -m:10 xCapture.sln -p:Configuration="Release" -p:Platform="x86"
if %ERRORLEVEL% NEQ 0 goto error
cd ..

cd xScanner
msbuild.exe -m:10 xScanner.sln -p:Configuration="Release" -p:Platform="x86"
if %ERRORLEVEL% NEQ 0 goto error
cd ..

cd xSchedule
msbuild.exe -m:10 xSchedule.sln -p:Configuration="Release" -p:Platform="x86"
if %ERRORLEVEL% NEQ 0 goto error
cd ..

cd xSchedule
cd xSMSDaemon
msbuild.exe -m:10 xSMSDaemon.sln -p:Configuration="Release" -p:Platform="x86"
if %ERRORLEVEL% NEQ 0 goto error
cd ..
cd ..

cd xSchedule
cd RemoteFalcon
msbuild.exe -m:10 RemoteFalcon.sln -p:Configuration="Release" -p:Platform="x86"
if %ERRORLEVEL% NEQ 0 goto error
cd ..
cd ..

cd xLights
rem del Release\Xlights.ipdb
msbuild.exe -restore -m:10 xLights.sln -p:Configuration="Release" -p:Platform="x86" -p:PreferredToolArchitecture="x64"
if %ERRORLEVEL% NEQ 0 goto error
cd ..

cd build_scripts
cd msw

goto exit

:error

@echo Error compiling x86
pause
exit 1

:exit