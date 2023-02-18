#pragma once

//TODO abstract from GLFW
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

namespace ehj {

template <typename T>
class ICBlistener {
public:
	virtual void callback(T t) = 0;
};

template <typename T>
class ICBcaster {
public:
	void addListener(ICBlistener<T>* c);
	void removeListener(ICBlistener<T>* c);
	void broadcast(T t);
private:
	std::vector<ICBlistener<T>*> m_listeners;
};

template <typename T>
void ICBcaster<T>::addListener(ICBlistener<T>* c) {
	m_listeners.push_back(c);
}

template <typename T>
void ICBcaster<T>::removeListener(ICBlistener<T>* c) {
	for (uint32_t i=0;i<m_listeners.size();++i) {
		if (c==m_listeners[i]) {
			m_listeners.erase(m_listeners.begin()+i);
		}
	}
}

template <typename T>
void ICBcaster<T>::broadcast(T t) {
	for (uint32_t i=0;i<m_listeners.size();++i)
		m_listeners[i]->callback(t);
}

struct KeyboardData {
	int32_t glfwKey; // GLFW_KEY_
	int32_t glfwScancode;
	int32_t glfwAction; // GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT
	int32_t glfwMods;
};

class Keyboard : public ICBcaster<KeyboardData> {
public:
	static Keyboard* instance();
	static void release();
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
private:
	static Keyboard* m_pInstance;
};

struct MouseData {
	int32_t glfwButton; // GLFW_MOUSE_BUTTON_
	int32_t glfwAction; // GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT
	int32_t glfwMod; //TODO wat dis
	double xpos;
	double ypos;
};

//TODO abstract from glfw
class Mouse : public ICBcaster<MouseData> {
public:
	static Mouse* instance();
	static void release();
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
private:
	static Mouse* m_pInstance;
};

class CallbackTest {
public:

	class MouseCBl : public ICBlistener<MouseData> {
	public:
		MouseCBl() {};
		MouseCBl(CallbackTest* pCbt) { m_pCbt = pCbt; };
		void callback(MouseData md) {
			if (m_pCbt)
				m_pCbt->testMouse(md);
		};
	private:
		CallbackTest* m_pCbt = nullptr;
	};
	
	class KeyboardCBl : public ICBlistener<KeyboardData> {
	public:
		KeyboardCBl() {};
		KeyboardCBl(CallbackTest* pCbt) { m_pCbt = pCbt; };
		void callback(KeyboardData kbd) {
			if (m_pCbt)
				m_pCbt->testKeyboard(kbd);
		};
	private:
		CallbackTest* m_pCbt = nullptr;
	};

	CallbackTest() {
		Mouse* m = Mouse::instance();
		Keyboard* k = Keyboard::instance();

		m_mouseCBl = MouseCBl(this);
		m->addListener(&m_mouseCBl);

		m_keyboardCBl = KeyboardCBl(this);
		k->addListener(&m_keyboardCBl);
	}
	~CallbackTest() {
		Mouse::instance()->removeListener(&m_mouseCBl);
		Keyboard::instance()->removeListener(&m_keyboardCBl);
	}

	void testMouse(MouseData md) {
		std::cout << md.glfwButton << " release: " << (md.glfwAction==GLFW_RELEASE?true:false) << std::endl;
		if (md.glfwButton == -1)
			std::cout << md.xpos << " " << md.ypos << std::endl;
	}

	void testKeyboard(KeyboardData kd) {
		if (kd.glfwAction != GLFW_REPEAT)
			std::cout << kd.glfwKey << (kd.glfwAction==GLFW_RELEASE?true:false) << std::endl;
	}
private:
	MouseCBl m_mouseCBl;
	KeyboardCBl m_keyboardCBl;
};

}//ehj
