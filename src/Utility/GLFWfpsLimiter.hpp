#include "FPSLimiter.hpp"
#include <glfw/glfw3.h>

namespace ehj {

/**
 * @brief Automatically limits Framerate to Primary Monitor.
*/
class GLFWfpsLimiter : public FPSLimiter {
public:
	GLFWfpsLimiter() : FPSLimiter(getRefreshRate()) {};
private:
	static uint32_t getRefreshRate() {
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		uint32_t refreshRate = glfwGetVideoMode(monitor)->refreshRate;
		return refreshRate;
	}
};

}//ehj
