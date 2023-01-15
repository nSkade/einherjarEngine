#include "scenes/sampleScene.hpp"

int main(void)
{
	std::cout << "hello world\n";
	SampleScene scene = SampleScene();
	scene.setup();
	scene.run();
	return 0;
};
