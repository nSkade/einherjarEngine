cmake_minimum_required (VERSION 3.8)

project ("einherjarEngine")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

include_directories(lib/glad)
include_directories(lib/imgui)
include_directories(lib/glfw3)
LINK_DIRECTORIES(lib/glad)
LINK_DIRECTORIES(lib/glfw3)

add_executable (einherjarEngine
	"src/main.cpp"
	"src/GPUTimer.cpp"
	"src/GLProgram.cpp"
	"src/Mesh.cpp"

	"lib/glad/glad.c"
	
	"lib/imgui/imgui.cpp"
	"lib/imgui/imgui_demo.cpp"
	"lib/imgui/imgui_draw.cpp"
	"lib/imgui/imgui_tables.cpp"
	"lib/imgui/imgui_widgets.cpp"
	"lib/imgui/backends/imgui_impl_opengl3.cpp"
	"lib/imgui/backends/imgui_impl_glfw.cpp"
)
target_link_libraries(einherjarEngine glfw3)

set_target_properties (einherjarEngine PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

