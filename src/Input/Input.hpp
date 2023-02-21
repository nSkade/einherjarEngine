#pragma once

#include "BCodes.hpp"

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
	IBCodes::KeyboardKey kk; // GLFW_KEY_
	//TODO int32_t glfwScancode;
	IBCodes::InputAction ia; // GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT
	//TODO int32_t glfwMods;
};

struct MouseData {
	IBCodes::MouseButton mb; // GLFW_MOUSE_BUTTON_
	IBCodes::InputAction ia; // GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT
	//TODO int32_t glfwMod; //TODO wat dis
	double xpos;
	double ypos;
};

}//ehj
