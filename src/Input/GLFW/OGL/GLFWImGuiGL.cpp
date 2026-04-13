#include "GLFWImGuiGL.hpp"

#include <Input/ImGuiStyle.hpp>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace ehj {

namespace GLFWImGuiGL {

	void init(GLFWwindow* window) {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		SetupImGuiStyle(true);
		ImGui_ImplGlfw_InitForOpenGL(window,true);
		ImGui_ImplOpenGL3_Init("#version 460");
	}

	void newFrame() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void render() {
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void shutdown() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
}//GLFWWindowGl

}//ehj