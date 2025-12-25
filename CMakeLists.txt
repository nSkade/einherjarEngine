cmake_minimum_required (VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

set(VCPKG_TARGET_TRIPLET x64-mingw-dynamic)
set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg-clang-mingw.cmake")

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
SET(VCPKG_BOOTSTRAP_OPTIONS "-disableMetrics") # Disable telemetry for vcpkg.
SET(X_VCPKG_APPLOCAL_DEPS_INSTALL ON)          # Install vcpkg dependencies automatically(experimental - might be changed or removed later; see: https://github.com/microsoft/vcpkg/issues/1653). 

# use local vcpkg_installed folder at the root of the project
set(VCPKG_INSTALLED_DIR "${CMAKE_SOURCE_DIR}/vcpkg_installed")
message(VCPKG_TARGET_TRIPLET "${VCPKG_TARGET_TRIPLET}")
list(APPEND CMAKE_PREFIX_PATH "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")

#include(CMakePrintHelpers)
#cmake_print_variables(CMAKE_TOOLCHAIN_FILE)
#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

project ("einherjarEngine")

find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
#find_package(tinyobjloader REQUIRED)
find_package(RapidObj CONFIG REQUIRED)
find_package(fastgltf CONFIG REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

include_directories(einherjarEngine
	src/
	lib/
	lib/stb
	${Stb_INCLUDE_DIR} #TODOf required?
)
#LINK_DIRECTORIES(lib/glad)
#LINK_DIRECTORIES(lib/glfw3)
LINK_DIRECTORIES(/)

SET(CMAKE_COLOR_MAKEFILE OFF)

# src files
file(GLOB_RECURSE einherjarEngine_SRC
	"src/*.cpp"
)
set(ehjSourceFiles )
list(APPEND ehjSourceFiles
	"lib/ehjEicon/my.rc"
	${einherjarEngine_SRC}
	
	#TODO remove
	#"lib/glad/glad.c"
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
set_property(TARGET ehjEngineMY PROPERTY OUTPUT_NAME einherjarEngineMY)

#TODO(skade) old filetoucher for same exe at MY and non MY
#add_custom_target(file_toucher 
#		COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${CMAKE_CURRENT_SOURCE_DIR}/scenes/main.cpp)
#add_custom_target(file_toucherMY 
#		COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${CMAKE_CURRENT_SOURCE_DIR}/myScenes/main.cpp)
#
#add_dependencies(ehjEngine file_toucher)
#add_dependencies(ehjEngineMY file_toucherMY)

set(linkedLibraries
	glm::glm
	glfw
	glad::glad
	Vulkan::Vulkan
	imgui::imgui

	# always link release
	#$<$<CONFIG:Debug>:"vcpkg_installed/x64-windows/lib/tinyobjloader">
	#$<$<CONFIG:Release>:tinyobjloader::tinyobjloader>
	rapidobj::rapidobj
	fastgltf::fastgltf
)

target_link_libraries(ehjEngine ${linkedLibraries})
target_link_libraries(ehjEngineMY ${linkedLibraries})

target_compile_definitions(ehjEngine PUBLIC $<$<CONFIG:Debug>:EHJ_DBG>)
target_compile_definitions(ehjEngineMY PUBLIC $<$<CONFIG:Debug>:EHJ_DBG>)

#set_target_properties (ehjEngine PROPERTIES CXX_STANDARD 17)
set_target_properties (ehjEngine PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set_target_properties (ehjEngine PROPERTIES DEBUG_POSTFIX "Dbg")

#set_target_properties (ehjEngineMY PROPERTIES CXX_STANDARD 17)
set_target_properties (ehjEngineMY PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set_target_properties (ehjEngineMY PROPERTIES DEBUG_POSTFIX "Dbg")
