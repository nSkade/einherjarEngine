@echo off
chcp 65001
:: set the path to your visual studio vcvars script, it is different for every version of Visual Studio.
set VS2019TOOLS="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
::"C:\Program Files(x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

:: make sure we found them
if not exist %VS2019TOOLS% (
    echo VS 2019 Build Tools are missing!
    exit
)

:: call that script, which essentially sets up the VS Developer Command Prompt
call %VS2019TOOLS%

set /a "NOPMO=%NUMBER_OF_PROCESSORS%-1"
:: run the compiler with your arguments
make "CXX = cl.exe /Od /EHsc /Zi" "BUILDFOLDER = out/mfdbg" "TARGET = einherjarEngineDbg" "ARGO = /c /FS" "ARGOP = /std:c++17 /c /FS" "ARGF = /std:c++17" "OUTF = /link /out:" "OUTC = /Fo:" "INC = /Ilib/imgui /Ilib/imgui/backends /Ilib/glfw3 /Ilib /DEHJ_DBG /D_DEBUG /MDd" "LIB = lib/glad/glad.lib glfw3.lib /LIBPATH:"""C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\lib\x64""" /LIBPATH:"""C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\um\x64""" /LIBPATH:"""C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\ucrt\x64"""" "VULKAN_INC = /I$(VULKAN_PATH)/Include" "VULKAN_LIB = $(VULKAN_PATH)/Lib/vulkan-1.lib" -j %NOPMO%
exit
