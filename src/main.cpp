#include "scenes/SampleScene.hpp"
#include "scenes/TriHelloWorldScene.hpp"
#include "scenes/ComputeShaderTestScene.hpp"
#include "scenes/vulkanTest.hpp"
#include "scenes/ADFtestScene.hpp"

int main(void)
{
	//TriangleHWScene scene;
	//SampleScene scene;
	//ComputeShaderTestScene scene;
	//VulkanTestScene scene;
	ADFtestScene scene;

	scene.setup();
	scene.run();
	scene.cleanup();
	return 0;
};
