#include "Input.hpp"

#include <iostream>

#include <glfw/glfw3.h>

namespace ehj {

class Camera {
public:
	Camera(Mouse* m, Keyboard* k) {
		m_mouseCB.setCallbackFunc(testMouse);
		m->addListener(&m_mouseCB);

		m_keyboardCB.setCallbackFunc(testKeyboard);
		k->addListener(&m_keyboardCB);

		std::cout << "Cam created\n";
	}

	static void testMouse(MouseData md) {
		std::cout << md.glfwMB << std::endl;
		if (md.glfwMB == -1)
			std::cout << md.xpos << " " << md.ypos << std::endl;
	}

	static void testKeyboard(KeyboardData md) {
		std::cout << md.glfwKey << std::endl;
	}

private:
	Callback<MouseData> m_mouseCB;
	Callback<KeyboardData> m_keyboardCB;
};

}//ehj
