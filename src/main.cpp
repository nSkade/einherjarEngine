#include "scenes/sampleScene.hpp"
#include "scenes/triangleHW.hpp"
#include "scenes/computeShaderTest.hpp"
#include "scenes/vulkanTest.hpp"

int main(void)
{
	TriangleHWScene scene;
	//SampleScene scene;
	//ComputeShaderTestScene scene;
	//VulkanTestScene scene;

	scene.setup();
	scene.run();
	return 0;
};
