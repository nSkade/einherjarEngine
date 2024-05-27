cmake_minimum_required (VERSION 3.8)

project ("einherjarEngine")

find_package(Vulkan REQUIRED FATAL_ERROR)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

include_directories(einherjarEngine
	lib/glad
	lib/imgui
	lib/imgui/backends
	lib/glfw3
	lib/
	src/
	PUBLIC C:/VulkanSDK/1.3.275.0/Include #TODO abstract
)
LINK_DIRECTORIES(lib/glad)
LINK_DIRECTORIES(lib/glfw3)
LINK_DIRECTORIES(/)

SET(CMAKE_COLOR_MAKEFILE OFF)

#OPTION(MYENVIR "Compile with main.cpp in myScenes" OFF)
#IF(MYENVIR)
#	ADD_DEFINITIONS(-DMYENVIR)
#ENDIF(MYENVIR)
#unset(MYENVIR CACHE)

# src files
file(GLOB_RECURSE einherjarEngine_SRC
	"src/*.cpp"
)
file(GLOB imgui_SRC
	"lib/imgui/*.cpp"
)
set(ehjSourceFiles )
list(APPEND ehjSourceFiles
	"my.rc"
	${einherjarEngine_SRC}
	
	"lib/glad/glad.c"

	${imgui_SRC}
	"lib/imgui/backends/imgui_impl_opengl3.cpp"
	"lib/imgui/backends/imgui_impl_glfw.cpp"
	"lib/imgui/misc/cpp/imgui_stdlib.cpp"
)

add_executable (ehjEngine
	${ehjSourceFiles}
	"scenes/main.cpp"
)
add_executable (ehjEngineMY
	${ehjSourceFiles}
	"myScenes/main.cpp"
)

set_property(TARGET ehjEngine PROPERTY OUTPUT_NAME einherjarEngine)
set_property(TARGET ehjEngineMY PROPERTY OUTPUT_NAME einherjarEngine)

add_custom_target(file_toucher 
		COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${CMAKE_CURRENT_SOURCE_DIR}/scenes/main.cpp)
add_custom_target(file_toucherMY 
		COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${CMAKE_CURRENT_SOURCE_DIR}/myScenes/main.cpp)

add_dependencies(ehjEngine file_toucher)
add_dependencies(ehjEngineMY file_toucherMY)

set(linkedLibraries
	glfw3
	C:/VulkanSDK/1.3.275.0/Lib/vulkan-1.lib #TODO more libraries
	#C:/VulkanSDK/1.3.275.0/Lib/volk.lib #TODO use vulkan.hpp instead
)
target_link_libraries(ehjEngine ${linkedLibraries})
target_link_libraries(ehjEngineMY ${linkedLibraries})

target_compile_definitions(ehjEngine PUBLIC $<$<CONFIG:Debug>:EHJ_DBG>)
target_compile_definitions(ehjEngineMY PUBLIC $<$<CONFIG:Debug>:EHJ_DBG>)

set_target_properties (ehjEngine PROPERTIES CXX_STANDARD 17)
set_target_properties (ehjEngine PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set_target_properties (ehjEngine PROPERTIES DEBUG_POSTFIX "Dbg")

set_target_properties (ehjEngineMY PROPERTIES CXX_STANDARD 17)
set_target_properties (ehjEngineMY PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set_target_properties (ehjEngineMY PROPERTIES DEBUG_POSTFIX "Dbg")
