#include "../src/suOGL.hpp"

//TODO global path
#include "../src/Input/GLFW/GLFWKeyboard.hpp"
#include "../src/Input/GLFW/GLFWMouse.hpp"
#include "../src/Input/GLFW/GLFWKeyboardCache.hpp"

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

//static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
//{
//	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//		glfwSetWindowShouldClose(window, GLFW_TRUE);
//}
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

	window = glfwCreateWindow(640, 480, "PathTracing", NULL, NULL);
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

	GLProgram glProg;
	glProg.addSourceFromFile("shaders/PathTracing/ssq.vs");
	ehj_gl_err();
	glProg.addSourceFromFile("shaders/PathTracing/pt.fs");
	ehj_gl_err();

	glProg.createProgram();
	glProg.bind();
 
	ehj::SSMesh mesh;
	mesh.toTriangles();
	OGLMesh oglMesh(mesh, GL_DYNAMIC_DRAW);
	oglMesh.bind(0);

	ehj_gl_err();
	glBindVertexArray(oglMesh.getVAO());
	ehj_gl_err();
 
	glBindAttribLocation(glProg.getProgramID(),oglMesh.getAttribPos(),"vPos");
	if (oglMesh.getAttribNrm()!=-1)
		glBindAttribLocation(glProg.getProgramID(),oglMesh.getAttribNrm(),"vNrm");

	glBindVertexArray(oglMesh.getVAO());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oglMesh.getEBO());

	glm::mat4 pvm = glm::ortho(-1.f,1.f,-1.f,1.f);
	
	glUniformMatrix4fv(glProg.getUnfLoc("u_pvm"), 1, GL_FALSE, &pvm[0][0]);

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

	float time = 0.;
	int frame = 0;
	while (!glfwWindowShouldClose(window)) {
		float ratio;
		//vec2 res = {width,height};
		//if (prevRes != res) {
		//	fbr1 = GLFrameBuffer(res);
		//	fbr2 = GLFrameBuffer(res);
		//	prevRes = res;
		//}

		if (ping) {
			glBindFramebuffer(GL_FRAMEBUFFER,fbr2.getFBO());
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,fbr1.getTexCol());
			glViewport(0, 0, prevRes.x, prevRes.y);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		} else {//pong
			glBindFramebuffer(GL_FRAMEBUFFER,fbr1.getFBO());
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,fbr2.getTexCol());
			glViewport(0, 0, prevRes.x, prevRes.y);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
		ping = !ping;

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
		float deltaTime = m_clock.update();
		time += deltaTime;
		frame++;
		m_cam.kbmActive(true);
		m_cam.update(deltaTime);
		m_cam.setProj(glm::perspective(glm::radians(60.0f), 1.f,0.1f,1000.0f));
 
		glm::mat4 m = glm::mat4(1.0f); // identity
		glm::mat4 p = glm::ortho(-1.f,1.f,-1.f,1.f);
		glm::mat4 mvp = p*m;

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
		glm::vec3 cPos = m_cam.getPos(); //TODO clear accumulation buffer on cam pos change
		glUniform3f(glProg.getUnfLoc("u_cPos"), cPos.x,cPos.y,cPos.z);
		glm::mat4 camPV = glm::scale(glm::mat4(1.f),glm::vec3(float(width)/height,1.,1.))*m_cam.getPV();
		glUniformMatrix4fv(glProg.getUnfLoc("u_m"),1,GL_TRUE,&camPV[0][0]);

		glProg.bind();
		glDrawElements(GL_TRIANGLES,oglMesh.getEBOsize(),GL_UNSIGNED_INT,0);
 
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float) height; //TODO uniform
 
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// accumulate
		if (ping) {
			glBlitNamedFramebuffer(fbr2.getFBO(),0,
				0,0,prevRes.x,prevRes.y,
				0,0,width,height,
				 GL_COLOR_BUFFER_BIT,GL_LINEAR);
		} else {
			glBlitNamedFramebuffer(fbr1.getFBO(),0,
				0,0,prevRes.x,prevRes.y,
				0,0,width,height,
				 GL_COLOR_BUFFER_BIT,GL_LINEAR);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
		processInput(window); // TODO check esc close window
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
	return 0;
};

void cleanup() {

}
};//scene
