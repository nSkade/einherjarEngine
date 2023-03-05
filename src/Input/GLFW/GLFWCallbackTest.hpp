#pragma once
#include "GLFWKeyboard.hpp"
#include "GLFWMouse.hpp"

namespace ehj {

class GLFWCallbackTest {
public:

	class MouseCBl : public ICBlistener<MouseData> {
	public:
		MouseCBl() {};
		MouseCBl(GLFWCallbackTest* pCbt) { m_pCbt = pCbt; };
		void callback(MouseData md) {
			if (m_pCbt)
				m_pCbt->testMouse(md);
		};
	private:
		GLFWCallbackTest* m_pCbt = nullptr;
	};
	
	class KeyboardCBl : public ICBlistener<KeyboardData> {
	public:
		KeyboardCBl() {};
		KeyboardCBl(GLFWCallbackTest* pCbt) { m_pCbt = pCbt; };
		void callback(KeyboardData kbd) {
			if (m_pCbt)
				m_pCbt->testKeyboard(kbd);
		};
	private:
		GLFWCallbackTest* m_pCbt = nullptr;
	};

	GLFWCallbackTest() {
		m_mouseCBl = MouseCBl(this);
		GLFWMouse::instance()->addListener(&m_mouseCBl);

		m_GLFWKeyboardCBl = KeyboardCBl(this);
		GLFWKeyboard::instance()->addListener(&m_GLFWKeyboardCBl);
	}
	~GLFWCallbackTest() {
		GLFWMouse::instance()->removeListener(&m_mouseCBl);
		GLFWKeyboard::instance()->removeListener(&m_GLFWKeyboardCBl);
	}

	void testMouse(MouseData md) {
		if (md.mb == IBCodes::MB_BUTTON_NONE)
			std::cout << md.xpos << " " << md.ypos << std::endl;
		else
			std::cout << md.mb << " release: " << (md.ia==IBCodes::IA_RELEASE?true:false) << std::endl;
	}

	void testKeyboard(KeyboardData kd) {
		if (kd.ia != IBCodes::IA_REPEAT)
			std::cout << kd.kk << " release: " << (kd.ia==IBCodes::IA_RELEASE?true:false) << std::endl;
	}
private:
	MouseCBl m_mouseCBl;
	KeyboardCBl m_GLFWKeyboardCBl;
};

}//ehj
