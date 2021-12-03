@set xlightsfolder=c:\program files\xlights

"%xlightsfolder%\xlDo.exe" -v -c "{\"cmd\":\"startxLights\", \"ifNotRunning\":\"true\"}"
@if errorlevel 1 goto abort

@call checkASequence "aaa.xsq"
@if errorlevel 1 goto abort

@call checkASequence "bbb.xsq"
@if errorlevel 1 goto abort

@call checkASequence "ccc.xsq"
@if errorlevel 1 goto abort

@goto end

:abort

@echo.
@echo Check sequence aborted
@echo.

:end

@echo.
@echo.
@pause