#pragma once
#include "../BCodes.hpp"

#include <GLFW/glfw3.h>

namespace ehj {

class GLFWCodes : public IBCodes {
public:
	InputAction getIA(int32_t code);
	KeyboardKey getKK(int32_t code);
	MouseButton getMB(int32_t code);
private:
};

}//ehj
