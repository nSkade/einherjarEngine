#pragma once
#include "../Input.hpp"

#include <GLFW/glfw3.h>

#include "GLFWCodes.hpp"

namespace ehj {

class GLFWKeyboard : public ICBcaster<KeyboardData>, public GLFWCodes {
public:
	~GLFWKeyboard();
	static std::shared_ptr<GLFWKeyboard> GLFWKeyboard::instance();
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
private:
	static std::weak_ptr<GLFWKeyboard> m_pInstance;
};

}//ehj
