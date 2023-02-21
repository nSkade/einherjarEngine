#include "../src/suOGL.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <chrono>
#include <fstream>
#include <sstream>

#include <stdlib.h>
#define sleep _sleep
class SampleScene : IScene {
public:

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

static constexpr struct
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

static constexpr int indices[4] = {
	0,1,2,3
};


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

float lastX = 0.0f, lastY = 0.0f;
float yaw = 0.0f, pitch = 0.0f;
bool firstMouse = true;
bool mousePressed = false;
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		std::cout << "mouse pressed\n";
		mousePressed = true;
		firstMouse = true;
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		std::cout << "mouse pressed\n";
		mousePressed = false;
	}
};
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; 
	lastX = xpos;
	lastY = ypos;
	
	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;
	
	yaw   += xoffset;
	pitch += yoffset;
	
	if(pitch > 89.0f)
		pitch = 89.0f;
	if(pitch < -89.0f)
		pitch = -89.0f;
};

void setup() {
	
}

int run() {
	glm::vec3 vec = glm::vec3(0.0f,1.0f,0.0f);
	
	GLFWwindow* window;
	//GLuint vertex_buffer;
	GLuint program;
	GLint mvp_location;
	
	GLProgram mainGLProgram;

	glfwSetErrorCallback(error_callback);
	
	std::chrono::steady_clock::time_point chrStartTime = std::chrono::steady_clock::now();
	
	if (!glfwInit())
		exit(EXIT_FAILURE);
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

	uint32_t windowW, windowH;
	windowW = 640;
	windowH = 480;
	
#if FULLSCREEN
	windowW = 1920;
	windowH = 1080;
#endif
	window = glfwCreateWindow(windowW,windowH, "Simple example", NULL, NULL);
	
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
#if FULLSCREEN
	glfwSetWindowPos(window, 0,0);
#endif

	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	//TODO mouse callback not in class possible
	//glfwsetcursorposcallback(window, mouse_callback);
	//glfwsetmousebuttoncallback(window, mouse_button_callback);
	glfwSetKeyCallback(window, key_callback);
	
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	if (!FRAME_CAP)
		glfwSwapInterval(0);
	
	// deprecated (GL 2.0)
	//glGenBuffers(1, &vertex_buffer);
	//glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	
//	GLuint attribPos = 0;
//	GLuint attribCol = 1;
//	GLuint attribNrm = 2;
//	
//	uint32_t quadVBO; // vertex buffer object
//	glCreateBuffers(1,&quadVBO);
//	glNamedBufferStorage(quadVBO, sizeof(vertices), vertices, GL_DYNAMIC_STORAGE_BIT);
//
//	uint32_t quadVAO; // vertex array object
//	glCreateVertexArrays(1,&quadVAO);
//
//	GLuint vaoBindingPoint = 0;
//	glVertexArrayVertexBuffer(quadVAO,vaoBindingPoint,quadVBO,0,sizeof(float)*7);
//
//	glEnableVertexArrayAttrib(quadVAO,attribPos);
//	glEnableVertexArrayAttrib(quadVAO,attribCol);
//	glEnableVertexArrayAttrib(quadVAO,attribNrm);
//
//	glVertexArrayAttribFormat(quadVAO,attribPos,2,GL_FLOAT,false,0);
//	glVertexArrayAttribFormat(quadVAO,attribCol,2,GL_FLOAT,false,2*sizeof(float));
//	glVertexArrayAttribFormat(quadVAO,attribNrm,3,GL_FLOAT,false,4*sizeof(float));
//
//	glVertexArrayAttribBinding(quadVAO,attribPos,vaoBindingPoint);
//	glVertexArrayAttribBinding(quadVAO,attribCol,vaoBindingPoint);
//	glVertexArrayAttribBinding(quadVAO,attribNrm,vaoBindingPoint);
//
//	uint32_t quadEBO; // element buffer object
//	glCreateBuffers(1,&quadEBO);
//
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW); //TODO static draw?
	
	// load model
	//ehj::Mesh mesh = ehj::SSMesh(false);
	ehj::Mesh mesh; mesh.loadOBJ("models/ssn4.obj");
	mesh.toTriangles();
	
	GLuint attribPos = 0;
	//GLuint attribCol = 1;
	GLuint attribNrm = 1;
	
	std::vector<float> VAO = mesh.getVertexBuffer();
	std::vector<int> EBO = mesh.getIndexBuffer();
	
	uint32_t BP = mesh.getMP();
	uint32_t Dim = mesh.getDim();
	uint32_t BPC = 1;
	if (BP & ehj::Mesh::MP_NORMAL)
		BPC++;
	
	uint32_t quadVBO; // vertex buffer object
	glCreateBuffers(1,&quadVBO);
	glNamedBufferStorage(quadVBO, VAO.size()*sizeof(float), &VAO[0], GL_DYNAMIC_STORAGE_BIT);

	uint32_t quadVAO; // vertex array object
	glCreateVertexArrays(1,&quadVAO);

	GLuint vaoBindingPoint = 0;
	glVertexArrayVertexBuffer(quadVAO,vaoBindingPoint,quadVBO,0,sizeof(float)*Dim*BPC);

	glEnableVertexArrayAttrib(quadVAO,attribPos);
	glEnableVertexArrayAttrib(quadVAO,attribNrm);

	glVertexArrayAttribFormat(quadVAO,attribPos,3,GL_FLOAT,GL_FALSE,0);
	glVertexArrayAttribFormat(quadVAO,attribNrm,3,GL_FLOAT,GL_FALSE,3*sizeof(float));

	glVertexArrayAttribBinding(quadVAO,attribPos,vaoBindingPoint);
	glVertexArrayAttribBinding(quadVAO,attribNrm,vaoBindingPoint);

	uint32_t quadEBO; // element buffer object
	glCreateBuffers(1,&quadEBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, EBO.size()*sizeof(int), &EBO[0], GL_DYNAMIC_DRAW); //TODO static draw?

	glBindVertexArray(quadVAO);
	std::cout << "check0\n";
	ehj_gl_err();
	
	// tesselation maximum supported vertices
	//GLint MaxPatchVertices = 0;
	//glGetIntegerv(GL_MAX_PATCH_VERTICES, &MaxPatchVertices);
	//std::cout << "Max supported patch vertices "<< MaxPatchVertices << "\n";
	glPatchParameteri(GL_PATCH_VERTICES, 4);

	//mainGLProgram.loadProgramFromFolder("shaders/tessQ");
	ehj_gl_err();
	mainGLProgram.addSourceFromFile("shaders/basic_v.vert", GL_VERTEX_SHADER);
	//mainGLProgram.addSourceFromFile("shaders/tessQ/basic_v.vert", GL_VERTEX_SHADER);
	ehj_gl_err();
	//mainGLProgram.addSourceFromFile("shaders/tellu.frag",GL_FRAGMENT_SHADER);
	mainGLProgram.addSourceFromFile("shaders/basic_f.frag",GL_FRAGMENT_SHADER);
	//mainGLProgram.addSourceFromFile("shaders/tessQ/basic_f.frag",GL_FRAGMENT_SHADER);
	ehj_gl_err();
	//mainGLProgram.addSourceFromFile("shaders/tessQ/basic_tcsQ.glsl", GL_TESS_CONTROL_SHADER);
	ehj_gl_err();
	//mainGLProgram.addSourceFromFile("shaders/tessQ/basic_tesQ.glsl", GL_TESS_EVALUATION_SHADER);
	//mainGLProgram.addSourceFromFile("shaders/tellu_tesQ.glsl", GL_TESS_EVALUATION_SHADER);

	ehj_gl_err();
	mainGLProgram.createProgram();
	ehj_gl_err();
	program = mainGLProgram.getProgramID();
	glBindAttribLocation(program,attribPos,"vPos");
	glBindAttribLocation(program,attribNrm,"vNrm");
	ehj_gl_err();
	glUseProgram(program);

	sleep(2000);

	std::cout << "check1\n";
	ehj_gl_err();
	mvp_location = glGetUniformLocation(program, "MVP");
	//GLuint vpos_location = 0;//glGetAttribLocation(program, "vPos");
	//GLuint vcol_location = 1;//glGetAttribLocation(program, "vCol");
	GLint utime_location = glGetUniformLocation(program, "u_time");
	GLint ures_location = glGetUniformLocation(program, "u_resolution");
	GLint uview_location = glGetUniformLocation(program, "u_view");

	GLint tessQ_location = glGetUniformLocation(program, "u_tessQ");
	std::cout << "\n tessQ: " << tessQ_location << "\n";
	
	std::cout << program << " " <<mvp_location << " " << utime_location << " " << ures_location;
	//glEnableVertexAttribArray(vpos_location);
	//glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
	//					sizeof(vertices[0]), (void*) 0);
	//glEnableVertexAttribArray(vcol_location);
	//glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
	//					sizeof(vertices[0]), (void*) (sizeof(float) * 3));
	
	std::cout << "check2\n";
	ehj_gl_err();
	GPUTimer fragSTimer;
	
	//glLineWidth(1.0f);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	//glFrontFace(GL_CW);

	//proj
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)windowW/(float)windowH,0.0f,1000.0f);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	//for key and mouse buttons
	//ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window,true);
	ImGui_ImplOpenGL3_Init("#version 460");

	int guiTess = 1;
	
	while (!glfwWindowShouldClose(window))
	{
		float ratio;
		int width, height;
		//mat4x4 m, p, mvp;

		//camera
		static glm::vec3 camPos = glm::vec3(0.0f,0.0f,3.0f);
		static glm::vec3 camTar = glm::vec3(0.0f,0.0f,0.0f);
		static glm::vec3 camDir = glm::normalize(camPos-camTar);

		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		camDir = glm::normalize(direction);
		
		static glm::vec3 camUp = glm::vec3(0.0f,1.0f,0.0f);
		static glm::vec3 camRight = glm::normalize(glm::cross(camUp,camDir));
		camUp = glm::cross(camDir,camRight);
	
		const float cameraSpeed = 0.05f; // adjust accordingly
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camPos += cameraSpeed * (-camDir);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camPos -= cameraSpeed * (-camDir);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camPos -= glm::normalize(glm::cross(-camDir, camUp)) * cameraSpeed;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camPos += glm::normalize(glm::cross(-camDir, camUp)) * cameraSpeed;
		
		glm::mat4 view = glm::lookAt(camPos,camPos-camDir,camUp);
		
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
		//glm::mat4 p = glm::perspectiveFov(45,10,10,0,1000);
		glm::mat4 mvp = p*m;
		//mvp = proj;
		
		glUseProgram(program);
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, &mvp[0][0]);
		
		auto elapsed = std::chrono::steady_clock::now() - chrStartTime;
		GLfloat utime = double(std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count()*0.000001);
		//std::cout << utime << "\n";
		
		glUniform1f(utime_location, utime);
		glUniform2f(ures_location, width, height);
		glUniform1i(tessQ_location, (GLint) guiTess);

		//const float radius = 10.0f;
		//float camX = sin(glfwGetTime()) * radius;
		//float camZ = cos(glfwGetTime()) * radius;
		//view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		view = glm::mat4(1.0f);
		glUniformMatrix4fv(uview_location,1,GL_FALSE,glm::value_ptr(view));
		
		ehj_gl_err();
		
		fragSTimer.start();
		//glDrawArrays(GL_TRIANGLES, 0, 6);
		//glDrawElements(GL_TRIANGLES,EBO.size(),GL_UNSIGNED_INT,0);
		//glDrawArrays(GL_PATCHES,0,6);
		glDrawElements(GL_TRIANGLES,EBO.size(),GL_UNSIGNED_INT,0);
		//glDrawArrays(GL_QUADS,0,EBO.size());
		//glDrawElements(GL_PATCHES, EBO.size(),GL_UNSIGNED_INT,0);
		fragSTimer.end();
		//fragSTimer.print();

		ehj_gl_err();

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
			ImGui::SliderInt("tess", &guiTess,1,64);
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
	};
	void cleanup() {

	}
};
