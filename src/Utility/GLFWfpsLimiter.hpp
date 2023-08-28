#include "FPSLimiter.hpp"
#include <glfw/glfw3.h>

namespace ehj {

//TODOf rework combine with FPSLimiter + abstract getting refresh Rate
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
