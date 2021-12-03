@set xlightsfolder=c:\program files\xlights

@echo.
@echo Configuring F16V4
curl.exe http://127.0.0.1:49913/uploadController?ip=192.168.0.173
@if errorlevel 1 goto abort

@echo.
@echo Configuring F48V4
curl.exe http://127.0.0.1:49913/uploadController?ip=192.168.0.122
@if errorlevel 1 goto abort

@echo.
@echo Configuring FPP
curl.exe "http://127.0.0.1:49913/uploadConfig?ip=192.168.0.81&udp=all&models=true&map=false"
@if errorlevel 1 goto abort

@goto end

:abort

@echo.
@echo Controller upload aborted
@echo.

:end

@echo.
@echo.
@pause