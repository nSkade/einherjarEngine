// Scene Utilities OpenGl
#pragma once

#include "suCMN.hpp"

//TODO remove
//#if defined(__clang__) // ignore trival copyable warnings
//#pragma clang diagnostic push
//#pragma clang diagnostic ignored "-Wnontrivial-memaccess"
//#endif

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

//TODO remove
//#if defined(__clang__)
//#pragma clang diagnostic pop
//#endif

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
