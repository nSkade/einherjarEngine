
#include <glm/ext/matrix_transform.hpp>
#include <suOGL.hpp>

#include <Input/GLFW/GLFWKeyboardCache.hpp>
#include <Utility/ConfigFile.hpp>

#include <thread>

#include "GLEntity.hpp"

using namespace ehj;

#define SCENETYPE ShadowMapTestScene
class ShadowMapTestScene : IScene {
public:
	~ShadowMapTestScene() {}
	void setup() {
		m_glWindow.setup(&m_configFile, "ehjE ShadowMapTestScene");
	}

	int run() {

		GLEntity sponza;
		sponza.load("myModels/sponza/gltf/Sponza.gltf");
		
		GLEntity glEmonkey;
		//glEmonkey.load("myModels/monkeyTex/monkeyTex.gltf");
		
		GLProgram glp;
		glp.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"v.vert");
		glp.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"f_shadowMap.frag");
		glp.createProgram();
		glp.bind();
		
		GLProgram glpShadowPass;
		glpShadowPass.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"shadowPass_v.vert");
		glpShadowPass.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"shadowPass_f.frag");
		glpShadowPass.createProgram();
		glpShadowPass.bind();

		GLFrameBuffer::Opt glFBshadowOpt;
		glFBshadowOpt.res = ivec2(1024*4,1024*4);
		GLFrameBuffer glFBshadow(glFBshadowOpt);

		GLGPUTimer fragSTimer;

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		float time = 0.0f;

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		m_cam.setPos({0.,0.,0.});

		glfwSwapInterval(0);
		GLFWfpsLimiter m_fpsLimiter;
		
		while (m_glWindow.stillOpen()) {
			{ // reload shader
				static bool suc = true;
				if (GLFWKeyboardCache::keyReleased(IBCodes::KK_KEY_R))
					m_kkTap[IBCodes::KK_KEY_R] = true;
				if (GLFWKeyboardCache::keyPressed(IBCodes::KK_KEY_R) && m_kkTap[IBCodes::KK_KEY_R]) {
					bool sucTmp = true;
					m_kkTap[IBCodes::KK_KEY_R] = false;
					glUseProgram(0);
					
					suc &= glp.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"v.vert");
					suc &= glp.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"f_shadowMap.frag");
					glp.createProgram();

					suc = sucTmp;
				}
				if (!suc) {
					m_fpsLimiter.wait();
					m_glWindow.pollInput(); // TODO check esc close window
					continue;
				}
			}

			float deltaTime = m_clock.update();
			time += deltaTime;
			
			m_fpsLimiter.wait();
			m_glWindow.pollInput(); //TODO polling should happen during wait?
			
			m_cam.update(deltaTime);

			mat4 shadowMat;
			if (1) { // shadow pass
				ivec2 shadowRes = glFBshadow.getRes();
				glViewport(0, 0,shadowRes.x,shadowRes.y);
				glBindFramebuffer(GL_FRAMEBUFFER,glFBshadow.getFBO());
				glClear(GL_DEPTH_BUFFER_BIT);
				glpShadowPass.bind();

				static mat4 p=glm::ortho(-1.f,1.f,-1.f,1.f);
				static mat4 v=glm::rotate(glm::identity<mat4>(),3.141592f*.5f, glm::vec3(0.f,1.f,0.f));
				if (GLFWKeyboardCache::keyPressed(IBCodes::KK_KEY_E) && m_kkTap[IBCodes::KK_KEY_E]) {
					p=m_cam.getProj();
					v=m_cam.getView();
				}

				mat4 m=glm::scale(mat4(1.),vec3(0.005));

				shadowMat=p*v;
				
				glUniformMatrix4fv(glpShadowPass.getUnfLoc("u_p"), 1, GL_FALSE, &p[0][0]);
				glUniformMatrix4fv(glpShadowPass.getUnfLoc("u_v"), 1, GL_FALSE, &v[0][0]);
				glUniformMatrix4fv(glpShadowPass.getUnfLoc("u_m"), 1, GL_FALSE, &m[0][0]);
				
				sponza.draw();
			}

			if (1) { // forward pass
				glp.bind();
				glBindFramebuffer(GL_FRAMEBUFFER,0);
				
				int width, height;
				glfwGetFramebufferSize(m_glWindow.m_pWindow, &width, &height);
				auto& m_winRes=m_glWindow.m_winRes;
				m_winRes = glm::ivec2(width,height);
				glViewport(0,0,width,height);
				
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				m_cam.setProj(glm::perspective(glm::radians(90.0f), std::fmax(0.00001f,(float)m_winRes.x/(float)m_winRes.y),0.01f,100.0f));
				mat4 m=glm::scale(mat4(1.),vec3(0.005));
				//mat4 m=glm::scale(mat4(1.),vec3(0.5));

				//mat4 pvm = m_cam.getPV()*m;
				mat4 p=m_cam.getProj();
				mat4 v=m_cam.getView();
			
				glUniformMatrix4fv(glp.getUnfLoc("u_p"), 1, GL_FALSE, &p[0][0]);
				glUniformMatrix4fv(glp.getUnfLoc("u_v"), 1, GL_FALSE, &v[0][0]);
				glUniformMatrix4fv(glp.getUnfLoc("u_m"), 1, GL_FALSE, &m[0][0]);
				//glUniformMatrix4fv(glp.getUnfLoc("u_pvm"), 1, GL_FALSE, &pvm[0][0]);

				// required for normal mapping so normals are scale invariant
				mat4 u_imtn = glm::transpose(glm::inverse(m));
				glUniformMatrix4fv(glp.getUnfLoc("u_imtn"), 1, GL_FALSE, &u_imtn[0][0]);

				glUniform1f(glp.getUnfLoc("u_time"), time);
				glUniform2f(glp.getUnfLoc("u_resolution"), width, height);
				glUniformMatrix4fv(glp.getUnfLoc("u_shadowMat"), 1, GL_FALSE, &shadowMat[0][0]);
				
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D,glFBshadow.getTexDep());
		
				fragSTimer.start();
					sponza.draw();
				fragSTimer.end();
				
				if (0) { // monkey
					mat4 m2=glm::scale(mat4(1.),vec3(.5));
					m2 = glm::rotate(m2,time*.1f, glm::vec3(0.f,1.f,0.f));
					m2 = glm::translate(m2,glm::vec3(0.f,1.f,0.f));
					glUniformMatrix4fv(glp.getUnfLoc("u_m"), 1, GL_FALSE, &m2[0][0]);
					//glEmonkey.draw();
				}
			}

			//fragSTimer.print();

			m_glWindow.swapBuffers();
		}
		
		return 0;
	}
	void cleanup() {
		m_glWindow.close(&m_configFile);

		m_configFile.baseStore();

		ImGui::DestroyContext();
	}
private:
	GLFWWindowGL m_glWindow;
	ConfigFile m_configFile;
	std::vector<bool> m_kkTap = std::vector<bool>(IBCodes::KK_COUNT,true);
	
	//bool m_winMaximized;

	FreeFlyCamera m_cam;
	Clock m_clock;
};
