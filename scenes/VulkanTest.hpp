//#include "../suOGL.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
//#include <vulkan/vulkan.hpp>
#include <Volk/volk.h>
//#include <Volk/volk.h> //TODO use volk

#include <iostream>

class VulkanTestScene : IScene {
public:
	void setup() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
		
		VkResult volkRes = volkInitialize();
		if (volkRes!=VK_SUCCESS)
			std::cerr << "volk: error loading Vulkan!" << std::endl;

		VkInstance vkInstance;
		volkLoadInstance(vkInstance);

		uint32_t extensionCount = 0;
		//vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::cout << extensionCount << " extensions supported\n";
	}
	int run() {

		glm::mat4 matrix;
		glm::vec4 vec;
		auto test = matrix * vec;

		while(!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}

		return 0;
	}
	void cleanup() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}
private:
	GLFWwindow* window;
};
