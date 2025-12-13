#include "GAPI/OGL/GLProgram.hpp"
#include "GAPI/OGL/GLUtils.hpp"
#include <suOGL.hpp>

#include <Input/GLFW/GLFWKeyboard.hpp>
#include <Input/GLFW/GLFWMouse.hpp>
#include <Input/GLFW/GLFWKeyboardCache.hpp>
#include <Input/GLFW/GLFWMouseCache.hpp>

#include <stdlib.h>
#include <stdio.h>

#include "JFA.hpp"

using namespace ehj;
using namespace glm;

#define SCENETYPE RCnaiveScene
class RCnaiveScene : IScene {
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

	int width = 640, height = 480;
	window = glfwCreateWindow(width, height, "ehjE RCnaive", NULL, NULL);
#if 0
	width = 1920, height = 1027;
	glfwSetWindowSize(window,width,height);
	glfwSetWindowPos(window,0,23);
#endif
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
 
	ehj::SSMesh mesh;
	mesh.toTriangles();
	GLMesh glMesh(mesh, GL_DYNAMIC_DRAW);
	glMesh.bind(0);

	ehj_gl_err();
	glBindVertexArray(glMesh.getVAO());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glMesh.getEBO());

	auto createPass = [&](GLProgram& glp, std::string vs, std::string fs) {
		glp.addSourceFromFile(vs); //TODO make abstraction to make reloading easier
		ehj_gl_err();
		glp.addSourceFromFile(fs);
		ehj_gl_err();
		glp.createProgram();
		glBindAttribLocation(glp.getID(),glMesh.getAttribPos(),"vPos");
		if (glMesh.getAttribNrm()!=-1)
			glBindAttribLocation(glp.getID(),glMesh.getAttribNrm(),"vNrm");
		ehj_gl_err();
	};
	GLProgram glpPencil;
	createPass(glpPencil,
		"scenes/RC2D/ssq.vs",
		"scenes/RC2D/pencil.fs"
	);
	
	GLProgram glpDO; // dynamic objects
	createPass(glpDO,
		"scenes/RC2D/ssq.vs",
		"scenes/RC2D/dynamicObj.fs"
	);
	
	//glfwGetFramebufferSize(window, &width, &height);
	ivec2 prevRes = {width,height};
	
	JFA jfa(prevRes,glMesh);
	//GLProgram glpJumpFlood;
	//createPass(glpJumpFlood,
	//	"scenes/RC2D/ssq.vs",
	//	"scenes/RC2D/jumpflood.fs"
	//);

	GLProgram glpRCnaive;
	createPass(glpRCnaive,
		"scenes/RC2D/ssq.vs",
		"scenes/RC2D/naive.fs"
	);

	glm::mat4 pvm = glm::ortho(-1.f,1.f,-1.f,1.f);

	m_cam.setTiltable(false);
	m_cam.setPos(vec3(0.,0.,3.));
	m_cam.setDir(vec3(0.,0.,-1.));

	GLFWfpsLimiter fpsLimiter;

	GLFrameBuffer fbr1(prevRes);
	GLFrameBuffer fbr2(prevRes);
	//GLFrameBuffer fbJumpFlood(prevRes);
	//GLFrameBuffer fbJumpFlood2(prevRes);

	float time = 0.;
	int frame = 0;
	
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window,true);
	ImGui_ImplOpenGL3_Init("#version 460");

	float widthPrev = width, heightPrev=height;

	float pencilSize = .02;
	vec4 pencilColor = vec4(1.,0.,0.,1.);

	int rayCount = 16;
	int raySteps = 16;
	float rayNoise = 1.;
	float rayDist = 1.;
	//int jfPassCount = 11;
	float lightStr = 1.;

	enum ViewPass {
		VP_NRM,
		VP_JFA,
		VP_COUNT,
	};
	int viewPass = VP_NRM;
	const char* viewPassStr[] = {
		"NRM",
		"JFA",
	};

	vec2 mouse = vec2(0.,0.);

	while (!glfwWindowShouldClose(window)) {
		
		bool hoveredImgui = ImGui::IsAnyItemHovered() || ImGui::IsAnyItemActive() || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
		glfwGetFramebufferSize(window, &width, &height);

		float deltaTime = m_clock.update();
		m_cam.kbmActive(true);
		m_cam.update(deltaTime);
		m_cam.setProj(glm::perspective(glm::radians(60.0f), 1.f,0.1f,1000.0f));
		//glm::mat4 m = glm::mat4(1.0f); // identity
		//glm::mat4 p = glm::ortho(-1.f,1.f,-1.f,1.f);
		//glm::mat4 mvp = p*m;

		vec2 mousePrev = mouse;
		mouse = vec2(GLFWMouseCache::getXPos(),height-GLFWMouseCache::getYPos());

		{ // reload shader
			static bool suc = true;
			if (GLFWKeyboardCache::keyReleased(IBCodes::KK_KEY_R))
				m_kkTap[IBCodes::KK_KEY_R] = true;
			if (GLFWKeyboardCache::keyPressed(IBCodes::KK_KEY_R) && m_kkTap[IBCodes::KK_KEY_R]) {
				bool sucTmp = true;
				m_kkTap[IBCodes::KK_KEY_R] = false;
				glUseProgram(0);

				auto reloadPass = [&](GLProgram& glp, std::string fs) {
					suc &= glp.addSourceFromFile(fs);
					glp.createProgram();
					glBindAttribLocation(glp.getID(),glMesh.getAttribPos(),"vPos");
					if (glMesh.getAttribNrm()!=-1)
						glBindAttribLocation(glp.getID(),glMesh.getAttribNrm(),"vNrm");
					ehj_gl_err_continue();
				};

				reloadPass(glpPencil,"scenes/RC2D/pencil.fs");
				reloadPass(glpDO,"scenes/RC2D/dynamicObj.fs");
				reloadPass(glpRCnaive,"scenes/RC2D/naive.fs");
				//reloadPass(glpJumpFlood,"scenes/RC2D/jumpflood.fs");
				
				frame = 0;
				suc = sucTmp;
			}
			if (!suc) {
				glfwSwapBuffers(window);
				glfwPollEvents();
				processInput(window); // TODO check esc close window
				fpsLimiter.wait();
				continue;
			}
		}

		auto setCMNuniforms = [&](GLProgram& glp) {
			glUniform1f(glp.getUnfLoc("u_time"), time);
			glUniform1f(glp.getUnfLoc("u_frame"), frame);
			glUniform2f(glp.getUnfLoc("u_resolution"), width,height);
			if (hoveredImgui)
				glUniform1i(glp.getUnfLoc("u_mbd"), 0);
			else
				glUniform1i(glp.getUnfLoc("u_mbd"), (int) GLFWMouseCache::keyPressed(IBCodes::MB_BUTTON_LEFT));
			glUniform2fv(glp.getUnfLoc("u_mouse"), 1, &mouse[0]);
			glUniform2fv(glp.getUnfLoc("u_mousePrev"), 1, &mousePrev[0]);
			//ehj_gl_err();
			glUniformMatrix4fv(glp.getUnfLoc("u_pvm"), 1, GL_FALSE, &pvm[0][0]);
			glUniform1f(glp.getUnfLoc("u_pencilSize"),pencilSize);
			glUniform4fv(glp.getUnfLoc("u_pencilColor"),1,&pencilColor[0]);
			glUniform1i(glp.getUnfLoc("u_rayCount"),rayCount);
			glUniform1i(glp.getUnfLoc("u_raySteps"),raySteps);
			glUniform1f(glp.getUnfLoc("u_rayNoise"),rayNoise);
			glUniform1f(glp.getUnfLoc("u_rayDist"),rayDist);
			glUniform1i(glp.getUnfLoc("u_viewPass"),viewPass);
			glUniform1f(glp.getUnfLoc("u_lightStr"),lightStr);
		};

		{ // draw pencil
			glBindFramebuffer(GL_FRAMEBUFFER,fbr1.getFBO());
			glViewport(0, 0, width, height);
			if (widthPrev != width || height != heightPrev) {
				frame = 0;
				fbr1 = GLFrameBuffer(ivec2(width,height));
				fbr2 = GLFrameBuffer(ivec2(width,height));
				//glClearColor(0.,0.,0.,0.);
				//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			} else {
				glClear(GL_DEPTH_BUFFER_BIT);
			}
			glpPencil.bind();
			setCMNuniforms(glpPencil);
			glDrawElements(GL_TRIANGLES,glMesh.getEBOsize(),GL_UNSIGNED_INT,0);
		}

		if (1) { // draw temporary elements / dynamic objects
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,fbr1.getTexCol());
			glBindFramebuffer(GL_FRAMEBUFFER,fbr2.getFBO());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glpDO.bind();
			setCMNuniforms(glpDO);
			glDrawElements(GL_TRIANGLES,glMesh.getEBOsize(),GL_UNSIGNED_INT,0);
		}

		if (widthPrev != width || height != heightPrev)
			jfa.updateBufferSize(ivec2(width,height));
		jfa.passJFA(glMesh,fbr2,pvm);

		{ // RC naive pass
			glBindFramebuffer(GL_FRAMEBUFFER,0);
			glViewport(0, 0, width, height);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glpRCnaive.bind();
			setCMNuniforms(glpRCnaive);
			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,fbr2.getTexCol());
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D,jfa.m_fb1.getTexCol());

			//TODO abstract this into GLProgram
			glUniform1i(glGetUniformLocation(glpRCnaive.getID(), "u_tex"), 0);           // texture unit 0
			glUniform1i(glGetUniformLocation(glpRCnaive.getID(), "u_texJumpFlood"), 1);  // texture unit 1

			glDrawElements(GL_TRIANGLES,glMesh.getEBOsize(),GL_UNSIGNED_INT,0);
		}
		
		{ // imgui
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			{
				ImGui::Begin("RCnaive");
				ImGui::DragFloat("pencilSize",&pencilSize,0.001,0.001,10.);
				ImGui::ColorEdit4("pencilColor",&pencilColor[0]);
				ImGui::DragInt("rayCount",&rayCount,1,1,100);
				ImGui::DragInt("raySteps",&raySteps,1,1,100);
				ImGui::DragFloat("rayNoise",&rayNoise,0.001,0.,1.);
				ImGui::DragFloat("rayDist",&rayDist,0.001,0.,1.);
				ImGui::SliderInt("jfPassCountMod",&jfa.m_jfPassCount,0,jfa.getMaxJfPassCount());
				ImGui::DragFloat("lightStr",&lightStr,0.001,0.,10.);
				ImGui::Separator();
				{
					ImGui::Combo("view pass",&viewPass,viewPassStr,VP_COUNT);
				}
				ImGui::End();
			}

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}

		ehj_gl_err();
		glfwSwapBuffers(window);
		glfwPollEvents();
		processInput(window); // TODO check esc close window
		ehj_gl_err();
		
		time += deltaTime;
		frame++;
		
		widthPrev = width;
		heightPrev = height;
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
