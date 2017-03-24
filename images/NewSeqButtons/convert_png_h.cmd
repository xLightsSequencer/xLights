@echo off
for %%1 in (*.png) do (
del ..\..\include\%%~n1.h
bin2c -c %%~n1.png ..\..\include\%%~n1.h
)