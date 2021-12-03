"%xlightsfolder%\xlDo.exe" -v -c "{\"cmd\":\"checkSequence\", \"seq\":\"%1\"}" -s result.cmd
@if errorlevel 1 goto end

@call result.cmd

@start notepad.exe "%output%"

:end
