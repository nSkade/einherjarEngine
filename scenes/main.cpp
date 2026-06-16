// AddressSanitizer options
//extern "C" const char* __asan_default_options() {
//	return "detect_leaks=1:log_path=stdout";
//}

// environment scene
//#include "helloWorld/EnvirScene.hpp"
//#include "EnvirTexScene/EnvirTexScene.hpp"
//#include "EnvirTexScene/ShadowMapTestScene.hpp"

// path tracing
//#include "PathTracing/spheres/fragTest/PTspheresFrag.hpp"
//#include "PathTracing/spheres/PTspheresComp.hpp"

//#include "PathTracing/rt test/EnvirRayTracing.hpp"
//#include "PathTracing/envir/EnvirPathTracing.hpp"
//#include "PathTracing/hybrid/EnvirPThybrid.hpp"
//#include "PathTracing/hybrid/IBLtestScene.hpp"
#include "PathTracing/hybrid/EnvirPThybridTex.hpp"

// rc
//#include "RC2D/naive/RCnaive.hpp"
//#include "RC2D/vanilla/RC.hpp"
//#include "voxelizer/VoxelTestScene.hpp"

// tools
//#include "plotGlsl2D/plotGlsl2D.hpp"

// exploratory
//#include "RayMarchingTestScene.hpp"
//#include "TessTestScene.hpp"
//#include "ADFtestScene.hpp"
//#include "FSRTestScene.hpp"
//#include "VulkanTest.hpp"

// tests
//#include "TriHelloWorldScene.hpp"
//#include "TexturedTri.hpp"
//#include "computeShaderExample/ComputeShaderTestScene.hpp"

#ifndef SCENETYPE
#error "SCENETYPE inside scene header undefined, "#define SCENETYPE <scene class name>" in your header"
#endif

int main(int argc, char** argv) {
#ifdef EHJ_DBG
	std::cout << "EHJ_DBG defined, running DEBUG mode.\n";
#endif
	{
	if (argv[0]) // set working directory to parent
		std::filesystem::current_path( // omit filename
			std::filesystem::absolute(argv[0]).parent_path().parent_path()
		);
	SCENETYPE scene;

	scene.setup();
	scene.run();
	scene.cleanup();
	}

	return 0;
};
