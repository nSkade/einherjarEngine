@echo off
set /a "NOPMO=%NUMBER_OF_PROCESSORS%-1"
if "%1"=="-my" (goto :a)
goto :b
:a
make -j %NOPMO% "MYENVIR = $(TRUE)"
goto :end
:b
make -j %NOPMO%
:end