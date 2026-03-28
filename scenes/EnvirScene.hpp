#include "../src/suOGL.hpp"
#include "../src/Input/GLFW/GLFWKeyboard.hpp"
#include "../src/Input/GLFW/GLFWMouse.hpp"
#include "../src/Input/GLFW/GLFWCallbackTest.hpp"

using namespace ehj;

#define FULLSCREEN false

#define SCENETYPE EnvirScene
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

		m_pWindow = glfwCreateWindow(m_windowRes.x,m_windowRes.y, "ehjE EnvirScene", NULL, NULL);
		ehjSetGLFWicon(m_pWindow);
	
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
		ehj_gl_err_callback();
		glViewport(0, 0, m_windowRes.x, m_windowRes.y);
	}

	int run() {
		//glfwSetErrorCallback(error_callback); //TODO
		//TODO //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	#ifdef EHJ_DBG
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			__debugbreak();
		}
	#endif
		
		ehj::Mesh mesh("models/monkey.obj");
		//ehj::Mesh mesh("myModels/cornellBoxObj.obj");
		//ehj::Mesh mesh("myModels/sponza/obj/Sponza.obj");

		//ehj::Model model("myModels/sponza/gltf/Sponza.gltf");
		//ehj::Mesh& mesh = model.m_meshes[0];

		mesh.toTriangles();
		mesh.m_vertexData.assembleVertexBuffer({&mesh});
		GLVertexBuffer glVb(mesh.m_vertexData);
		GLMesh glMesh(mesh, GL_DYNAMIC_DRAW);

		glVb.bind(0); // instead of glBindVertexArray(glMesh.getVAO());
		glMesh.bind();
		
		GLProgram glp;
		glp.addSourceFromFile("shaders/basic_v.vert");
		glp.addSourceFromFile("shaders/basic_f.frag");

		glp.createProgram();
		glp.bind();

		
		glBindAttribLocation(glp.getID(),glVb.getAttribPos(),"vPos");
		glBindAttribLocation(glp.getID(),glVb.getAttribNrm(),"vNrm");
		glBindAttribLocation(glp.getID(),glVb.getAttribUV(),"vUV");


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

		// set init cam pos
		m_cam.setPos({0.,0.,2.});
		
		while (!glfwWindowShouldClose(m_pWindow))
		{
			float deltaTime = m_clock.update();
			time += deltaTime;
			
			m_cam.update(deltaTime);

			int width, height;
			glfwGetFramebufferSize(m_pWindow, &width, &height);
			m_windowRes = glm::ivec2(width,height);
			
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			m_cam.setProj(glm::perspective(glm::radians(90.0f), (float)m_windowRes.x/(float)m_windowRes.y,0.01f,100.0f));
			glm::mat4 pvm = m_cam.getPV();
			//pvm = pvm* glm::scale(mat4(1.),vec3(0.005));
		
			glUniformMatrix4fv(glp.getUnfLoc("u_pvm"), 1, GL_FALSE, &pvm[0][0]);

			glUniform1f(glp.getUnfLoc("u_time"), time);
			glUniform2f(glp.getUnfLoc("u_resolution"), width, height);
			glUniform1i(glp.getUnfLoc("u_tess"), (GLint) guiTess);

	
			//glDepthMask(GL_TRUE);
			fragSTimer.start();
				glDrawElements(GL_TRIANGLES,glMesh.getEBOsize(),GL_UNSIGNED_INT,0);
			fragSTimer.end();

			{ // test render model second time
				//pvm = m_cam.getPV();
				//glm::mat4 t2(1.f);
				//pvm = pvm * glm::translate(t2,glm::vec3(1.f,0.f,0.f));
				//glUniformMatrix4fv(mainGLP.getUnfLoc("u_pvm"), 1, GL_FALSE, &pvm[0][0]);
				//glDrawElements(GL_TRIANGLES,oglMesh.getEBOsize(),GL_UNSIGNED_INT,0);
			}

	

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
