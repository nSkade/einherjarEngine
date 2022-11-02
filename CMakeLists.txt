# CMakeList.txt: CMake-Projekt für "GLsLhop". Schließen Sie die Quelle ein, und definieren Sie
# projektspezifische Logik hier.
#
cmake_minimum_required (VERSION 3.8)

project ("einherjarEngine")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

include_directories(lib/glad)
include_directories(lib/glfw3)
include_directories(lib/imgui-1.88)
LINK_DIRECTORIES(lib/glfw3)

# Fügen Sie der ausführbaren Datei dieses Projekts eine Quelle hinzu.
add_executable (einherjarEngine
	"src/main.cpp"
	"src/GPUTimer.cpp"
	"src/GLProgram.cpp"
	"lib/glad/glad.c"
	"lib/imgui-1.88.cpp"
)

target_link_libraries(einherjarEngine glfw3dll)

# TODO: Fügen Sie bei Bedarf Tests hinzu, und installieren Sie Ziele.
