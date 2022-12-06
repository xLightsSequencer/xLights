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
cd ..

cd libltc
cd libltc
msbuild.exe -m:10 libltc.sln -p:Configuration="Release" -p:Platform="x64"
copy x64\release\libltc.lib ..\..\xlights\lib\windows64
if %ERRORLEVEL% NEQ 0 goto error

msbuild.exe -m:10 libltc.sln -p:Configuration="Release" -p:Platform="x86"
copy release\libltc.lib ..\..\xlights\lib\windows
if %ERRORLEVEL% NEQ 0 goto error

msbuild.exe -m:10 libltc.sln -p:Configuration="Debug" -p:Platform="x64"
copy x64\debug\libltcd.lib ..\..\xlights\lib\windows64
if %ERRORLEVEL% NEQ 0 goto error

msbuild.exe -m:10 libltc.sln -p:Configuration="Debug" -p:Platform="x86"
copy debug\libltcd.lib ..\..\xlights\lib\windows
if %ERRORLEVEL% NEQ 0 goto error

goto exit

:error

@echo Error compiling LibLTC
pause
exit 1

:exit
pause