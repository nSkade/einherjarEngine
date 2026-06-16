#include <algorithm>
#include <memory>
#include <suOGL.hpp>

#include <Input/GLFW/GLFWKeyboardCache.hpp>
#include <Input/GLFW/GLFWMouseCache.hpp>

#include "../utils/JFA2D.hpp"
#include "../utils/DrawBoard.hpp"
#include "Input/GLFW/OGL/GLFWImGuiGL.hpp"
#include "suCMN.hpp"

#include <Utility/ConfigFile.hpp>

using namespace ehj;
using namespace glm;

#define SCENETYPE RCScene
class RCScene : IScene {
public:
	ConfigFile m_configFile;
	GLFWWindowGL m_window;
	Clock m_clock;

	std::vector<bool> m_kkTap = std::vector<bool>(IBCodes::KK_COUNT,true);

void setup(void) {
	m_configFile.baseLoad();
	m_window.setup(&m_configFile,"ehjE RC 2D");
};

int run(void) {
	ehj::SSMesh mesh;
	mesh.toTriangles();
	mesh.assembleVertexBuffer();
	GLMesh glMesh(mesh, GL_DYNAMIC_DRAW);
	GLVertexBuffer glVB(mesh.m_vertexData);
	glMesh.bind();

	glVB.bind(0);
	
	int width=m_window.m_winRes[0], height=m_window.m_winRes[1];
	ivec2 prevRes = {width,height};
	JFA2D jfa(prevRes,glVB);
	DrawBoard drawBoard(prevRes,glVB);

	GLProgram glpRC;
	glpRC.createPass(EHJ_THIS_FOLDER()+"../utils/ssq.vs",EHJ_THIS_FOLDER()+"rc.fs");

	glm::mat4 pvm = glm::ortho(-1.f,1.f,-1.f,1.f);

	GLFWfpsLimiter fpsLimiter;

	GLFrameBuffer::Opt fbOpt {prevRes,GL_RGBA8,GL_LINEAR};
	
	fbOpt = GLFrameBuffer::Opt{prevRes,GL_RGBA8,GL_LINEAR};
#if 0
	GLFrameBuffer fbrc1(fbOpt);
	GLFrameBuffer fbrc2(fbOpt);
#else
	std::vector<std::unique_ptr<GLFrameBuffer>> fbrc;
	for (int i=0;i<7;++i)
		fbrc.emplace_back(std::make_unique<GLFrameBuffer>(fbOpt));
#endif

	float time = 0.;
	int frame = 0;
	
	GLFWImGuiGL::init(m_window.m_pWindow);

	float widthPrev = width, heightPrev=height;

	bool linearFilter = true;
	bool linearFilterPrev = true;

	int rayCount = 4;
	int raySteps = 100;
	float rayNoise = 1.;
	float rayDist = 1.;
	float lightStr = 1.;
	
	int cascadeCount = 5;
	int viewCascade = 0;
	
	float rayOverlap = 0.;

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
	GLGPUTimer gpuTimer;

	while (m_window.stillOpen()) {
		
		//gpuTimer.start();

		bool hoveredImgui = ImGui::IsAnyItemHovered() || ImGui::IsAnyItemActive() || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
		glfwGetFramebufferSize(m_window.m_pWindow, &width, &height);
		width = max(1,width);
		height = max(1,height);

		float deltaTime = m_clock.update();

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
				};

				reloadPass(glpRC,EHJ_THIS_FOLDER()+"rc.fs");

				suc = sucTmp;
			}
			if (!suc) {
				m_window.swapBuffers();
				fpsLimiter.wait();
				m_window.pollInput();
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
			glUniformMatrix4fv(glp.getUnfLoc("u_pvm"), 1, GL_FALSE, &pvm[0][0]);
		};
		
		auto setRCuniforms = [&](GLProgram& glp) {
			glUniform1i(glp.getUnfLoc("u_baseRayCount"),rayCount);
			glUniform1i(glp.getUnfLoc("u_raySteps"),raySteps);
			glUniform1f(glp.getUnfLoc("u_rayNoise"),rayNoise);
			glUniform1f(glp.getUnfLoc("u_rayDist"),rayDist);
			glUniform1i(glp.getUnfLoc("u_viewPass"),viewPass);
			glUniform1f(glp.getUnfLoc("u_lightStr"),lightStr);
			glUniform1i(glp.getUnfLoc("u_cascadeCount"),cascadeCount);
			glUniform1i(glp.getUnfLoc("u_viewCascade"),viewCascade);
		};
		
