// Scene Utilities OpenGL
#pragma once

#include "../../lib/imgui/imgui.h"
#include "../../lib/imgui/backends/imgui_impl_glfw.h"
#include "../../lib/imgui/backends/imgui_impl_opengl3.h"

#include "../../lib/glad/glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define GLM_FORCE_XYZW_ONLY

#include "../../lib/glm/glm.hpp"
#include "../../lib/glm/ext/matrix_clip_space.hpp"
#include "../../lib/glm/ext/matrix_projection.hpp"
#include "../../lib/glm/ext/matrix_transform.hpp"
#include "../../lib/glm/gtc/type_ptr.hpp"

#include "../GAPI/OGL/GPUTimer.hpp"
#include "../GAPI/OGL/GLProgram.hpp"
#include "../GAPI/OGL/GLUtils.hpp"
#include "../Mesh.hpp"
#include "../GAPI/OGL/OGLMesh.hpp"
#include "Camera.hpp"

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
