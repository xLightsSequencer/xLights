cd x64\Debug

rem drmemory.exe -no_count_leaks xSchedule.exe
rem drmemory.exe -light xSchedule.exe
rem drmemory.exe -leaks_only xSchedule.exe
rem drmemory.exe -leaks_only -no_count_leaks xschedule.exe
rem drmemory.exe -leaks_only -no_count_leaks -no_track_allocs xschedule.exe

drmemory.exe -no_check_uninitialized -report_leak_max -1 -no_check_gdi xschedule.exe
pause
