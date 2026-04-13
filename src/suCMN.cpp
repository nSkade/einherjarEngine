#include "suCMN.hpp"

void ehjSetGLFWicon(GLFWwindow* window) {
	GLFWimage images[1];
	images[0].pixels = stbi_load("lib/ehjEicon/icon.png", &images[0].width, &images[0].height, 0, 4); //rgba channels
	glfwSetWindowIcon(window, 1, images); 
	stbi_image_free(images[0].pixels);
}
