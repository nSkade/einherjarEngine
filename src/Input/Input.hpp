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
protected:
	void broadcast(T t);
	std::vector<ICBlistener<T>*> m_listeners;
};

template <typename T>
void ICBcaster<T>::addListener(ICBlistener<T>* c) {
	for (uint32_t i=0;i<m_listeners.size();++i) //TODO faster?
		if (m_listeners[i]==c) return;
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
	for (uint32_t i=0;i<m_listeners.size();++i) {
		if (m_listeners[i])
			m_listeners[i]->callback(t);
	}
}

struct KeyboardData {
	IBCodes::KeyboardKey kk;
	//TODO int32_t glfwScancode; //TODO
	IBCodes::InputAction ia;
	//TODO int32_t glfwMods; //TODO
};

struct MouseData {
	IBCodes::MouseButton mb;
	IBCodes::InputAction ia;
	//TODO int32_t glfwMod; //TODO wat dis
	//TODO change double to glm::ivec2
	double xpos;
	double ypos;
};

}//ehj
