#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include "../src/suVULK.hpp"

#include <iostream>
#include <vector>

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#define SCENETYPE VulkanTestScene
class VulkanTestScene : IScene {
public:
	bool checkValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
		for (const char* layerName : validationLayers) {
			bool layerFound = false;
			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}
			if (!layerFound)
				return false;
		}
		return true;
	}
	void createInstance() {
		if (enableValidationLayers && !checkValidationLayerSupport())
			throw std::runtime_error("validation layers requested,but not available!");

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		} else
			createInfo.enabledLayerCount = 0;
		
		std::vector<const char*> extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();
		createInfo.enabledLayerCount = 0;
		VkResult result = vkCreateInstance(&createInfo, nullptr, &m_vkInstance);
	}

	std::vector<const char*> getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		if (enableValidationLayers)
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		return extensions;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {
		
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		if (messageSeverity >=
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			// Message is important enough to show
			std::cout << "VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT" << std::endl;
		}

		// collect message
		std::string msg = "";
		{
			uint32_t msgIdx = 0;
			while (pCallbackData->pMessage[msgIdx] != '\0') {
				msg.push_back(pCallbackData->pMessage[msgIdx]);
				msgIdx++;
			}
		}
		std::cout << msg << std::endl;

		return VK_FALSE;
	}

	void setup() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //TODO
		m_window = glfwCreateWindow(800, 600, "ehjE Vulkan window", nullptr, nullptr);
		ehjSetGLFWicon(m_window);

		//VkResult volkRes = volkInitialize();
		//if (volkRes!=VK_SUCCESS)
		//	std::cerr << "volk: error loading Vulkan!" << std::endl;

		//uint32_t version = volkGetInstanceVersion();
		//printf("Vulkan version %d.%d.%d initialized.\n",
		//		VK_VERSION_MAJOR(version),
		//		VK_VERSION_MINOR(version),
		//		VK_VERSION_PATCH(version));
		
		//this->createInstance();
		//volkLoadInstance(m_vkInstance);

		this->createInstance();

		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::cout << extensionCount << " extensions supported\n";
	}
	int run() {

		glm::mat4 matrix;
		glm::vec4 vec;
		auto test = matrix * vec;

		while(!glfwWindowShouldClose(m_window)) {
			glfwPollEvents();
		}

		return 0;
	}
	void cleanup() {
		vkDestroyInstance(m_vkInstance,nullptr);
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}
private:
	GLFWwindow* m_window;
	VkInstance m_vkInstance;
};

// Thanks to Alexander Overvoorde for the Vulkan Tutorial
