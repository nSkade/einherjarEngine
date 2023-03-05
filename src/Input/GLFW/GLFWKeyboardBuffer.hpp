#include "../Input.hpp"
#include "../BCodes.hpp"
#include "GLFWKeyboard.hpp"

namespace ehj {

/**
 * @brief Stores states of buttons for simple query
*/
class GLFWKeyboardCache : public ICBlistener<KeyboardData> {
public:
	//TODO make protected?
	void callback(KeyboardData kbd) {
		if (kbd.ia == IBCodes::IA_PRESS)
			m_KK[kbd.kk] = true;
		else
			m_KK[kbd.kk] = false;
	};
	GLFWKeyboardCache(const GLFWKeyboardCache&) = delete;
	GLFWKeyboardCache& operator=(const GLFWKeyboardCache &) = delete;
	GLFWKeyboardCache(GLFWKeyboardCache &&) = delete;
	GLFWKeyboardCache & operator=(GLFWKeyboardCache &&) = delete;
	static bool keyPressed(IBCodes::KeyboardKey kk) { return instance()->m_KK[kk]; };
private:
	GLFWKeyboardCache() {
		for (uint32_t i=0;i<IBCodes::KK_COUNT;++i)
			m_KK[i] = false;
		GLFWKeyboard::instance()->addListener(this);
	}
	static GLFWKeyboardCache* instance() {
		static GLFWKeyboardCache kb;
		return &kb;
	}

	bool m_KK[IBCodes::KK_COUNT];
};

}//ehj
