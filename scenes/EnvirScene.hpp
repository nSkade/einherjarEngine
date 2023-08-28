#include "../src/suOGL.hpp"
#include "../src/Input/GLFW/GLFWKeyboard.hpp"
#include "../src/Input/GLFW/GLFWMouse.hpp"
#include "../src/Input/GLFW/GLFWCallbackTest.hpp"

using namespace ehj;

#define FULLSCREEN false

class EnvirScene : IScene {
public:
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
	}
	static void processInput(GLFWwindow *window) {
		if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
	}

	~EnvirScene() {
		//__debugbreak();
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

		// make sure to keep instances alive
		m_kb = ehj::GLFWKeyboard::instance();
		m_mouse = ehj::GLFWMouse::instance();

		//TODO set with imgui, exit with escape
		//glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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
		//glfwSetErrorCallback(error_callback); //TODO
		//TODO //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	#ifdef EHJ_DBG
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			__debugbreak();
		}
	#endif
		
		GLProgram mainGLP;
		
		ehj::Mesh mesh; mesh.loadOBJ("models/monkey.obj");
		mesh.toTriangles();
		OGLMesh oglMesh(mesh, GL_DYNAMIC_DRAW);
		oglMesh.bind(0);

		ehj_gl_err();
		glBindVertexArray(oglMesh.getVAO());
		ehj_gl_err();
		
		// tesselation maximum supported vertices
		//GLint MaxPatchVertices = 0;
		//glGetIntegerv(GL_MAX_PATCH_VERTICES, &MaxPatchVertices);
		//std::cout << "Max supported patch vertices "<< MaxPatchVertices << "\n";
		//glPatchParameteri(GL_PATCH_VERTICES, 4);
		ehj_gl_err();
		
		mainGLP.loadProgramFromFolder("shaders");
		mainGLP.addSourceFromFile("shaders/basic_f.frag");
		//mainGLP.addSourceFromFile("shaders/basic_v.vert", GL_FRAGMENT_SHADER);
		//mainGLP.loadProgramFromFolder("shaders/tessQ/");
		//mainGLP.addSourceFromFile("shaders/tessQ/basic_tesQ.glsl",GL_TESS_EVALUATION_SHADER);

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
	//	glFrontFace(GL_CW);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		int guiTess = 1;
		float time = 0.0f;

		glEnable(GL_DEPTH_TEST);
		//glDepthFunc(GL_ALWAYS);
		glDepthFunc(GL_LESS);
		//glDepthMask(true);
		//glDepthRangef(0.0f,1.0f);
		
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
		//	glm::vec3 cPos = m_cam.getPos();
		//	glUniform3f(mainGLP.getUnfLoc("u_cPos"), cPos.x,cPos.y,cPos.z);
		//	glm::vec3 cDir = m_cam.getDir();
		//	glUniform3f(mainGLP.getUnfLoc("u_cDir"), cDir.x,cDir.y,cDir.z);
		//	glm::vec3 cUp = m_cam.getUp();
		//	glUniform3f(mainGLP.getUnfLoc("u_cUp"), cUp.x,cUp.y,cUp.z);
		//	glm::vec3 cRgt = m_cam.getRight();
		//	glUniform3f(mainGLP.getUnfLoc("u_cRgt"), cRgt.x,cRgt.y,cRgt.z);
		//	float cFoc = m_cam.getFocus();
		//	glUniform1f(mainGLP.getUnfLoc("u_cFoc"), cFoc);

			ehj_gl_err();
			//glDepthMask(GL_TRUE);
			fragSTimer.start();
				glDrawElements(GL_TRIANGLES,oglMesh.getEBOsize(),GL_UNSIGNED_INT,0);
			fragSTimer.end();

			pvm = m_cam.getPV();
			glm::mat4 t2(1.f);
			pvm = pvm * glm::translate(t2,glm::vec3(1.f,0.f,0.f));
			glUniformMatrix4fv(mainGLP.getUnfLoc("u_pvm"), 1, GL_FALSE, &pvm[0][0]);
			glDrawElements(GL_TRIANGLES,oglMesh.getEBOsize(),GL_UNSIGNED_INT,0);

			ehj_gl_err();

			glfwSwapBuffers(m_pWindow);
			glfwPollEvents();
			processInput(m_pWindow); // TODO check esc close window
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

	// to hold references to instances
	std::shared_ptr<ehj::GLFWKeyboard> m_kb;
	std::shared_ptr<ehj::GLFWMouse> m_mouse;

	FreeFlyCamera m_cam;
	Clock m_clock;
};
