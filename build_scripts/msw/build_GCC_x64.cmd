
"C:\Program Files (x86)\CodeBlocks\codeblocks.exe" --build --target=64bit_MinGW_Release ..\..\xSchedule\xSMSDaemon\xSMSDaemon.cbp

"C:\Program Files (x86)\CodeBlocks\codeblocks.exe" --build --target=64bit_MinGW_Release ..\..\xSchedule\RemoteFalcon\RemoteFalcon.cbp

"C:\Program Files (x86)\CodeBlocks\codeblocks.exe" --build --target=64bit_MinGW_Release ..\..\xfade\xfade.cbp

"C:\Program Files (x86)\CodeBlocks\codeblocks.exe" --build --target=64bit_MinGW_Release ..\..\xCapture\xCapture.cbp

"C:\Program Files (x86)\CodeBlocks\codeblocks.exe" --build --target=64bit_MinGW_Release ..\..\xSchedule\xSchedule.cbp

"C:\Program Files (x86)\CodeBlocks\codeblocks.exe" --build "--target=64bit MinGW_Release" ..\..\xLights\xLights.cbp

if %1==1 goto exit

pause

:exit