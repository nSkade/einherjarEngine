cmake_minimum_required (VERSION 3.8)

project ("einherjarEngine")

find_package(Vulkan REQUIRED FATAL_ERROR)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

include_directories(einherjarEngine
	lib/glad
	lib/imgui
	lib/glfw3
	lib
	PUBLIC C:/VulkanSDK/1.3.231.1/Include
)
LINK_DIRECTORIES(lib/glad)
LINK_DIRECTORIES(lib/glfw3)
LINK_DIRECTORIES(/)

add_executable (einherjarEngine
	"src/main.cpp"
	"src/Mesh.cpp"
	"src/Input/Input.cpp"
	"src/Camera.cpp"

	"src/Structures/ADF.cpp"
	"src/MeshProcessing/DCADF.cpp"

	"src/GAPI/OGL/GPUTimer.cpp"
	"src/GAPI/OGL/GLProgram.cpp"
	"src/GAPI/OGL/GLUtils.cpp"
	"src/GAPI/OGL/OGLMesh.cpp"
	
	"lib/glad/glad.c"
	
	"lib/imgui/imgui.cpp"
	"lib/imgui/imgui_demo.cpp"
	"lib/imgui/imgui_draw.cpp"
	"lib/imgui/imgui_tables.cpp"
	"lib/imgui/imgui_widgets.cpp"
	"lib/imgui/backends/imgui_impl_opengl3.cpp"
	"lib/imgui/backends/imgui_impl_glfw.cpp"
)
target_link_libraries(einherjarEngine
	glfw3
	C:/VulkanSDK/1.3.231.1/Lib/vulkan-1.lib
)
target_compile_definitions(einherjarEngine
PRIVATE
PUBLIC
	$<$<CONFIG:Debug>:EHJ_DBG>
#	$<$<CONFIG:RelWithDebInfo>:>
#	$<$<CONFIG:Release>:>
#	$<$<CONFIG:MinSizeRel>:>
)

set_target_properties (einherjarEngine PROPERTIES CXX_STANDARD 17)
set_target_properties (einherjarEngine PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

