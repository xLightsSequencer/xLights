rem This script assumes curl source repo is replicated into a sibling directory of the root xlights folder named "curl"

set VCINSTALLDIR=c:\program files\microsoft visual studio\2022\community\vc\

if [%1]==[NO64] goto x86
if [%2]==[NO64] goto x86

rem this ensures the x86 environment variables are also set
call "%VCINSTALLDIR%auxiliary\build\Vcvarsall.bat" x64

cd ..\..\..\curl\winbuild

rmdir /S /Q ..\builds\libcurl-vc-x64-release-dll-ipv6-sspi-schannel

nmake /f Makefile.vc mode=dll MACHINE=x64 

copy ..\builds\libcurl-vc-x64-release-dll-ipv6-sspi-schannel\lib\*.lib ..\..\xlights\lib\windows64
copy ..\builds\libcurl-vc-x64-release-dll-ipv6-sspi-schannel\bin\*.dll ..\..\xlights\bin64

rmdir /S /Q ..\builds\libcurl-vc-x64-debug-dll-ipv6-sspi-schannel

nmake /f Makefile.vc mode=dll MACHINE=x64 DEBUG=yes 

copy ..\builds\libcurl-vc-x64-debug-dll-ipv6-sspi-schannel\lib\*.lib ..\..\xlights\lib\windows64
copy ..\builds\libcurl-vc-x64-debug-dll-ipv6-sspi-schannel\bin\*.dll ..\..\xlights\bin64

:x86

if [%1]==[NO86] goto done
if [%2]==[NO86] goto done

rem this ensures the x86 environment variables are also set
call "%VCINSTALLDIR%auxiliary\build\Vcvarsall.bat" x86

rmdir /S /Q ..\builds\libcurl-vc-x86-release-dll-ipv6-sspi-schannel

nmake /f Makefile.vc mode=dll MACHINE=x86 

copy ..\builds\libcurl-vc-x86-release-dll-ipv6-sspi-schannel\lib\*.lib ..\..\xlights\lib\windows
copy ..\builds\libcurl-vc-x86-release-dll-ipv6-sspi-schannel\bin\*.dll ..\..\xlights\bin

rmdir /S /Q ..\builds\libcurl-vc-x86-debug-dll-ipv6-sspi-schannel

nmake /f Makefile.vc mode=dll MACHINE=x86 DEBUG=yes

copy ..\builds\libcurl-vc-x86-debug-dll-ipv6-sspi-schannel\lib\*.lib ..\..\xlights\lib\windows
copy ..\builds\libcurl-vc-x86-debug-dll-ipv6-sspi-schannel\bin\*.dll ..\..\xlights\bin

:done

copy ..\builds\libcurl-vc-x64-debug-dll-ipv6-sspi-schannel\include\curl\*.h ..\..\xlights\include\curl

cd ..\..\xlights\build_scripts\msw

pause
