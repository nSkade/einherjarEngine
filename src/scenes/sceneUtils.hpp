
#include "../../lib/imgui/imgui.h"
#include "../../lib/imgui/backends/imgui_impl_glfw.h"
#include "../../lib/imgui/backends/imgui_impl_opengl3.h"

#include "../../lib/glad/glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "../../lib/glm/glm.hpp"
#include "../../lib/glm/ext/matrix_clip_space.hpp"
#include "../../lib/glm/ext/matrix_projection.hpp"
#include "../../lib/glm/ext/matrix_transform.hpp"
#include "../../lib/glm/gtc/type_ptr.hpp"

#include "../GPUTimer.hpp"
#include "../GLProgram.hpp"
#include "../Mesh.hpp"

class IScene {
public:
	virtual void setup() = 0;
	virtual int run() = 0;
};
