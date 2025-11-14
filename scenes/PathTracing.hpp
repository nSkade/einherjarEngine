#include "GAPI/OGL/GPUTimer.hpp"
#include <suOGL.hpp>

#include <Input/GLFW/GLFWKeyboard.hpp>
#include <Input/GLFW/GLFWMouse.hpp>
#include <Input/GLFW/GLFWKeyboardCache.hpp>

#include <stdlib.h>
#include <stdio.h>

using namespace ehj;
using namespace glm;

#define SCENETYPE PathTracingScene
class PathTracingScene : IScene {
public:
	FreeFlyCamera m_cam;
	Clock m_clock;

	// to hold references to instances
	std::shared_ptr<ehj::GLFWKeyboard> m_kb;
	std::shared_ptr<ehj::GLFWMouse> m_mouse;
	std::vector<bool> m_kkTap = std::vector<bool>(IBCodes::KK_COUNT,true);

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

static void processInput(GLFWwindow *window) {
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void setup(void) {

};

int run(void)
{
	GLFWwindow* window;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

	window = glfwCreateWindow(640, 480, "ehjE PathTracing", NULL, NULL);
	ehjSetGLFWicon(window);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	
	glfwSetCursorPosCallback(window, m_mouse->mouse_callback);
	glfwSetMouseButtonCallback(window, m_mouse->mouse_button_callback);
	glfwSetKeyCallback(window, m_kb->key_callback);

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval(1);
	glEnable(GL_DEPTH_TEST);
	
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window,true);
	ImGui_ImplOpenGL3_Init("#version 460");


	GLProgram glProg;
	glProg.addSourceFromFile("shaders/PathTracing/ssq.vs");
	ehj_gl_err();
	glProg.addSourceFromFile("shaders/PathTracing/pt.fs");
	ehj_gl_err();

	glProg.createProgram();
	glProg.bind();

	GLProgram glpPP;
	glpPP.addSourceFromFile("shaders/PathTracing/ssq.vs");
	ehj_gl_err();
	glpPP.addSourceFromFile("shaders/PathTracing/pp.fs");
	ehj_gl_err();

	glpPP.createProgram();
	glpPP.bind();
	glProg.bind();
 
	ehj::SSMesh mesh;
	mesh.toTriangles();
	OGLMesh oglMesh(mesh, GL_DYNAMIC_DRAW);
	oglMesh.bind(0);

	ehj_gl_err();
	glBindVertexArray(oglMesh.getVAO());
	ehj_gl_err();
 
	glProg.bind();
	glBindAttribLocation(glProg.getProgramID(),oglMesh.getAttribPos(),"vPos");
	if (oglMesh.getAttribNrm()!=-1)
		glBindAttribLocation(glProg.getProgramID(),oglMesh.getAttribNrm(),"vNrm");
	glpPP.bind();
	glBindAttribLocation(glpPP.getProgramID(),oglMesh.getAttribPos(),"vPos");
	if (oglMesh.getAttribNrm()!=-1)
		glBindAttribLocation(glpPP.getProgramID(),oglMesh.getAttribNrm(),"vNrm");

	glProg.bind();
	glBindVertexArray(oglMesh.getVAO());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oglMesh.getEBO());
	glm::mat4 pvm = glm::ortho(-1.f,1.f,-1.f,1.f);

	m_cam.setTiltable(false);
	m_cam.setPos(vec3(0.,0.,3.));
	m_cam.setDir(vec3(0.,0.,-1.));

	GLFWfpsLimiter fpsLimiter;

	int width, height;
	//glfwGetFramebufferSize(window, &width, &height);
	ivec2 prevRes = {1920,1080};

	GLFrameBuffer fbr1(prevRes);
	GLFrameBuffer fbr2(prevRes);
	bool ping = true;

	GPUTimer gpuTimer;

