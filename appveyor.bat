rem I need to rework this so it works for mingw builds as well

set "xlightsdir=%cd%"

git clone -q --branch=master https://github.com/wxWidgets/wxWidgets.git \projects\wxWidgets

cd ..\wxWidgets\build\msw

msbuild /m wx_custom_build.vcxproj /p:PlatformToolset=v141

sed -i 's/#   define wxUSE_GRAPHICS_CONTEXT 0/#   define wxUSE_GRAPHICS_CONTEXT 1/g' ..\..\include\wx\msw\setup.h

msbuild /m wx_vc14.sln /p:PlatformToolset=v141

cd %xlightsdir%

cd xLights

msbuild /m xLights.sln

cd ..

cd xSchedule

msbuild /m xSchedule.sln
