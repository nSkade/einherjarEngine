#pragma once
/*
 * @brief Scene Utilities Common
 */

#include "Mesh.hpp"
#include "Model.hpp"
#include "Camera.hpp"
#include "Utility/Clock.hpp"  //TODOf replace with Timer.hpp?
#include "Utility/Timer.hpp"
#include "Utility/GLFWfpsLimiter.hpp"

////////////
// structure
////////////

/**
 * @brief Scene utility for OpenGL.
*/
class IScene {
public:
	virtual void setup() = 0;
	virtual int run() = 0;
	virtual void cleanup() = 0;
};

////////
// tools
////////

#define EHJ_THIS_FOLDER() ehj_get_path_relative_to_root(__FILE__)

inline std::string ehj_get_path_relative_to_root(const char* file_path) {
	std::filesystem::path caller_file(file_path);
	std::filesystem::path root(EHJ_ROOT_PATH);
	return std::filesystem::relative(caller_file.parent_path(), root).string()+"/";
}

////////
// extas
////////

void ehjSetGLFWicon(GLFWwindow* window) {
	GLFWimage images[1];
	images[0].pixels = stbi_load("lib/ehjEicon/icon.png", &images[0].width, &images[0].height, 0, 4); //rgba channels
	glfwSetWindowIcon(window, 1, images); 
	stbi_image_free(images[0].pixels);
}

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
