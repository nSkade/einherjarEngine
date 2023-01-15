#include "scenes/sampleScene.hpp"
#include "scenes/triangleHW.hpp"

int main(void)
{
	SampleScene scene = SampleScene();
	//TriangleHW scene = TriangleHW();
	scene.setup();
	scene.run();
	return 0;
};
