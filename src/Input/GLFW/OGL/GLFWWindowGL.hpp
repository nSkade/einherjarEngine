#pragma once

#include "../GLFWKeyboard.hpp"
#include "../GLFWMouse.hpp"

#include <Utility/ConfigFile.hpp>

#include <glad/glad.h>
#include <GAPI/OGL/GLUtils.hpp>

namespace ehj {

struct GLFWWindowGL {
	GLFWwindow* m_pWindow;
	glm::ivec2 m_winRes;
	glm::ivec2 m_winPos;
	std::shared_ptr<ehj::GLFWKeyboard> m_kb;
	std::shared_ptr<ehj::GLFWMouse> m_mouse;
	
	static void error_callback(int error, const char* description)
	{
		fprintf(stderr, "Error: %s\n", description);
	}

	void setup(ConfigFile* configFile, std::string windowTitle, void(*framebuffer_size_callback)(GLFWwindow*, int, int)) {
		if (!glfwInit())
			exit(EXIT_FAILURE);
		
		glfwSetErrorCallback(error_callback);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

		m_winRes.x = 640;
		m_winRes.y = 480;
	#if FULLSCREEN
		m_winRes.x = 1920;
		m_winRes.y = 1080;
	#endif
		configFile->baseLoad();
		configFile->load("winRes",&m_winRes);
		configFile->load("winPos",&m_winPos);
		
		//m_winMaximized=false;
		//m_configFile.load("winMaximized",&m_winMaximized);
		//if (m_winMaximized)
		//	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

		m_pWindow = glfwCreateWindow(m_winRes.x,m_winRes.y, windowTitle.c_str(), NULL, NULL);
		glfwSetWindowPos(m_pWindow,m_winPos.x,m_winPos.y);
		ehjSetGLFWicon(m_pWindow);

		if (!m_pWindow)
		{
			glfwTerminate();
			exit(EXIT_FAILURE);
		}
	#if FULLSCREEN
		glfwSetWindowPos(m_pWindow, 0,0);
	#endif
		glfwSetFramebufferSizeCallback(m_pWindow, framebuffer_size_callback);

		m_kb = ehj::GLFWKeyboard::instance();
		m_mouse = ehj::GLFWMouse::instance();

		glfwSetCursorPosCallback(m_pWindow, m_mouse->mouse_callback);
		glfwSetMouseButtonCallback(m_pWindow, m_mouse->mouse_button_callback);
		glfwSetKeyCallback(m_pWindow, m_kb->key_callback);

		glfwMakeContextCurrent(m_pWindow);
		gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
		ehj_gl_err_callback();
		glViewport(0,0, m_winRes.x, m_winRes.y);
		//TODO glfwSwapInterval(1);
	}
	
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
	}
	void setup(ConfigFile* configFile, std::string windowTitle) { setup(configFile,windowTitle,framebuffer_size_callback); };

	bool stillOpen() { return !glfwWindowShouldClose(m_pWindow); }
	void swapBuffers() { glfwSwapBuffers(m_pWindow); }

	void pollInput(void(*processInput)(GLFWwindow*)) {
		glfwPollEvents();
		processInput(m_pWindow);
	}

	static void processInput(GLFWwindow *window) {
		if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
	}
	
	void pollInput() {
		pollInput(processInput);
	}
	void close(ConfigFile* configFile=nullptr) {
		glfwGetWindowPos(m_pWindow,&m_winPos.x,&m_winPos.y);
		//m_winMaximized =(bool) glfwGetWindowAttrib(m_pWindow, GLFW_MAXIMIZED); //TODO

		if (configFile) {
			configFile->store("winRes",m_winRes);
			configFile->store("winPos",m_winPos);
			//configFile->store("winMaximized",m_winMaximized); //TODO
		}
		glfwDestroyWindow(m_pWindow);
		glfwTerminate();
	}
};

}//ehj
