@set xlightsfolder=c:\program files\xlights

"%xlightsfolder%\xlDo.exe" -v -c "{\"cmd\":\"startxLights\", \"ifNotRunning\":\"true\"}"
@if errorlevel 1 goto abort

"%xlightsfolder%\xlDo.exe" -v -c "{\"cmd\":\"uploadSequence\", \"ip\":\"192.168.0.81\", \"media\":\"false\", \"format\":\"v2std\", \"seq\":\"aaa.xsq\"}"
@if errorlevel 1 goto abort

"%xlightsfolder%\xlDo.exe" -v -c "{\"cmd\":\"uploadSequence\", \"ip\":\"192.168.0.81\", \"media\":\"false\", \"format\":\"v2std\", \"seq\":\"bbb.xsq\"}"
@if errorlevel 1 goto abort

"%xlightsfolder%\xlDo.exe" -v -c "{\"cmd\":\"uploadSequence\", \"ip\":\"192.168.0.81\", \"media\":\"false\", \"format\":\"v2std\", \"seq\":\"ccc.xsq\"}"
@if errorlevel 1 goto abort

@goto end

:abort

@echo.
@echo Sequence upload aborted
@echo.

:end

@echo.
@echo.
@pause