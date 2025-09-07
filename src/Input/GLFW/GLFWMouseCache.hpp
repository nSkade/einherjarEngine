#pragma once
#include "../Input.hpp"
#include "../BCodes.hpp"
#include "GLFWMouse.hpp"

namespace ehj {

/**
 * @brief Stores states of buttons for simple query
*/
class GLFWMouseCache : public ICBlistener<MouseData> {
public:
	//TODO make protected?
	void callback(MouseData md) {
		if (md.mb == IBCodes::MB_BUTTON_NONE) {
			m_xpos = md.xpos;
			m_ypos = md.ypos;
		}
		m_MB[md.mb] = false;
		m_MBr[md.mb] = false;
		if (md.ia == IBCodes::IA_PRESS)
			m_MB[md.mb] = true;
		if (md.ia == IBCodes::IA_RELEASE)
			m_MBr[md.mb] = true;
	};
	GLFWMouseCache(const GLFWMouseCache&) = delete;
	GLFWMouseCache& operator=(const GLFWMouseCache &) = delete;
	GLFWMouseCache(GLFWMouseCache &&) = delete;
	GLFWMouseCache & operator=(GLFWMouseCache &&) = delete;
	static double getXPos() { return instance()->m_xpos; };
	static double getYPos() { return instance()->m_ypos; };
	static bool keyPressed(IBCodes::MouseButton mb) { return instance()->m_MB[mb]; };
	static bool keyReleased(IBCodes::MouseButton mb) { return instance()->m_MBr[mb]; };
private:
	GLFWMouseCache() {
		for (uint32_t i=0;i<IBCodes::KK_COUNT;++i)
			m_MB[i] = false;
		GLFWMouse::instance()->addListener(this);
	}
	static GLFWMouseCache* instance() {
		static GLFWMouseCache kb;
		return &kb;
	}

	double m_xpos=0.,m_ypos=0.;
	bool m_MB[IBCodes::KK_COUNT];
	bool m_MBr[IBCodes::KK_COUNT];
};

}//ehj
