
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
	
	class GLFWKeyboardCBl : public ICBlistener<KeyboardData> {
	public:
		GLFWKeyboardCBl() {};
		GLFWKeyboardCBl(GLFWCallbackTest* pCbt) { m_pCbt = pCbt; };
		void callback(KeyboardData kbd) {
			if (m_pCbt)
				m_pCbt->testGLFWKeyboard(kbd);
		};
	private:
		GLFWCallbackTest* m_pCbt = nullptr;
	};

	GLFWCallbackTest() {
		std::shared_ptr<GLFWMouse> m = GLFWMouse::instance();
		std::shared_ptr<GLFWKeyboard> k = GLFWKeyboard::instance();

		if (!m || !k)
			return;

		m_mouseCBl = MouseCBl(this);
		m->addListener(&m_mouseCBl);

		m_GLFWKeyboardCBl = GLFWKeyboardCBl(this);
		k->addListener(&m_GLFWKeyboardCBl);
	}
	~GLFWCallbackTest() {
		std::shared_ptr<GLFWMouse> m = GLFWMouse::instance();
		if (m)
			m->removeListener(&m_mouseCBl);
		std::shared_ptr<GLFWKeyboard> k = GLFWKeyboard::instance();
		if (k)
			k->removeListener(&m_GLFWKeyboardCBl);
	}

	void testMouse(MouseData md) {
		if (md.mb == IBCodes::MB_BUTTON_NONE)
			std::cout << md.xpos << " " << md.ypos << std::endl;
		else
			std::cout << md.mb << " release: " << (md.ia==IBCodes::IA_RELEASE?true:false) << std::endl;
	}

	void testGLFWKeyboard(KeyboardData kd) {
		if (kd.ia != IBCodes::IA_REPEAT)
			std::cout << kd.kk << " release: " << (kd.ia==IBCodes::IA_RELEASE?true:false) << std::endl;
	}
private:
	MouseCBl m_mouseCBl;
	GLFWKeyboardCBl m_GLFWKeyboardCBl;
};

}//ehj
