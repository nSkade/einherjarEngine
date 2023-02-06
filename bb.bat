@echo off
make
if %ERRORLEVEL% EQU 0 (
	a.exe
)
