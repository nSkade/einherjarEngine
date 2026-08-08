#include "../src/suOGL.hpp"
#include "../src/Input/GLFW/GLFWKeyboard.hpp"
#include "../src/Input/GLFW/GLFWMouse.hpp"
//#include "../src/Input/GLFW/GLFWCallbackTest.hpp"
#include "../src/Input/GLFW/GLFWKeyboardCache.hpp"
#include "suCMN.hpp"
#include <efsw/efsw.hpp>
#include <imgui.h>

#include <Utility/FileWatcher.hpp>

using namespace ehj;

#define FULLSCREEN false

#define SCENETYPE Plot2DScene
class Plot2DScene : IScene {
public:
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
	}
	static void processInput(GLFWwindow *window) {
		if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
	}

	~Plot2DScene() {
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

		m_pWindow = glfwCreateWindow(m_windowRes.x,m_windowRes.y, "ehjE Plot 2D", NULL, NULL);
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

		glfwSetCursorPosCallback(m_pWindow, m_mouse->mouse_callback);
		glfwSetMouseButtonCallback(m_pWindow, m_mouse->mouse_button_callback);
		glfwSetKeyCallback(m_pWindow, m_kb->key_callback);

		glfwMakeContextCurrent(m_pWindow);
		gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
		ehj_gl_err_callback();
		glViewport(0, 0, m_windowRes.x, m_windowRes.y);
		glfwSwapInterval(0);
	}

	int run() {
		GLint pvm_location;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	#ifdef EHJ_DBG
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			__debugbreak();
		}
	#endif
		
		GLProgram glpMain;
		
		ehj::SSMesh ssm; {
			ssm.toTriangles();
			ssm.assembleVertexBuffer();
		}
		GLVertexBuffer ssmGlVb(ssm.m_vertexData); ssmGlVb.bind(0);
		GLMesh ssmGl(ssm, GL_DYNAMIC_DRAW); ssmGl.bind();
		
		//glpMain.loadProgramFromFolder("shaders");
		glpMain.addSourceFromFile("shaders/basic_v.vert");
		glpMain.addSourceFromFileRecursive(std::string("scenes/plotGlsl2D/")+"grid.frag");

		glpMain.createProgram();

		glpMain.bind();

		//GLGPUTimer fragSTimer;

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(m_pWindow,true);
		ImGui_ImplOpenGL3_Init("#version 460");

		float time = 0.0f;

		//Timer frameTimer;
		GLFWfpsLimiter fpsLimiter;
		bool fps30 = true;
		glBindFramebuffer(GL_FRAMEBUFFER,0);

		ehj::FileWatcher glpMainFW(EHJ_THIS_FOLDER());
		
		while (!glfwWindowShouldClose(m_pWindow)) {
			{
				static bool recomp = false;
				static float timeLastCheck = 0.;
				if (glpMainFW.CheckAndReset()) {
					timeLastCheck = time;
					recomp = true;
				}
				if (recomp && time - timeLastCheck > 1.) {
				//if (GLFWKeyboardCache::keyPressed(IBCodes::KK_KEY_R)) {
					glUseProgram(0);
					glpMain.addSourceFromFile("shaders/basic_v.vert");
					glpMain.addSourceFromFileRecursive(std::string("scenes/plotGlsl2D/")+"grid.frag");

					glpMain.createProgram();
					recomp = false;
				}
			}
			if (GLFWKeyboardCache::keyPressed(IBCodes::KK_KEY_F)) {
				fps30 = !fps30;
				uint32_t n = fps30 ? 15 : fpsLimiter.getRefreshRate();
				fpsLimiter.setLimit(n);
			}

			glpMain.bind();

			float deltaTime = m_clock.update();
			time += deltaTime;
			
			//m_cam.update(deltaTime);

			int width, height;
			glfwGetFramebufferSize(m_pWindow, &width, &height);
			m_windowRes = glm::ivec2(width,height);
	

			//m_cam.setProj(glm::perspective(glm::radians(90.0f), (float)renderRes.x/(float)renderRes.y,0.01f,100.0f));
			glm::mat4 pvm = m_cam.getPV();
			pvm = glm::ortho(-1.f,1.f,-1.f,1.f);
			glUniformMatrix4fv(glpMain.getUnfLoc("u_pvm"), 1, GL_FALSE, &pvm[0][0]);

			glUniform1f(glpMain.getUnfLoc("u_time"), time);
			glUniform2iv(glpMain.getUnfLoc("u_resolution"), 1, &m_windowRes.x);

			//TODOff just use matrix
			//glm::vec3 cPos = m_cam.getPos();
			//glUniform3f(glpMain.getUnfLoc("u_cPos"), cPos.x,cPos.y,cPos.z);
			//glm::vec3 cDir = m_cam.getDir();
			//glUniform3f(glpMain.getUnfLoc("u_cDir"), cDir.x,cDir.y,cDir.z);
			//glm::vec3 cUp = m_cam.getUp();
			//glUniform3f(glpMain.getUnfLoc("u_cUp"), cUp.x,cUp.y,cUp.z);
			//glm::vec3 cRgt = m_cam.getRight();
			//glUniform3f(glpMain.getUnfLoc("u_cRgt"), cRgt.x,cRgt.y,cRgt.z);
			//float cFoc = m_cam.getFocus();
			//glUniform1f(glpMain.getUnfLoc("u_cFoc"), cFoc);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//glDepthMask(GL_TRUE);
			//fragSTimer.start();
				ssmGlVb.bind(0);
				ssmGl.bind();
				ssmGl.draw();
				//glDrawElements(GL_TRIANGLES,oglMesh.getEBOsize(),GL_UNSIGNED_INT,0);
			//fragSTimer.end();

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			{
				ImGui::Begin("Render Info");
				//std::string fps = "fps: " + std::to_string(1.0/(fragSTimer.getMS()/1000.0));
				////std::string fps = "fps: " + std::to_string(1.0/(frameTimer.endTimer()*0.001));
				//// need to take samples over larger timespan
				////frameTimer.startTimer();
				//std::string frameTime = "ms: " + std::to_string(fragSTimer.getMS());
				//ImGui::TextUnformatted(fps.c_str());
				//ImGui::TextUnformatted(frameTime.c_str());
				//std::string fsrTime = "ms frag: " + std::to_string(fsrTimer.getMS());
				{
					std::string t = std::to_string(fpsLimiter.getLimit());
					ImGui::TextUnformatted(t.c_str());
				}
				ImGui::End();
			}
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	

			glfwSwapBuffers(m_pWindow);
			glfwPollEvents();
			processInput(m_pWindow); // TODO check esc close window

			fpsLimiter.wait();
		}
		return 0;
	}
	void cleanup() {
		glfwDestroyWindow(m_pWindow);
		glfwTerminate();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
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

