
#include "GLFWMouse.hpp"

namespace ehj {

std::weak_ptr<GLFWMouse> GLFWMouse::m_pInstance;

std::shared_ptr<GLFWMouse> GLFWMouse::instance() {
	auto i = m_pInstance.lock();
	if (!i) {
		i = std::make_shared<GLFWMouse>();
		m_pInstance = i;
	}
	return i;
}

void GLFWMouse::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	auto i = instance();
	if (!i)
		return;

	MouseData d;
	d.mb = i->getMB(button);
	d.ia = i->getIA(action);
	i->broadcast(d);
}
void GLFWMouse::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	auto i = instance();
	if (!i)
		return;
	MouseData d;
	d.mb = MB_BUTTON_NONE;
	d.xpos=xpos;
	d.ypos=ypos;
	i->broadcast(d);
}

GLFWMouse::~GLFWMouse() {
#ifdef EHJ_DBG
	std::cout << "GLFWMouse released!" << std::endl;
#endif
	m_pInstance.reset();
}

}//ehj
