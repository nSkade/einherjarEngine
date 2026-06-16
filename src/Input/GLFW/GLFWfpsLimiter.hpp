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
		// add 2 digit prime to avoid drifting into blanc interval which results in half fps on windowed
		uint32_t refreshRate = glfwGetVideoMode(monitor)->refreshRate + 13;
		return refreshRate;
	}
private:
};

}//ehj
