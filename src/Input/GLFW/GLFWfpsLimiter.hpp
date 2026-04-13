#pragma once
#include <Utility/FPSLimiter.hpp>

namespace ehj {

/**
 * @brief Automatically limits Framerate to Primary Monitor.
*/
class GLFWfpsLimiter : public FPSLimiter {
public:
	GLFWfpsLimiter() : FPSLimiter(getRefreshRate()) {};
	GLFWfpsLimiter(uint32_t fps) : FPSLimiter(fps) {};
	static uint32_t getRefreshRate() {
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		uint32_t refreshRate = glfwGetVideoMode(monitor)->refreshRate;
		return refreshRate;
	}
private:
};

}//ehj
