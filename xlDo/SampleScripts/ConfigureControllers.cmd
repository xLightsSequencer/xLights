@set xlightsfolder=c:\program files\xlights

"%xlightsfolder%\xlDo.exe" -v -c "{\"cmd\":\"startxLights\", \"ifNotRunning\":\"true\"}"
@if errorlevel 1 goto abort

@echo Configuring F16V4
"%xlightsfolder%\xlDo.exe" -v -c "{\"cmd\":\"uploadController\", \"ip\":\"192.168.0.173\"}"
@if errorlevel 1 goto abort

@echo Configuring F48V4
"%xlightsfolder%\xlDo.exe" -v -c "{\"cmd\":\"uploadController\", \"ip\":\"192.168.0.122\"}"
@if errorlevel 1 goto abort

@echo Configuring FPP
"%xlightsfolder%\xlDo.exe" -v -c "{\"cmd\":\"uploadFPPConfig\", \"ip\":\"192.168.0.81\", \"udp\":\"all\", \"models\":\"true\", \"map\":\"false\"}"
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