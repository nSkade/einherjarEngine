# CMakeList.txt: CMake-Projekt für "GLsLhop". Schließen Sie die Quelle ein, und definieren Sie
# projektspezifische Logik hier.
#
cmake_minimum_required (VERSION 3.8)

project ("einherjarEngine")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

include_directories(glad)
include_directories(glfw3)
LINK_DIRECTORIES(glfw3)

# Fügen Sie der ausführbaren Datei dieses Projekts eine Quelle hinzu.
add_executable (einherjarEngine
	"src/main.cpp"
	"src/GPUTimer.cpp"
	"glad/glad.c"
)

target_link_libraries(einherjarEngine glfw3dll)

# TODO: Fügen Sie bei Bedarf Tests hinzu, und installieren Sie Ziele.
