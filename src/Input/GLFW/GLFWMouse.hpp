#pragma once
#include <GLFW/glfw3.h>
#include "../Input.hpp"
#include "GLFWCodes.hpp"

namespace ehj {

//TODO combine GLFWCodes of GLFWMouse and keyboard
class GLFWMouse : public ICBcaster<MouseData>, public GLFWCodes {
public:
	~GLFWMouse();
	//static void release();
	static std::shared_ptr<GLFWMouse> instance();
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
private:
	static std::weak_ptr<GLFWMouse> m_pInstance;
};

}//ehj
