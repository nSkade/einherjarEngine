#include "../src/suOGL.hpp"
#include "../src/Input/GLFW/GLFWKeyboard.hpp"
#include "../src/Input/GLFW/GLFWMouse.hpp"
#include "../src/Input/GLFW/GLFWCallbackTest.hpp"
#include "../src/Input/GLFW/GLFWKeyboardCache.hpp"

#include <ffx-fsr/ffx_a.h>

using namespace ehj;

#define FULLSCREEN false

class FSRTestScene : IScene {
public:
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
	}
	static void processInput(GLFWwindow *window) {
		if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
	}

	~FSRTestScene() {
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

		glfwSetCursorPosCallback(m_pWindow, m_mouse->mouse_callback);
		glfwSetMouseButtonCallback(m_pWindow, m_mouse->mouse_button_callback);
		glfwSetKeyCallback(m_pWindow, m_kb->key_callback);

		glfwMakeContextCurrent(m_pWindow);
		gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
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
		GLProgram glpEASU;
		
		ehj::SSMesh mesh;
		mesh.toTriangles();
		OGLMesh oglMesh(mesh, GL_DYNAMIC_DRAW);
		oglMesh.bind(0);

		ehj_gl_err();
		glBindVertexArray(oglMesh.getVAO());
		ehj_gl_err();
		
		//glpMain.loadProgramFromFolder("shaders");
		glpMain.addSourceFromFile("shaders/basic_v.vert");
		std::string fragShader = "tellu.frag";
		glpMain.addSourceFromFileRecursive("shaders/"+fragShader);

		glpMain.createProgram();
		glBindAttribLocation(glpMain.getProgramID(),oglMesh.getAttribPos(),"vPos");
		if (oglMesh.getAttribNrm()!=-1)
			glBindAttribLocation(glpMain.getProgramID(),oglMesh.getAttribNrm(),"vNrm");
		ehj_gl_err();
		glpMain.bind();

		glpEASU.addSourceFromFile("shaders/basic_v.vert");
		std::string fragShader2 = "invert.frag";
		glpEASU.addSourceFromFileRecursive("shaders/"+fragShader2);

		glpEASU.createProgram();
		glBindAttribLocation(glpEASU.getProgramID(),oglMesh.getAttribPos(),"vPos");
		if (oglMesh.getAttribNrm()!=-1)
			glBindAttribLocation(glpEASU.getProgramID(),oglMesh.getAttribNrm(),"vNrm");
		ehj_gl_err();
		//glpEASU.bind();

		GPUTimer fragSTimer;
		GPUTimer fsrTimer;

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(m_pWindow,true);
		ImGui_ImplOpenGL3_Init("#version 460");

		int guiTess = 1;
		float time = 0.0f;

		glm::ivec2 renderRes = {960,540};
		GLFrameBuffer fb(renderRes);

		glBindVertexArray(oglMesh.getVAO());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oglMesh.getEBO());
		bool enable_fsr = true;

		//Timer frameTimer;
		GLFWfpsLimiter fpsLimiter;
		
		while (!glfwWindowShouldClose(m_pWindow))
		{
			if (GLFWKeyboardCache::keyPressed(IBCodes::KK_KEY_R)) {
				glUseProgram(0);
				glpMain.loadProgramFromFolder("shaders");
				glpMain.addSourceFromFileRecursive("shaders/"+fragShader);

				glpMain.createProgram();
		
				glBindAttribLocation(glpMain.getProgramID(),oglMesh.getAttribPos(),"vPos");
				if (oglMesh.getAttribNrm()!=-1)
					glBindAttribLocation(glpMain.getProgramID(),oglMesh.getAttribNrm(),"vNrm");
			}
			glpMain.bind();

			float deltaTime = m_clock.update();
			time += deltaTime;
			
			m_cam.update(deltaTime);

			int width, height;
			glfwGetFramebufferSize(m_pWindow, &width, &height);
			m_windowRes = glm::ivec2(width,height);
			
			glClear(GL_COLOR_BUFFER_BIT);
			glBindFramebuffer(GL_FRAMEBUFFER,fb.getFBO());
			glClear(GL_COLOR_BUFFER_BIT);

			m_cam.setProj(glm::perspective(glm::radians(90.0f), (float)renderRes.x/(float)renderRes.y,0.01f,100.0f));
			glm::mat4 pvm = m_cam.getPV();
			pvm = glm::ortho(-1.f,1.f,-1.f,1.f);
		
			glUniformMatrix4fv(glpMain.getUnfLoc("u_pvm"), 1, GL_FALSE, &pvm[0][0]);

			glUniform1f(glpMain.getUnfLoc("u_time"), time);
			glUniform2f(glpMain.getUnfLoc("u_resolution"), renderRes.x, renderRes.y);
			glUniform1i(glpMain.getUnfLoc("u_tess"), (GLint) guiTess);
			glm::vec3 cPos = m_cam.getPos();
			glUniform3f(glpMain.getUnfLoc("u_cPos"), cPos.x,cPos.y,cPos.z);
			glm::vec3 cDir = m_cam.getDir();
			glUniform3f(glpMain.getUnfLoc("u_cDir"), cDir.x,cDir.y,cDir.z);
			glm::vec3 cUp = m_cam.getUp();
			glUniform3f(glpMain.getUnfLoc("u_cUp"), cUp.x,cUp.y,cUp.z);
			glm::vec3 cRgt = m_cam.getRight();
			glUniform3f(glpMain.getUnfLoc("u_cRgt"), cRgt.x,cRgt.y,cRgt.z);
			float cFoc = m_cam.getFocus();
			glUniform1f(glpMain.getUnfLoc("u_cFoc"), cFoc);

			ehj_gl_err();
			glDepthMask(GL_TRUE);
			fragSTimer.start();
				glDrawElements(GL_TRIANGLES,oglMesh.getEBOsize(),GL_UNSIGNED_INT,0);
			fragSTimer.end();

			//fb.update(m_windowRes);
			ehj_gl_err();
			glBindFramebuffer(GL_FRAMEBUFFER,0);
			ehj_gl_err();

			fsrTimer.start();
			glpEASU.bind();
			ehj_gl_err();
			glUniformMatrix4fv(glpEASU.getUnfLoc("u_pvm"), 1, GL_FALSE, &pvm[0][0]);
			glUniform1f(glpEASU.getUnfLoc("u_time"), time);
			glUniform1i(glpEASU.getUnfLoc("u_enableFSR"), enable_fsr);
			glUniform2f(glpEASU.getUnfLoc("u_texRes"), renderRes.x, renderRes.y);
			glUniform2f(glpEASU.getUnfLoc("u_resolution"), m_windowRes.x, m_windowRes.y);
			ehj_gl_err();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,fb.getTexCol());
			ehj_gl_err();
			glDrawElements(GL_TRIANGLES,oglMesh.getEBOsize(),GL_UNSIGNED_INT,0);
			ehj_gl_err();
			fsrTimer.end();

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			{
				ImGui::Begin("Render Info");
				std::string fps = "fps: " + std::to_string(1.0/(fragSTimer.getMS()/1000.0));
				//std::string fps = "fps: " + std::to_string(1.0/(frameTimer.endTimer()*0.001));
				// need to take samples over larger timespan
				//frameTimer.startTimer();
				std::string frameTime = "ms: " + std::to_string(fragSTimer.getMS());
				ImGui::TextUnformatted(fps.c_str());
				ImGui::TextUnformatted(frameTime.c_str());
				std::string fsrTime = "ms frag: " + std::to_string(fsrTimer.getMS());
				ImGui::TextUnformatted(fsrTime.c_str());
				ImGui::Button("enable_fsr");
				if (ImGui::IsItemClicked(0))
					enable_fsr = !enable_fsr;
				ImGui::End();
			}
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			ehj_gl_err();

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
