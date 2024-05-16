// Scene Utilities OpenGl
#pragma once

#include "suCMN.hpp"

#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <glad/glad.h>

#include "GAPI/OGL/GPUTimer.hpp"
#include "GAPI/OGL/GLProgram.hpp"
#include "GAPI/OGL/GLFrameBuffer.hpp"
#include "GAPI/OGL/GLUtils.hpp"
#include "GAPI/OGL/OGLMesh.hpp"

//TODO seems not to work, remove
// try and use dedicated Graphics
// enable optimus!
extern "C" {
	_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
	_declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}
