#pragma once

//#ifndef EHJ_DBG
//TODO imgui wont allow overwriting of new
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
//#endif

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "Utility/GLFWFPSLimiter.hpp"

#define GLM_FORCE_XYZW_ONLY

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GAPI/OGL/GPUTimer.hpp"
#include "GAPI/OGL/GLProgram.hpp"
#include "GAPI/OGL/GLFrameBuffer.hpp"
#include "GAPI/OGL/GLUtils.hpp"
#include "Mesh.hpp"
#include "GAPI/OGL/OGLMesh.hpp"
#include "Camera.hpp"
#include "Clock.hpp" //TODO replace with "Utility/Timer.hpp"
#include "Utility/Timer.hpp"

/**
 * @brief Scene utility for OpenGL.
*/
class IScene {
public:
	virtual void setup() = 0;
	virtual int run() = 0;
	virtual void cleanup() = 0;
};

// try and use dedicated Graphics
// enable optimus!
extern "C" {
	_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
	_declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}
