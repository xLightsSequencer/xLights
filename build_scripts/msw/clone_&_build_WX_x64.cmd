IF NOT EXIST "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\amd64" GOTO Community
set PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\amd64;%PATH%
Echo VS Professional Detected
GOTO Pro

:Community
set PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\amd64;%PATH%
Echo VS Community Detected
:PRO

cd ..
cd ..
cd ..

git clone --recurse-submodules -b xlights_2021.31 https://github.com/xLightsSequencer/wxWidgets wxWidgets

cd wxWidgets
msbuild.exe /m .\build\msw\wx_vc16.sln /p:Configuration="Debug" /p:Platform="x64"
if %ERRORLEVEL% NEQ 0 goto error


msbuild.exe /m .\build\msw\wx_vc16.sln /p:Configuration="Release" /p:Platform="x64"
if %ERRORLEVEL% NEQ 0 goto error


goto exit

:error

@echo Error compiling wxWidgets x64
pause
exit 1

:exit