@echo off
set /a "NOPMO=%NUMBER_OF_PROCESSORS%-1"
if "%1"=="-my" (goto :b)
if "%1"=="-nj" (goto :c)

:a
::make -j %NOPMO%
cd build & cmake .. -DCMAKE_BUILD_TYPE=Release & cmake --build . --target ehjEngine -j & cd..
goto :end

:b
if "%2"=="-nj" (goto :d)
::make -j %NOPMO% "MYENVIR = $(TRUE)"
cd build & cmake .. -DCMAKE_BUILD_TYPE=Release & cmake --build . --target ehjEngineMY -j & cd..
goto :end

:c
if "%2"=="-my" (goto :d)
::make -j %NOPMO%
cd build & cmake .. -DCMAKE_BUILD_TYPE=Release & cmake --build . --target ehjEngine & cd..
goto :end

:d
::make -j %NOPMO% "MYENVIR = $(TRUE)"
cd build & cmake .. -DCMAKE_BUILD_TYPE=Release & cmake --build . --target ehjEngineMY & cd..
goto :end

:end
