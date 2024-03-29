#include "../src/suOGL.hpp"

#include <stdlib.h>
#include <stdio.h>

class TriangleHWScene : IScene {
public:

static constexpr struct
{
	float x, y;
	float r, g, b;
} vertices[3] =
{
	{ -0.6f, -0.4f, 1.f, 0.f, 0.f },
	{  0.6f, -0.4f, 0.f, 1.f, 0.f },
	{   0.f,  0.6f, 0.f, 0.f, 1.f }
};

static constexpr char* vertex_shader_text =
"#version 110\n"
"uniform mat4 MVP;\n"
"attribute vec3 vCol;\n"
"attribute vec2 vPos;\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    color = vCol;\n"
"}\n";
 
static constexpr char* fragment_shader_text =
"#version 110\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(color, 1.0);\n"
"}\n";

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}
void setup(void) {

};
//TODO rewrite in newer gl with helper functions
int run(void)
{
	GLFWwindow* window;
	GLuint vertex_buffer, vertex_shader, fragment_shader, program;
	GLint mvp_location, vpos_location, vcol_location;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	//TODO target at least 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval(1);
	// NOTE: OpenGL error checks have been omitted for brevity
	glEnable(GL_DEPTH_TEST);

	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
	glCompileShader(vertex_shader);

	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
	glCompileShader(fragment_shader);
 
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);
 
	mvp_location = glGetUniformLocation(program, "MVP");
	vpos_location = glGetAttribLocation(program, "vPos");
	vcol_location = glGetAttribLocation(program, "vCol");
 
	glEnableVertexAttribArray(vpos_location);
	glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
						  sizeof(vertices[0]), (void*) 0);
	glEnableVertexAttribArray(vcol_location);
	glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
						  sizeof(vertices[0]), (void*) (sizeof(float) * 2));
 
	while (!glfwWindowShouldClose(window))
	{
		float ratio;
		int width, height;
	//	mat4x4 m, p, mvp;
 
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float) height;
 
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
		glm::mat4 m = glm::mat4(1.0f); // identity
		m = glm::rotate(m,(float) glfwGetTime(), glm::vec3(0.f,1.f,0.f));
		//m = glm::translate(m,glm::vec3(0.0f,0.0f,0.0f));
		glm::mat4 p = glm::ortho(-1.f,1.f,-1.f,1.f);
		//glm::mat4 p = glm::perspectiveFov(45,10,10,0.1,1000);
		glm::mat4 mvp = p*m;
		//mvp = proj;
		glUseProgram(program);
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) &(mvp[0].x));
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
	return 0;
};
void cleanup() {

}
};
