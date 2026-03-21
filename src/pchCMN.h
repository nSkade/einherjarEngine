#pragma once
///////////
// libs
///////////
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define GLM_FORCE_XYZW_ONLY

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//#ifndef EHJ_DBG
//TODO remove
// imgui wont allow overwriting of new
//#if defined(__clang__) // ignore trival copyable warnings
//#pragma clang diagnostic push
//#pragma clang diagnostic ignored "-Wnontrivial-memaccess"
//#endif

#include <imgui.h>
#include <imgui_stdlib.h>

//TODO remove
//#if defined(__clang__)
//#pragma clang diagnostic pop
//#endif

#include <stb_image.h>

///////////
// std libs
///////////

#include <stdint.h>
#include <iostream>
#include <filesystem>
