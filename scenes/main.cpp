#ifdef EHJ_DBG
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
//	#define new new(_CLIENT_BLOCK,__FILE__,__LINE__)
#endif

// tests
//#include "TriHelloWorldScene.hpp"
//#include "TessTestScene.hpp"
//#include "ComputeShaderTestScene.hpp"
//#include "ADFtestScene.hpp"
//#include "EnvirScene.hpp"
//#include "RayMarchingTestScene.hpp"
//#include "FSRTestScene.hpp"
//#include "VulkanTest.hpp"

//#include "TexturedTri.hpp"

// lighting
//#include "PathTracing.hpp"
//#include "RC2D/RCnaive.hpp"
#include "RC2D/RC.hpp"

// tools
//#include "plotGlsl2D/plotGlsl2D.hpp"

#ifndef SCENETYPE
#error "SCENETYPE inside scene header undefined, "#define SCENETYPE <scene class name>" in your header"
#endif

int main(int argc, char** argv) {
#ifdef EHJ_DBG
	std::cout << "EHJ_DBG defined, running DEBUG mode.\n";
#endif
	{
	SCENETYPE scene;

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
