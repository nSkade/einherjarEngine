#ifdef EHJ_DBG
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
//	#define new new(_CLIENT_BLOCK,__FILE__,__LINE__)
#endif

#include "../scenes/SampleScene.hpp"
#include "../scenes/TriHelloWorldScene.hpp"
#include "../scenes/ComputeShaderTestScene.hpp"
#include "../scenes/VulkanTest.hpp"
#include "../scenes/ADFtestScene.hpp"
#include "../scenes/EnvirScene.hpp"
#include "../scenes/RayMarchingTestScene.hpp"
#include "../scenes/FSRTestScene.hpp"

int main(void) {
#ifdef EHJ_DBG
	std::cout << "EHJ_DBG defined, running DEBUG mode.\n";
#endif
	{
	
	//TriangleHWScene scene;
	//SampleScene scene;
	//ComputeShaderTestScene scene;
	//VulkanTestScene scene;
	//ADFtestScene scene;
	//EnvirScene scene;
	//RayMarchingTestScene scene;
	FSRTestScene scene;

	scene.setup();
	scene.run();
	scene.cleanup();

	}
#ifdef EHJ_DBG
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG); 
	_CrtDumpMemoryLeaks();
#endif

	return 0;
};

