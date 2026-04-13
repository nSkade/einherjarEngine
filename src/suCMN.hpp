#pragma once
/*
 * @brief Scene Utilities Common
 */

#include "Mesh.hpp"
#include "Model.hpp"
#include "Camera.hpp"
#include "Utility/Clock.hpp"  //TODOf replace with Timer.hpp?
#include "Utility/Timer.hpp"
#include "Input/GLFW/GLFWfpsLimiter.hpp"

////////////
// structure
////////////

/**
 * @brief Scene utility for OpenGL.
*/
class IScene {
public:
	virtual void setup() = 0;
	virtual int run() = 0;
	virtual void cleanup() = 0;
};

////////
// tools
////////

#define EHJ_THIS_FOLDER() ehj_get_path_relative_to_root(__FILE__)

inline std::string ehj_get_path_relative_to_root(const char* file_path) {
	std::filesystem::path caller_file(file_path);
	std::filesystem::path root(EHJ_ROOT_PATH);
	return std::filesystem::relative(caller_file.parent_path(), root).string()+"/";
}

////////
// extas
////////

void ehjSetGLFWicon(GLFWwindow* window);