	float time = 0.;
	int frame = 0;
	while (!glfwWindowShouldClose(window)) {

		{ // reload shader
			static bool suc = true;
			if (GLFWKeyboardCache::keyReleased(IBCodes::KK_KEY_R))
				m_kkTap[IBCodes::KK_KEY_R] = true;
			if (GLFWKeyboardCache::keyPressed(IBCodes::KK_KEY_R) && m_kkTap[IBCodes::KK_KEY_R]) {
				m_kkTap[IBCodes::KK_KEY_R] = false;
				glUseProgram(0);
				//suc = glProg.addSourceFromFileRecursive("myScenes/shader/"+fragShader,GL_FRAGMENT_SHADER);
				suc = glProg.addSourceFromFile("shaders/PathTracing/pt.fs");

				glProg.createProgram();
			
				glBindAttribLocation(glProg.getProgramID(),oglMesh.getAttribPos(),"vPos");
				if (oglMesh.getAttribNrm()!=-1)
					glBindAttribLocation(glProg.getProgramID(),oglMesh.getAttribNrm(),"vNrm");
				ehj_gl_err_continue();
				
				suc &= glpPP.addSourceFromFile("shaders/PathTracing/pp.fs");
				glpPP.createProgram();
				glBindAttribLocation(glpPP.getProgramID(),oglMesh.getAttribPos(),"vPos");
				if (oglMesh.getAttribNrm()!=-1)
					glBindAttribLocation(glpPP.getProgramID(),oglMesh.getAttribNrm(),"vNrm");
				ehj_gl_err_continue();
				frame = 0;
			}
			if (!suc) {
				glfwSwapBuffers(window);
				glfwPollEvents();
				processInput(window); // TODO check esc close window
				fpsLimiter.wait();
				continue;
			}
		}

		gpuTimer.start();

		if (ping) {
			glBindFramebuffer(GL_FRAMEBUFFER,fbr2.getFBO());
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,fbr1.getTexCol());
		} else {//pong
			glBindFramebuffer(GL_FRAMEBUFFER,fbr1.getFBO());
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,fbr2.getTexCol());
		}
		ping = !ping;
		glViewport(0, 0, prevRes.x, prevRes.y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float deltaTime = m_clock.update();
		time += deltaTime;
		frame++;
		m_cam.kbmActive(true);
		m_cam.update(deltaTime);
		m_cam.setProj(glm::perspective(glm::radians(60.0f), 1.f,0.1f,1000.0f));
 
		glm::mat4 m = glm::mat4(1.0f); // identity
		glm::mat4 p = glm::ortho(-1.f,1.f,-1.f,1.f);
		glm::mat4 mvp = p*m;
		glProg.bind();

		static vec3 prevDir;
		static vec3 prevPos;
		if (m_cam.getDir() != prevDir || m_cam.getPos() != prevPos) {
			glUniform1f(glProg.getUnfLoc("u_camChange"), 1.f);
			frame = 0;
		}
		else
			glUniform1f(glProg.getUnfLoc("u_camChange"), 0.f);
		prevDir = m_cam.getDir();
		prevPos = m_cam.getPos();

		glUniform1f(glProg.getUnfLoc("u_time"), time);
		glUniform1f(glProg.getUnfLoc("u_frame"), frame);
		glUniform2f(glProg.getUnfLoc("u_resolution"), prevRes.x,prevRes.y);
		glUniformMatrix4fv(glProg.getUnfLoc("u_pvm"), 1, GL_FALSE, &pvm[0][0]);
		glm::vec3 cPos = m_cam.getPos(); //TODO clear accumulation buffer on cam pos change
		glUniform3f(glProg.getUnfLoc("u_cPos"), cPos.x,cPos.y,cPos.z);
		glm::mat4 camPV = glm::scale(glm::mat4(1.f),glm::vec3(float(width)/height,1.,1.))*m_cam.getPV();
		glUniformMatrix4fv(glProg.getUnfLoc("u_m"),1,GL_TRUE,&camPV[0][0]);

		glDrawElements(GL_TRIANGLES,oglMesh.getEBOsize(),GL_UNSIGNED_INT,0);
 
		glfwGetFramebufferSize(window, &width, &height);
		float ratio;
		ratio = width / (float) height; //TODO uniform

		glBindFramebuffer(GL_FRAMEBUFFER,0);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// accumulate
//		if (ping) {
//			glBlitNamedFramebuffer(fbr2.getFBO(),0,
//				0,0,prevRes.x,prevRes.y,
//				0,0,width,height,
//				 GL_COLOR_BUFFER_BIT,GL_LINEAR);
//		} else {
//			glBlitNamedFramebuffer(fbr1.getFBO(),0,
//				0,0,prevRes.x,prevRes.y,
//				0,0,width,height,
//				 GL_COLOR_BUFFER_BIT,GL_LINEAR);
//		}

		glpPP.bind();
		//PP
		glUniform1f(glpPP.getUnfLoc("u_time"), time);
		glUniform1f(glpPP.getUnfLoc("u_frame"), frame);
		glUniform2f(glpPP.getUnfLoc("u_resolution"), prevRes.x,prevRes.y);
		glUniformMatrix4fv(glpPP.getUnfLoc("u_pvm"), 1, GL_FALSE, &pvm[0][0]);

		if (ping) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,fbr2.getTexCol());
		} else {//pong
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,fbr1.getTexCol());
		}

		glDrawElements(GL_TRIANGLES,oglMesh.getEBOsize(),GL_UNSIGNED_INT,0);

		gpuTimer.end();

		{ // imgui
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			{
				ImGui::Begin("PT");
				float ms1 = gpuTimer.getMS();
				static float ms = 1.;
				ms = ms*.99 + ms1*.01;
				std::string strMS = "MS: " + std::to_string(ms);
				ImGui::Text("%s", strMS.c_str());
				std::string strFPS = "FPS: " + std::to_string(1000./ms);
				ImGui::Text("%s", strFPS.c_str());
				ImGui::End();
			}

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
		processInput(window); // TODO check esc close window
	}
	
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
	return 0;
};

void cleanup() {

}
};//scene
