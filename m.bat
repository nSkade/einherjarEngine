@echo off
set /a "NOPMO=%NUMBER_OF_PROCESSORS%-1"
make -j %NOPMO%