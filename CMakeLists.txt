cmake_minimum_required (VERSION 3.8)

project ("einherjarEngine")

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
SET(VCPKG_BOOTSTRAP_OPTIONS "-disableMetrics") # Disable telemetry for vcpkg.
SET(X_VCPKG_APPLOCAL_DEPS_INSTALL ON)          # Install vcpkg dependencies automatically(experimental - might be changed or removed later; see: https://github.com/microsoft/vcpkg/issues/1653). 

#include(CMakePrintHelpers)
#cmake_print_variables(CMAKE_TOOLCHAIN_FILE)
#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

find_package(Vulkan REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

include_directories(einherjarEngine
	lib/
	lib/glad           #TODO add git submodule
	lib/imgui          #TODO add git submodule
	lib/imgui/backends #TODO add git submodule
	lib/glfw3          #TODO add git submodule
	lib/stb
	src/
	${Stb_INCLUDE_DIR}
)
LINK_DIRECTORIES(lib/glad)
LINK_DIRECTORIES(lib/glfw3)
LINK_DIRECTORIES(/)

SET(CMAKE_COLOR_MAKEFILE OFF)

# src files
file(GLOB_RECURSE einherjarEngine_SRC
	"src/*.cpp"
)
file(GLOB imgui_SRC
	"lib/imgui/*.cpp"
)
set(ehjSourceFiles )
list(APPEND ehjSourceFiles
	"xicon/my.rc"
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
	Vulkan::Vulkan
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
