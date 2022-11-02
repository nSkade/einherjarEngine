set DIR=lib/imgui/
set IMGUI=lib/imgui/imgui.cpp %DIR%imgui_demo.cpp %DIR%imgui_draw.cpp %DIR%imgui_tables.cpp %DIR%imgui_widgets.cpp lib/imgui/backends/imgui_impl_opengl3.cpp lib/imgui/backends/imgui_impl_glfw.cpp
clang src/main.cpp src/GPUTimer.cpp src/GLProgram.cpp %IMGUI% lib/glad/glad.c -o einherjarEngine.exe -Lfolder -lglad -lglfw3 -Ilib -Ilib/imgui -Ilib/imgui/backends
if %ERRORLEVEL% EQU 0 a.exe
