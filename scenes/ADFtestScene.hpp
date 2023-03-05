#include "../src/suOGL.hpp"
#include "../src/Input/GLFW/GLFWKeyboard.hpp"
#include "../src/Input/GLFW/GLFWMouse.hpp"
#include "../src/Input/GLFW/GLFWCallbackTest.hpp"

//#include "../src/Structures/ADF.hpp"
#include "../src/MeshProcessing/DCADF.hpp"

using namespace ehj;

#define FULLSCREEN false

class ADFtestScene : IScene {
public:
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
	}
	static void processInput(GLFWwindow *window) {
		if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
	}
	
	void setup() {
		if (!glfwInit())
			exit(EXIT_FAILURE);
	
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

		m_windowRes.x = 640;
		m_windowRes.y = 480;
	
	#if FULLSCREEN
		m_windowRes.x = 1920;
		m_windowRes.y = 1080;
	#endif

		m_pWindow = glfwCreateWindow(m_windowRes.x,m_windowRes.y, "EnvirScene", NULL, NULL);
	
		if (!m_pWindow)
		{
			glfwTerminate();
			exit(EXIT_FAILURE);
		}
	#if FULLSCREEN
		glfwSetWindowPos(m_pWindow, 0,0);
	#endif
		glfwSetFramebufferSizeCallback(m_pWindow, framebuffer_size_callback);

		m_kb = ehj::GLFWKeyboard::instance();
		m_mouse = ehj::GLFWMouse::instance();

		glfwSetCursorPosCallback(m_pWindow, m_mouse->mouse_callback);
		glfwSetMouseButtonCallback(m_pWindow, m_mouse->mouse_button_callback);
		glfwSetKeyCallback(m_pWindow, m_kb->key_callback);

		glfwMakeContextCurrent(m_pWindow);
		gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
		glViewport(0, 0, m_windowRes.x, m_windowRes.y);
	}

	int run() {
		GLuint program;
		GLint pvm_location;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
		GLProgram mainGLP;
		
		ehj::Mesh mesh; mesh.loadOBJ("models/monkey.obj");
		mesh.toTriangles();
		OGLMesh oglMesh(mesh, GL_DYNAMIC_DRAW);
		oglMesh.bind(0);
		
		//TODO
		//ehj::Mesh mesh("models/ssn4.obj");
		//mesh.toTriangles();
		//ehj::Mesh mesh;
		//ADF adf;
		//adf.test();

		DCADF dcadf;
		dcadf.setAABB({glm::vec3(-1.5f), glm::vec3(1.5f)});
		dcadf.setSDF([](glm::vec3 p){ return glm::length(p-glm::vec3(0.0f))-1.0f; });
		dcadf.process();


		//mesh.storeOBJ("modelsOut/sdfc.obj");

		ehj_gl_err();
		glBindVertexArray(oglMesh.getVAO());
		ehj_gl_err();
		
		ehj_gl_err();
		
		mainGLP.loadProgramFromFolder("shaders");
		mainGLP.addSourceFromFile("shaders/basic_f.frag");

		mainGLP.createProgram();
		program = mainGLP.getProgramID();
		glBindAttribLocation(program,oglMesh.getAttribPos(),"vPos");
		if (oglMesh.getAttribNrm()!=-1)
			glBindAttribLocation(program,oglMesh.getAttribNrm(),"vNrm");
		ehj_gl_err();
		glUseProgram(program);

		GPUTimer fragSTimer;
	
		//glLineWidth(1.0f);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		int guiTess = 1;
		float time = 0.0f;

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		
		while (!glfwWindowShouldClose(m_pWindow))
		{
			float deltaTime = m_clock.update();
			time += deltaTime;
			
			m_cam.update(deltaTime);

			int width, height;
			glfwGetFramebufferSize(m_pWindow, &width, &height);
			m_windowRes = glm::ivec2(width,height);
			
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindVertexArray(oglMesh.getVAO());
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oglMesh.getEBO());

			m_cam.setProj(glm::perspective(glm::radians(90.0f), (float)m_windowRes.x/(float)m_windowRes.y,0.01f,100.0f));
			glm::mat4 pvm = m_cam.getPV();
		
			glUseProgram(program);
			glUniformMatrix4fv(mainGLP.getUnfLoc("u_pvm"), 1, GL_FALSE, &pvm[0][0]);

			glUniform1f(mainGLP.getUnfLoc("u_time"), time);
			glUniform2f(mainGLP.getUnfLoc("u_resolution"), width, height);
			glUniform1i(mainGLP.getUnfLoc("u_tess"), (GLint) guiTess);

			ehj_gl_err();
			fragSTimer.start();
				glDrawElements(GL_TRIANGLES,oglMesh.getEBOsize(),GL_UNSIGNED_INT,0);
			fragSTimer.end();

			ehj_gl_err();

			glfwSwapBuffers(m_pWindow);
			glfwPollEvents();
			processInput(m_pWindow);
		}
		return 0;
	}
	void cleanup() {
		glfwDestroyWindow(m_pWindow);
		glfwTerminate();

		ImGui::DestroyContext();
	}
private:
	GLFWwindow* m_pWindow;
	glm::ivec2 m_windowRes;

	std::shared_ptr<ehj::GLFWKeyboard> m_kb;
	std::shared_ptr<ehj::GLFWMouse> m_mouse;

	FreeFlyCamera m_cam;
	Clock m_clock;
};
