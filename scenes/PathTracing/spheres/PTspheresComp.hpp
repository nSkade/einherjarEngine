#include "GAPI/OGL/GLGPUTimer.hpp"
#include "Input/GLFW/OGL/GLFWImGuiGL.hpp"
#include "suCMN.hpp"
#include <GLFW/glfw3.h>
#include <glm/gtc/epsilon.hpp>
#include <suOGL.hpp>

#include <Input/GLFW/GLFWKeyboard.hpp>
#include <Input/GLFW/GLFWMouse.hpp>
#include <Input/GLFW/GLFWKeyboardCache.hpp>


using namespace ehj;
using namespace glm;

#define SCENETYPE PTspheresCompScene
class PTspheresCompScene : IScene {
public:
void setup(void) {
	m_glWindow.setup(&m_configFile,"ehjE PTspheresComp");
};

int run(void) {
	GLFWImGuiGL::init(m_glWindow.m_pWindow);

	GLProgram glpPT; 
	auto glpPTbuild = [&]() -> bool {
		//bool r = glpPT.addSourceFromFile("shaders/ssq.vs");
		bool r = glpPT.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"pt.comp");
		glpPT.createProgram();
		return r;
	};
	glpPTbuild();

	GLProgram glpPP;
	auto glpPPbuild = [&]() -> bool {
		bool r= glpPP.addSourceFromFile("shaders/ssq.vs");
		r &= glpPP.addSourceFromFile(EHJ_THIS_FOLDER()+"../pp.fs");
		glpPP.createProgram();
		return r;
	};
	glpPPbuild();
 
	ehj::SSMesh ssm; {
		ssm.toTriangles();
		ssm.assembleVertexBuffer();
	}
	GLVertexBuffer ssmGlVb(ssm.m_vertexData); ssmGlVb.bind(0);
	GLMesh ssmGl(ssm, GL_DYNAMIC_DRAW); ssmGl.bind();

	m_cam.setTiltable(false);
	m_cam.setPos(vec3(0.,0.,3.));
	m_cam.setDir(vec3(0.,0.,-1.));

	int width, height;
	width=m_glWindow.m_winRes.x;
	height=m_glWindow.m_winRes.y;
	ivec2 prevRes = {width,height};

	GLFrameBuffer fbr1(prevRes);
	GLFrameBuffer fbr2(prevRes);
	bool ping = true;

	GLGPUTimer gpuTimer;

	float time = 0.;
	int frame = 0;

	glfwSwapInterval(1);

	while (m_glWindow.stillOpen()) {

		{ // reload shader
			static bool suc = true;
			if (GLFWKeyboardCache::keyReleased(IBCodes::KK_KEY_R))
				m_kkTap[IBCodes::KK_KEY_R] = true;
			if (GLFWKeyboardCache::keyPressed(IBCodes::KK_KEY_R) && m_kkTap[IBCodes::KK_KEY_R]) {
				m_kkTap[IBCodes::KK_KEY_R] = false;
				//glUseProgram(0);
				suc = glpPTbuild();
				suc &= glpPPbuild();
				frame = 0;
			}
			if (!suc) {
				m_glWindow.swapBuffers();
				m_glWindow.pollInput();
				continue;
			}
		}
 
		glfwGetFramebufferSize(m_glWindow.m_pWindow, &width, &height);
		float ratio;
		ratio = width / (float) height; //TODO uniform

		gpuTimer.start();

		float deltaTime = m_clock.update();
		time += deltaTime;
		frame++;
		m_cam.kbmActive(true);
		m_cam.update(deltaTime);
		m_cam.setProj(glm::perspective(glm::radians(60.0f), 1.f,0.1f,1000.0f));
			
		glm::mat4 pvm = glm::ortho(-1.f,1.f,-1.f,1.f);

		{ // PT
			if (ping) {
				//glBindFramebuffer(GL_FRAMEBUFFER,fbr2.getFBO());
				glBindImageTexture(0, fbr2.getTexCol(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
				
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D,fbr1.getTexCol());
			} else {//pong
				//glBindFramebuffer(GL_FRAMEBUFFER,fbr1.getFBO());
				glBindImageTexture(0, fbr1.getTexCol(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D,fbr2.getTexCol());
			}
			ping = !ping;
			glViewport(0, 0, prevRes.x, prevRes.y);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glpPT.bind();

			static vec3 prevDir;
			static vec3 prevPos;
			if (!all(epsilonEqual(m_cam.getDir(),prevDir,epsilon<float>()*2.f))
				|| !all(epsilonEqual(m_cam.getPos(),prevPos,epsilon<float>()*2.f))) {
				glUniform1f(glpPT.getUnfLoc("u_camChange"), 1.f);
				frame = 0;
			}
			else
				glUniform1f(glpPT.getUnfLoc("u_camChange"), 0.f);
			prevDir = m_cam.getDir();
			prevPos = m_cam.getPos();

			glUniform1f(glpPT.getUnfLoc("u_time"), time);
			glUniform1f(glpPT.getUnfLoc("u_frame"), frame);
			glUniform2f(glpPT.getUnfLoc("u_resolution"), prevRes.x,prevRes.y);
			
			glUniformMatrix4fv(glpPT.getUnfLoc("u_pvm"), 1, GL_FALSE, &pvm[0][0]);
			glm::vec3 cPos = m_cam.getPos(); //TODO clear accumulation buffer on cam pos change
			glUniform3f(glpPT.getUnfLoc("u_cPos"), cPos.x,cPos.y,cPos.z);
			glm::mat4 camPV = glm::scale(glm::mat4(1.f),glm::vec3(float(width)/height,1.,1.))*m_cam.getPV();
			glUniformMatrix4fv(glpPT.getUnfLoc("u_m"),1,GL_TRUE,&camPV[0][0]);

			//ssmGl.draw();

			{ // dispatch compute thread for each pixel
				ivec3 ls; // local work group size
				glGetProgramiv(glpPT.getID(), GL_COMPUTE_WORK_GROUP_SIZE, &ls[0]);
				ivec3 ds = (ivec3(width, height, 1) + ls - ivec3(1)) / ls;  // dispatch size
				glDispatchCompute(ds[0],ds[1],ds[2]);
				glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
			}
		}

		{ //PP
			glBindFramebuffer(GL_FRAMEBUFFER,0);
			glViewport(0, 0, width, height);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glpPP.bind();
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

			ssmGl.draw();
		}

		gpuTimer.end();

		{ // imgui
			GLFWImGuiGL::newFrame();

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

			GLFWImGuiGL::render();
		}

		m_glWindow.swapBuffers();
		m_glWindow.pollInput();
	}
	return 0;
};

void cleanup() {
	m_glWindow.close(&m_configFile);

	m_configFile.baseStore();

	ehj::GLFWImGuiGL::shutdown();
}

private:
	GLFWWindowGL m_glWindow;
	ConfigFile m_configFile;

	std::shared_ptr<ehj::GLFWKeyboard> m_kb;
	std::shared_ptr<ehj::GLFWMouse> m_mouse;
	std::vector<bool> m_kkTap = std::vector<bool>(IBCodes::KK_COUNT,true);
	
	FreeFlyCamera m_cam;
	Clock m_clock;
};//scene