		if (widthPrev != width || height != heightPrev)
			frame = 0;

		gpuTimer.start();
		//TODO DrawScene
		if (widthPrev != width || height != heightPrev)
			drawBoard.updateBufferSize(ivec2(width,height));
		drawBoard.drawPencil(glMesh,{width,height},deltaTime,setCMNuniforms);
		gpuTimer.end();
		double gt_drawScene = gpuTimer.getMS();
	
		gpuTimer.start();
		if (widthPrev != width || height != heightPrev) {
			ivec2 size=ivec2(width,height)/4;
			size=glm::max(size,ivec2(1,1));
			jfa.updateBufferSize(size);
		}
		jfa.passJFA(glMesh,drawBoard.m_fb2,pvm);
		gpuTimer.end();
		double gt_jfa = gpuTimer.getMS();
		std::vector<double> gt_rc;

		{ // RC 2D pass, TODO test upscale with FSR2
			if (widthPrev != width || height != heightPrev || linearFilter != linearFilterPrev) {
#define RCHR 1 // rc halfres
#if RCHR
				GLFrameBuffer::Opt fbOpt {ivec2(width,height)/2,GL_RGBA8};
#else
				GLFrameBuffer::Opt fbOpt {ivec2(width,height),GL_RGBA8};
#endif
				fbOpt.texturefilter = linearFilter ? GL_LINEAR : GL_NEAREST;
#if 0
				fbrc1 = GLFrameBuffer(fbOpt);
				fbrc2 = GLFrameBuffer(fbOpt);
#else
				for (int i=0;i<7;++i)
					fbrc[i] = std::make_unique<GLFrameBuffer>(fbOpt);
#endif
			}

			glViewport(0, 0, width, height);

//TODO only clear after x frames test, not needed -> remove
//#if 0
//			glBindFramebuffer(GL_FRAMEBUFFER,fbrc1.getFBO());
//			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//			glBindFramebuffer(GL_FRAMEBUFFER,fbrc2.getFBO());
//			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//#else
//			for (int i=0;i<7;++i) {
//				//if (frame%(i*1+1)==0)
//				//if (frame%(max(0,i-1)+1)==0)
//				{
//					glBindFramebuffer(GL_FRAMEBUFFER,fbrc[i].getFBO());
//					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//				}
//			}
//			//else {
//			//	glBindFramebuffer(GL_FRAMEBUFFER,fbrc[0].getFBO());
//			//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//			//}
//#endif

			glpRC.bind();
			setCMNuniforms(glpRC);
			setRCuniforms(glpRC);
			drawBoard.setPencilUniforms(glpRC);
#if RCHR
			glUniform2f(glpRC.getUnfLoc("u_resolution"), float(width)/2,float(height)/2);
#else
			glUniform2f(glpRC.getUnfLoc("u_resolution"), width,height);
#endif

			// already bound glBindFramebuffer(GL_FRAMEBUFFER,fbRC.getFBO());
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,drawBoard.m_fb2.getTexCol());
			glUniform1i(glGetUniformLocation(glpRC.getID(), "u_tex"), 0);           // texture unit 0
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D,jfa.m_fb1.getTexCol());
			glUniform1i(glGetUniformLocation(glpRC.getID(), "u_texJumpFlood"), 1);  // texture unit 1
			// render uv
			//glUniform1f(glpRC.getUnfLoc("u_jfOffset"),0.);
			
			gt_rc.clear();
			for (int i=cascadeCount-1;i>=0;--i) {
				gpuTimer.start();
				//if (frame%(i*1+1)==0)
				//if (frame%(max(0,i-1)+1)==0)
				{
			//for (int i=0;i<cascadeCount;++i) {
#if 0
				auto* fbFrom = &fbrc1;
				auto* fbTo = &fbrc2;
				if (i%2!=cascadeCount % 2)
					std::swap(fbFrom,fbTo);
#else
				auto& fbFrom = fbrc[i+1];
				auto& fbTo = fbrc[i];
#endif
				//if (i%2==1)
				//	std::swap(fbFrom,fbTo);
				
				glBindFramebuffer(GL_FRAMEBUFFER,fbTo->getFBO());
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				
				glUniform1i(glpRC.getUnfLoc("u_cascade"),i);
				glUniform1i(glpRC.getUnfLoc("u_cascadeCount"),cascadeCount);
				glUniform1f(glpRC.getUnfLoc("u_overlap"),rayOverlap);
				
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D,fbFrom->getTexCol());
				glUniform1i(glGetUniformLocation(glpRC.getID(), "u_texPrev"), 2);           // texture unit 0

				glDrawElements(GL_TRIANGLES,glMesh.getEBOsize(),GL_UNSIGNED_INT,0);
				
				gpuTimer.end();
				gt_rc.push_back(gpuTimer.getMS());
				}
			}
	
