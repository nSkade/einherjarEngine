#include <suOGL.hpp>

#include <stdlib.h>
#include <stdio.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

using namespace glm;

#define SCENETYPE TexturedTriScene
class TexturedTriScene : IScene {
public:

//static constexpr struct
//{
//	float x, y;
//	float r, g, b;
//} vertices[3] =
//{
//	{ -0.6f, -0.4f, 1.f, 0.f, 0.f },
//	{  0.6f, -0.4f, 0.f, 1.f, 0.f },
//	{   0.f,  0.6f, 0.f, 0.f, 1.f }
//};

static constexpr struct
{
	glm::vec2 pos;
	glm::vec2 uv;
} vertices[6] =
{
	{{-.5f,-.5f},{0.f, 0.f}},
	{{ .5f,-.5f},{1.f, 0.f}},
	{{-.5f, .5f},{0.f, 1.f}},
	{{ .5f,-.5f},{1.f, 0.f}},
	{{ .5f, .5f},{1.f, 1.f}},
	{{-.5f, .5f},{0.f, 1.f}}
};

static constexpr char* vertex_shader_text = (char*)
"#version 330 core\n"
"uniform mat4 MVP;\n"
"attribute vec2 vPos;\n"
"attribute vec2 vTex;\n"
"varying vec2 texUV;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    texUV = vTex;\n"
"}\n";
 
static constexpr char* fragment_shader_text = (char*)
"#version 330 core\n"
"uniform sampler2D u_tex;\n"
"varying vec2 texUV;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(1.0);\n"
"    gl_FragColor = vec4(texture(u_tex,texUV).xyz,0.);\n"
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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

	window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
	ehjSetGLFWicon(window);
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

	// current info state on vertex data
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLProgram shadProg;
	shadProg.addSourceFromString(std::string(vertex_shader_text),GL_VERTEX_SHADER);
	shadProg.addSourceFromString(std::string(fragment_shader_text),GL_FRAGMENT_SHADER);
	shadProg.createProgram();
	shadProg.bind();

	mvp_location = shadProg.getUnfLoc("MVP");
	vpos_location = shadProg.getAttribLoc("vPos");
	vcol_location = shadProg.getAttribLoc("vTex");

	glEnableVertexAttribArray(vpos_location);
	std::cout << sizeof(vertices[0]) << std::endl;
	glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
						  sizeof(vertices[0]), (void*) 0);
	glEnableVertexAttribArray(vcol_location);
	glVertexAttribPointer(vcol_location, 2, GL_FLOAT, GL_FALSE,
						  sizeof(vertices[0]), (void*) (sizeof(GLfloat) * 2));

	ivec2 t_res;
	int t_channels;
	stbi_set_flip_vertically_on_load(true);
	stbi_flip_vertically_on_write(true);

	GLubyte* data = stbi_load("models/test.png",&t_res[0],&t_res[1], &t_channels,0);
	//GLubyte* data = stbi_load("myModels/pb.png",&t_res[0],&t_res[1], &t_channels,0);
	if (!data) {
		std::cerr << "Failed to load image" << std::endl;
		return -1;
	}
	//stbi_write_png("models/test3.png",t_res[0],t_res[1],3,data,t_res[0]*3);

	uint32_t texture;
	glGenTextures(1,&texture);
	glBindTexture(GL_TEXTURE_2D,texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t_res[0], t_res[1], 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	//glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,texture);
	ehj_gl_err();

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
		//m = glm::rotate(m,(float) glfwGetTime(), glm::vec3(0.f,1.f,0.f));
		//m = glm::translate(m,glm::vec3(0.0f,0.0f,0.0f));
		glm::mat4 p = glm::ortho(-1.f,1.f,-1.f,1.f);
		//glm::mat4 p = glm::perspectiveFov(45,10,10,0.1,1000);
		glm::mat4 mvp = p*m;
		//mvp = proj;
		glUseProgram(shadProg.getProgramID());
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) &(mvp[0].x));
		glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices)/sizeof(vertices[0]));

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
