#pragma once
#include <vector>

//TODO abstract from GLFW
#include <GLFW/glfw3.h>

namespace ehj {

template <typename T>
class Callback {
public:
	void addListener(Callback* c);
	void removeListener(Callback* c);
	void broadcast(T t);
	void setCallbackFunc(void (*callbackFunc)(T t));
	void call(T t);
private:
	void (*m_callbackFunc)(T t);
	std::vector<Callback*> m_listeners;
};

template <typename T>
void Callback<T>::addListener(Callback<T>* c) {
	m_listeners.push_back(c);
}

template <typename T>
void Callback<T>::removeListener(Callback<T>* c) {
	for (uint32_t i=0;i<m_listeners.size();++i) {
		if (c==m_listeners[i]) {
			m_listeners.erase(m_listeners.begin()+i);
		}
	}
}

template <typename T>
void Callback<T>::setCallbackFunc(void (*callbackFunc)(T t)) {
	m_callbackFunc =  callbackFunc;
}

template <typename T>
void Callback<T>::broadcast(T t) {
	for (uint32_t i=0;i<m_listeners.size();++i)
		m_listeners[i]->call(t);
}

template <typename T>
void Callback<T>::call(T t) {
	m_callbackFunc(t);
}

struct KeyboardData {
	int32_t glfwKey; // GLFW_KEY_
};

class Keyboard : public Callback<KeyboardData> {
public:
	static Keyboard* instance();
	static void release();

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
private:
	static Keyboard* m_pInstance;
};

struct MouseData {
	int32_t glfwMB; // GLFW_MOUSE_BUTTON_
	double xpos;
	double ypos;
};

//TODO test multiple classes
class Mouse : public Callback<MouseData> {
public:
	static Mouse* instance();
	static void release();
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
private:
	static Mouse* m_pInstance;
};

}//ehj
