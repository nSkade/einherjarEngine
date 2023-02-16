#include "Input.hpp"
#include <iostream>

namespace ehj {

Keyboard* Keyboard::m_pInstance = nullptr;

Keyboard* Keyboard::instance() {
	if (!m_pInstance)
		m_pInstance = new Keyboard;
	return m_pInstance;
}

void Keyboard::release() {
	delete m_pInstance;
}

void Keyboard::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (!m_pInstance)
		return;
	KeyboardData d;
	d.glfwKey = key;
	m_pInstance->broadcast(d);
}

Mouse* Mouse::m_pInstance = nullptr;

Mouse* Mouse::instance() {
	if (!m_pInstance)
		m_pInstance = new Mouse;
	return m_pInstance;
}

void Mouse::release() {
	delete m_pInstance;
}

void Mouse::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (!m_pInstance)
		return;

	MouseData d;
	d.glfwMB = button;
	m_pInstance->broadcast(d);
}
void Mouse::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (!m_pInstance)
		return;
	MouseData d;
	d.glfwMB = -1;
	d.xpos=xpos;
	d.ypos=ypos;
	m_pInstance->broadcast(d);
}

}//ehj
