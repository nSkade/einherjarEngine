clang src/main.cpp src/GPUTimer.cpp src/GLProgram.cpp glad/glad.c -o einherjarEngine.exe -Lfolder -lglad -lglfw3
if %ERRORLEVEL% EQU 0 a.exe
