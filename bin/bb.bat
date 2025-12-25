@echo off
call m.bat %1 %2
if %ERRORLEVEL% EQU 0 (
	a.exe
)
