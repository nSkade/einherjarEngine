@echo off
call m.bat %1
if %ERRORLEVEL% EQU 0 (
	a.exe
)
