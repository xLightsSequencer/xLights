@if [%1]==[] goto usage

@set xlightsfolder=c:\program files\xlights

"%xlightsfolder%\xlDo.exe" -v -c "{\"cmd\":\"startxLights\", \"ifNotRunning\":\"true\"}"
@if errorlevel 1 goto abort

"%xlightsfolder%\xlDo.exe" -v -t %1
@if errorlevel 1 goto abort

@goto end

:usage

@echo.
@echo USAGE BatchRender batch render json file
@echo.

@goto end

:abort

@echo.
@echo Batch render aborted
@echo.

:end

@echo.
@echo.
@pause