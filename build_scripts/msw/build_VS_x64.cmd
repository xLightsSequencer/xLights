set PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\amd64;%PATH%

cd ..
cd ..

cd xFade
msbuild.exe /m xFade.sln /p:Configuration="Release" /p:Platform="x64"
if %ERRORLEVEL% NEQ 0 goto error
cd ..

cd xCapture
msbuild.exe /m xCapture.sln /p:Configuration="Release" /p:Platform="x64"
if %ERRORLEVEL% NEQ 0 goto error
cd ..

cd xSchedule
msbuild.exe /m xSchedule.sln /p:Configuration="Release" /p:Platform="x64"
if %ERRORLEVEL% NEQ 0 goto error
cd ..

cd xSchedule
cd xSMSDaemon
msbuild.exe /m xSMSDaemon.sln /p:Configuration="Release" /p:Platform="x64"
if %ERRORLEVEL% NEQ 0 goto error
cd ..
cd ..

cd xLights
msbuild.exe /m xLights.sln /p:Configuration="Release" /p:Platform="x64"
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