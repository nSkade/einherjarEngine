#include "scenes/SampleScene.hpp"
#include "scenes/TriHelloWorldScene.hpp"
#include "scenes/ComputeShaderTestScene.hpp"
#include "scenes/VulkanTest.hpp"
#include "scenes/ADFtestScene.hpp"
#include "scenes/EnvirScene.hpp"

int main(void)
{
	//TriangleHWScene scene;
	//SampleScene scene;
	//ComputeShaderTestScene scene;
	//VulkanTestScene scene;
	//ADFtestScene scene;
	EnvirScene scene;

	scene.setup();
	scene.run();
	scene.cleanup();
	return 0;
};