#if 0
			if (cascadeCount % 2 == 0) { //TODO improve eff
				glBlitNamedFramebuffer(fbrc2.getFBO(),fbrc1.getFBO(),
					0,0,width,height,0,0,width,height,
					GL_COLOR_BUFFER_BIT,GL_NEAREST);
			}
#else
#endif
	
			//TODO combine drawboard with 

			{ // blit rc result into buf 0 
				glBindFramebuffer(GL_FRAMEBUFFER,0);
				glViewport(0, 0, width, height);
				//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // shouldnt be required
#if 0
				glBlitNamedFramebuffer(fbrc1.getFBO(),0,
					0,0,width/2,height/2,0,0,width,height,
					GL_COLOR_BUFFER_BIT,GL_NEAREST);
#else
				glBlitNamedFramebuffer(fbrc[0]->getFBO(),0,
#if RCHR
					0,0,width/2,height/2,0,0,width,height,
#else
					0,0,width,height,0,0,width,height,
#endif
					GL_COLOR_BUFFER_BIT,GL_NEAREST);
#endif
			}
		}

		//gpuTimer.end();
		
		{ // imgui
			linearFilterPrev = linearFilter;
			
			GLFWImGuiGL::newFrame();
			{
				ImGui::Begin("RC 2D");
				//float ms1 = gpuTimer.getMS();
				//static float ms = 1.;
				//ms = ms*.5 + ms1*.5;
				//std::string strMS = "MS: " + std::to_string(ms);
				//ImGui::Text("%s", strMS.c_str());
				//std::string strFPS = "FPS: " + std::to_string(1000./ms);
				//ImGui::Text("%s", strFPS.c_str());

				drawBoard.imgui();

				ImGui::DragInt("rayCount",&rayCount,1,1,16);
				ImGui::DragInt("raySteps",&raySteps,1,1,100);
				ImGui::DragFloat("rayNoise",&rayNoise,0.001,0.,1.);
				ImGui::DragFloat("rayDist",&rayDist,0.001,0.,1.);
				ImGui::SliderInt("jfPassCountMod",&jfa.m_jfPassCount,0,jfa.getMaxJfPassCount()+1);
				ImGui::DragFloat("lightStr",&lightStr,0.001);
				ImGui::Separator();
				{
					ImGui::Combo("view pass",&viewPass,viewPassStr,VP_COUNT);
				}
				ImGui::SliderInt("cascade Count",&cascadeCount,0,6);
				ImGui::SliderInt("viewCascade",&viewCascade,0,cascadeCount);
				ImGui::Checkbox("linear texture filter",&linearFilter);

				ImGui::DragFloat("rayOverlap",&rayOverlap,0.001);
				
				{
					auto printGT = [](double v, std::string n, double& s) {
						s = s*.5 + v*.5;
						std::string strMS = n + ": " + std::to_string(s);
						ImGui::Text("%s", strMS.c_str());
					};
					static double gt_dss= 1.;
					printGT(gt_drawScene,"gt_ds",gt_dss);
					static double gt_jfas = 1.;
					printGT(gt_jfa,"gt_jfa",gt_jfas);
					static std::vector<double> gt_rcs;
					while (gt_rcs.size() < gt_rc.size())
						gt_rcs.push_back(1.);
					{
						for (int i=0;i<gt_rc.size();++i)
							printGT(gt_rc[i],"gt_rc"+std::to_string(i),gt_rcs[i]);
					}
				}
				ImGui::End();
			}

			GLFWImGuiGL::render();
		}


		m_window.swapBuffers();
		m_window.pollInput();
		
		time += deltaTime;
		frame++;
		
		widthPrev = width;
		heightPrev = height;
	}
	
	ivec2 winRes=ivec2(width,height);
	ivec2 winPos;
	bool winMaximized=false;
	glfwGetWindowPos(m_window.m_pWindow, &winPos.x,&winPos.y);

	m_configFile.store("winRes",winRes);
	m_configFile.store("winPos",winPos);
	m_configFile.store("winMaximized",winMaximized);
	m_configFile.baseStore();
	return 0;
};

void cleanup() {
	GLFWImGuiGL::shutdown();
	m_window.close();
}
};//scene
