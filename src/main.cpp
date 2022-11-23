
#include "../lib/imgui/imgui.h"
#include "../lib/imgui/backends/imgui_impl_glfw.h"
#include "../lib/imgui/backends/imgui_impl_opengl3.h"

#include "../lib/glad/glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "../lib/glm/glm.hpp"
#include "../lib/glm/ext/matrix_clip_space.hpp"
#include "../lib/glm/ext/matrix_projection.hpp"
#include "../lib/glm/ext/matrix_transform.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <chrono>
#include <fstream>
#include <sstream>

#include "GPUTimer.hpp"
#include "GLProgram.hpp"

#include <stdlib.h>
#define sleep _sleep

//TODO remove?
// enable optimus!
extern "C" {
	_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
	_declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

//static const struct
//{
//	float x, y;
//	float u, v;
//} vertices[6] =
//{
//	{ -1.0f, -1.0f, 0.f, 0.f },
//	{  1.0f,  1.0f, 1.f, 1.f },
//	{ -1.0f,  1.0f, 0.f, 1.f },
//	{ -1.0f, -1.0f, 0.f, 0.f },
//	{  1.0f, -1.0f, 1.f, 0.f },
//	{  1.0f,  1.0f, 1.f, 1.f }
//};
//
//static const int indices[4] = {
//	3,4,5,2
//	//0,1,2,
//	//3,4,5
//};

static const struct
{
	float x, y;
	float u, v;
	float n1,n2,n3;
} vertices[4] =
{
	{ -1.0f, -1.0f, 0.f, 0.f,     0.0f,0.0f,1.0f },
	{  1.0f, -1.0f, 1.f, 0.f,     0.0f,0.0f,1.0f },
	{  1.0f,  1.0f, 1.f, 1.f,     0.0f,0.0f,1.0f },
	{ -1.0f,  1.0f, 0.f, 1.f,     0.0f,0.0f,1.0f },
};

static const int indices[4] = {
	0,1,2,3
};

uint32_t ehj_gl_err() {
	unsigned int err = 0;
	err = glGetError();
	while ((err != 0)) {
		std::cout << "\nehj_gl_err: " << err << "\n";
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
#define FRAME_CAP true

int main(void)
{
	glm::vec3 vec = glm::vec3(0.0f,1.0f,0.0f);
	
	GLFWwindow* window;
	//GLuint vertex_buffer;
	GLuint program;
	GLint mvp_location;
	
	ehj::GLProgram mainGLProgram;

	glfwSetErrorCallback(error_callback);
	
	std::chrono::steady_clock::time_point chrStartTime = std::chrono::steady_clock::now();
	
	if (!glfwInit())
		exit(EXIT_FAILURE);
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	
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
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	if (!FRAME_CAP)
		glfwSwapInterval(0);
	
	// deprecated (GL 2.0)
	//glGenBuffers(1, &vertex_buffer);
	//glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	GLuint attribPos = 0;
	GLuint attribCol = 1;
	GLuint attribNrm = 2;

	uint32_t quadVBO; // vertex buffer object
	glCreateBuffers(1,&quadVBO);
	glNamedBufferStorage(quadVBO, sizeof(vertices), vertices, GL_DYNAMIC_STORAGE_BIT);

	uint32_t quadVAO; // vertex array object
	glCreateVertexArrays(1,&quadVAO);

	GLuint vaoBindingPoint = 0;
	glVertexArrayVertexBuffer(quadVAO,vaoBindingPoint,quadVBO,0,sizeof(float)*7);

	glEnableVertexArrayAttrib(quadVAO,attribPos);
	glEnableVertexArrayAttrib(quadVAO,attribCol);
	glEnableVertexArrayAttrib(quadVAO,attribNrm);

	glVertexArrayAttribFormat(quadVAO,attribPos,2,GL_FLOAT,false,0);
	glVertexArrayAttribFormat(quadVAO,attribCol,2,GL_FLOAT,false,2*sizeof(float));
	glVertexArrayAttribFormat(quadVAO,attribNrm,3,GL_FLOAT,false,4*sizeof(float));

	glVertexArrayAttribBinding(quadVAO,attribPos,vaoBindingPoint);
	glVertexArrayAttribBinding(quadVAO,attribCol,vaoBindingPoint);
	glVertexArrayAttribBinding(quadVAO,attribNrm,vaoBindingPoint);

	uint32_t quadEBO; // element buffer object
	glCreateBuffers(1,&quadEBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW); //TODO static draw?

	glBindVertexArray(quadVAO);
	std::cout << "check0\n";
	if (ehj_gl_err())
		return -1;
	
	// tesselation maximum supported vertices
	//GLint MaxPatchVertices = 0;
	//glGetIntegerv(GL_MAX_PATCH_VERTICES, &MaxPatchVertices);
	//std::cout << "Max supported patch vertices "<< MaxPatchVertices << "\n";
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	
	mainGLProgram.addSourceFromFile("assets/basic_v.vert", GL_VERTEX_SHADER);
	//mainGLProgram.addSourceFromFile("assets/tellu.frag",GL_FRAGMENT_SHADER);
	mainGLProgram.addSourceFromFile("assets/basic_f.frag",GL_FRAGMENT_SHADER);
	mainGLProgram.addSourceFromFile("assets/basic_tcsQ.glsl", GL_TESS_CONTROL_SHADER);
	mainGLProgram.addSourceFromFile("assets/basic_tesQ.glsl", GL_TESS_EVALUATION_SHADER);

	mainGLProgram.createProgram();
	program = mainGLProgram.getProgramID();
	glUseProgram(program);

	//sleep(2000);

	std::cout << "check1\n";
	if (ehj_gl_err())
		return -1;
	mvp_location = glGetUniformLocation(program, "MVP");
	//GLuint vpos_location = 0;//glGetAttribLocation(program, "vPos");
	//GLuint vcol_location = 1;//glGetAttribLocation(program, "vCol");
	GLint utime_location = glGetUniformLocation(program, "u_time");
	GLint ures_location = glGetUniformLocation(program, "u_resolution");

	GLint tessQ_location = glGetUniformLocation(program, "u_tessQ");
	std::cout << "\n tessQ: " << tessQ_location << "\n";
	
	std::cout << program << " " <<mvp_location << " " << utime_location << " " << ures_location;
	//glEnableVertexAttribArray(vpos_location);
	//glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
	//					sizeof(vertices[0]), (void*) 0);
	//glEnableVertexAttribArray(vcol_location);
	//glVertexAttribPointer(vcol_location, 2, GL_FLOAT, GL_FALSE,
	//					sizeof(vertices[0]), (void*) (sizeof(float) * 2));
	
	std::cout << "check2\n";
	if (ehj_gl_err())
		return -1;
	GPUTimer fragSTimer;
	
	//glLineWidth(1.0f);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	//for key and mouse buttons
	//ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window,true);
	ImGui_ImplOpenGL3_Init("#version 460");

	int guiTess = 0.0f;
	
	while (!glfwWindowShouldClose(window))
	{
		float ratio;
		int width, height;
		//mat4x4 m, p, mvp;

		glfwGetFramebufferSize(window, &width, &height);
		
		ratio = (float) width / (float) height;
		
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);

		glm::mat4 m = glm::mat4(1.0f); // identity
		//m = glm::rotate(m,(float) glfwGetTime(), glm::vec3(0.f,1.f,0.f));
		//m = glm::translate(m,glm::vec3(0.0f,0.0f,0.0f));
		glm::mat4 p = glm::ortho(-1.f,1.f,-1.f,1.f);
		glm::mat4 mvp = p*m;
		
		glUseProgram(program);
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, &mvp[0][0]);
		
		auto elapsed = std::chrono::steady_clock::now() - chrStartTime;
		GLfloat utime = double(std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count()*0.000001);
		//std::cout << utime << "\n";
		
		glUniform1f(utime_location, utime);
		glUniform2f(ures_location, width, height);
		glUniform1i(tessQ_location, (GLint) guiTess);
		
		fragSTimer.start();
		//glDrawArrays(GL_TRIANGLES, 0, 6);
		//glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
		//glDrawArrays(GL_PATCHES,0,6);
		glDrawElements(GL_PATCHES,4,GL_UNSIGNED_INT,0);
		fragSTimer.end();
		//fragSTimer.print();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		{
			ImGui::Begin("Render Info");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			//ImGui::Text("Hello from another window!");
			std::string fps = "fps: " + std::to_string(1.0/(fragSTimer.getMS()/1000.0));
			std::string frameTime = "ms: " + std::to_string(fragSTimer.getMS());
			ImGui::TextUnformatted(fps.c_str());
			ImGui::TextUnformatted(frameTime.c_str());
			ImGui::SliderInt("tess", &guiTess,0,100);
			ImGui::End();
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
 