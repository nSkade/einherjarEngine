#include "../glad/glad.h"
#define GLFW_INCLUDE_NONE
#include "../glfw3/glfw3.h"
#include "../glm/matrix.hpp"
#include "../glm/ext/matrix_clip_space.hpp"
#include "../glm/ext/matrix_projection.hpp"
#include "../glm/ext/matrix_transform.hpp"

#include "linmath.h"
 
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <chrono>
#include <fstream>
#include <sstream>

#include "GPUTimer.hpp"

// enable optimus!
extern "C" {
	_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
	_declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

static const struct
{
	float x, y;
	float u, v;
} vertices[6] =
{
	{ -1.0f, -1.0f, 0.f, 0.f },
	{  1.0f,  1.0f, 1.f, 1.f },
	{ -1.0f,  1.0f, 0.f, 1.f },
	{ -1.0f, -1.0f, 0.f, 0.f },
	{  1.0f, -1.0f, 1.f, 0.f },
	{  1.0f,  1.0f, 1.f, 1.f }
};

uint32_t ehj_gl_err() {
	unsigned int err = 0;
	err = glGetError();
	while ((err != 0)) {
		std::cout << err << "\n";
		err = glGetError();
		return 1;
	}
	return 0;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}
 
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

#define FULLSCREEN false

int main(void)
{
	glm::vec3 vec = glm::vec3(0.0f,1.0f,0.0f);
	
	GLFWwindow* window;
	GLuint vertex_buffer, vertex_shader, fragment_shader, program;
	GLint mvp_location, vpos_location, vcol_location;
	
	glfwSetErrorCallback(error_callback);
	
	std::chrono::steady_clock::time_point chrStartTime = std::chrono::steady_clock::now();
	
	if (!glfwInit())
		exit(EXIT_FAILURE);
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	
	#if FULLSCREEN
	window = glfwCreateWindow(1920, 1080, "Simple example", NULL, NULL);
	#else
	window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
	#endif
	
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	#if FULLSCREEN
	glfwSetWindowPos(window, 0,0);
	#endif
	
	glfwSetKeyCallback(window, key_callback);
	
	glfwMakeContextCurrent(window);
	//gladLoadGL(glfwGetProcAddress);
	//gladLoadGL();
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval(0);
	
	// NOTE: OpenGL error checks have been omitted for brevity
	
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	
	std::stringstream buffer;
	std::ifstream t("assets/basic_v.vert");
	buffer << t.rdbuf();
	std::string vertex_shader_text = buffer.str();
	
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	const char* vertex_shader_c = vertex_shader_text.c_str();
	std::cout << vertex_shader_c;
	//return 0;
	glShaderSource(vertex_shader, 1, &vertex_shader_c, NULL);
	glCompileShader(vertex_shader);

	t.clear();
	t.close();
	t = std::ifstream("assets/tellu.frag");
	buffer.seekp(0);
	buffer.clear();
	buffer << t.rdbuf();
	
	std::string fileShaderS = buffer.str();
	const char* fileShader = fileShaderS.c_str();
	std::cout << fileShader << "\n";
	
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fileShader, NULL);
	glCompileShader(fragment_shader);

	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);
	
	
	if (ehj_gl_err())
		return -1;
	mvp_location = glGetUniformLocation(program, "MVP");
	vpos_location = glGetAttribLocation(program, "vPos");
	vcol_location = glGetAttribLocation(program, "vCol");
	GLint utime_location = glGetUniformLocation(program, "u_time");
	GLint ures_location = glGetUniformLocation(program, "u_resolution");
	
	std::cout << mvp_location << vpos_location << vcol_location << utime_location << ures_location;
	glEnableVertexAttribArray(vpos_location);
	glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
						sizeof(vertices[0]), (void*) 0);
	//glEnableVertexAttribArray(vcol_location);
	//glVertexAttribPointer(vcol_location, 2, GL_FLOAT, GL_FALSE,
	//					sizeof(vertices[0]), (void*) (sizeof(float) * 2));
	
	GPUTimer gpuTimer;
	
	while (!glfwWindowShouldClose(window))
	{
		float ratio;
		int width, height;
		//mat4x4 m, p, mvp;
	
		glfwGetFramebufferSize(window, &width, &height);
		
		ratio = (float) width / (float) height;
		
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		glm::mat4 m = glm::mat4(1.0f); // identity
		glm::rotate(m,(float) glfwGetTime(), glm::vec3(0.f,1.f,0.f));
		glm::mat4 p = glm::ortho(-1.f,1.f,-1.f,1.f);
		glm::mat4 mvp = p*m;

		//mat4x4_identity(m);
		//mat4x4_rotate_X(m, m, (float) glfwGetTime());
		//mat4x4_ortho(p, -1.0, 1.0, -1.f, 1.f, 1.f, -1.f);
		
		//mat4x4_mul(mvp, p, m);
	
		glUseProgram(program);
		//glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, &mvp[0][0]);
		
		auto elapsed = std::chrono::steady_clock::now() - chrStartTime;
		GLfloat utime = double(std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count()*0.000001);
		//std::cout << utime << "\n";
		glUniform1f(utime_location, utime);
		glUniform2f(ures_location, width, height);
		
		gpuTimer.startQuery();
		
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		gpuTimer.printQuery();
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	glfwDestroyWindow(window);
	
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
 
