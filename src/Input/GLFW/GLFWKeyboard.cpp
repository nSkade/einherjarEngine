#include "GLFWKeyboard.hpp"

namespace ehj {

std::weak_ptr<GLFWKeyboard> GLFWKeyboard::m_pInstance;

std::shared_ptr<GLFWKeyboard> GLFWKeyboard::instance() {
	auto i = m_pInstance.lock();
	if (!i) {
		std::cout << "GLFWKeyboard created!" << std::endl;
		i = std::make_shared<GLFWKeyboard>();
		m_pInstance = i;
	}
	return i;
}

void GLFWKeyboard::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//TODO
	auto i = instance();
	if (!i)
		return;
	KeyboardData d;
	d.kk = i->getKK(key);
	d.ia = i->getIA(action);
	i->broadcast(d);
}

GLFWKeyboard::~GLFWKeyboard() {
	m_pInstance.reset();
}

}//ehj
