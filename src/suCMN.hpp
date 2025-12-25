// Scene Utilities Common
#pragma once

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

// https://gist.github.com/dougbinks/8089b4bbaccaaf6fa204236978d165a9#file-imguiutils-h-L9-L93
inline void SetupImGuiStyle(bool is_dark_style, float alpha_threshold) {
	//Use a ternary operator
	is_dark_style ? ImGui::StyleColorsDark() : ImGui::StyleColorsLight();

	ImGuiStyle& style = ImGui::GetStyle();

	// Adjusts the alpha values of the ImGui colors based on the alpha threshold.
	for (int i = 0; i < ImGuiCol_COUNT; i++) {
		const auto color_id = static_cast<ImGuiCol>(i);
		auto& color = style.Colors[i];
		if (color.w < alpha_threshold || color_id == ImGuiCol_FrameBg || color_id == ImGuiCol_WindowBg || color_id == ImGuiCol_ChildBg)
			color.w *= alpha_threshold;
	}

	// Sets the border sizes and rounding.
	style.ChildBorderSize = 1.0f;
	style.FrameBorderSize = 0.0f;
	style.PopupBorderSize = 1.0f;
	style.WindowBorderSize = 0.0f;
	style.FrameRounding = 3.0f;
	style.Alpha = 1.0f;
}
//#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "Utility/GLFWfpsLimiter.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define GLM_FORCE_XYZW_ONLY

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Mesh.hpp"
#include "Model.hpp"
#include "Camera.hpp"
#include "Utility/Clock.hpp"  //TODOf replace with Timer.hpp?
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

void ehjSetGLFWicon(GLFWwindow* window) {
	GLFWimage images[1];
	images[0].pixels = stbi_load("lib/ehjEicon/icon.png", &images[0].width, &images[0].height, 0, 4); //rgba channels
	glfwSetWindowIcon(window, 1, images); 
	stbi_image_free(images[0].pixels);
}
