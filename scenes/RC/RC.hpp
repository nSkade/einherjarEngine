#include "GAPI/OGL/GLProgram.hpp"
#include "GAPI/OGL/GLUtils.hpp"
#include <string>
#include <suOGL.hpp>

#include <Input/GLFW/GLFWKeyboard.hpp>
#include <Input/GLFW/GLFWMouse.hpp>
#include <Input/GLFW/GLFWKeyboardCache.hpp>
#include <Input/GLFW/GLFWMouseCache.hpp>

#include <stdlib.h>
#include <stdio.h>

using namespace ehj;
using namespace glm;

#define SCENETYPE RCScene
class RCScene : IScene {
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

int run(void) {
	GLFWwindow* window;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

	int width = 640, height = 480;
	window = glfwCreateWindow(width, height, "ehjE RC 2D", NULL, NULL);
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
	OGLMesh oglMesh(mesh, GL_DYNAMIC_DRAW);
	oglMesh.bind(0);

	ehj_gl_err();
	glBindVertexArray(oglMesh.getVAO());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oglMesh.getEBO());

	auto createPass = [&](GLProgram& glp, std::string vs, std::string fs) {
		glp.addSourceFromFile(vs); //TODO make abstraction to make reloading easier
		ehj_gl_err();
		glp.addSourceFromFile(fs);
		ehj_gl_err();
		glp.createProgram();
		glBindAttribLocation(glp.getProgramID(),oglMesh.getAttribPos(),"vPos");
		if (oglMesh.getAttribNrm()!=-1)
			glBindAttribLocation(glp.getProgramID(),oglMesh.getAttribNrm(),"vNrm");
		ehj_gl_err();
	};
	GLProgram glpPencil;
	createPass(glpPencil,
		"scenes/RC/ssq.vs",
		"scenes/RC/pencil.fs"
	);
	
	GLProgram glpDO; // dynamic objects
	createPass(glpDO,
		"scenes/RC/ssq.vs",
		"scenes/RC/dynamicObj.fs"
	);
	
	GLProgram glpJumpFlood;
	createPass(glpJumpFlood,
		"scenes/RC/ssq.vs",
		"scenes/RC/jumpflood.fs"
	);

	GLProgram glpRC;
	createPass(glpRC,
		"scenes/RC/ssq.vs",
		"scenes/RC/rc.fs"
	);

	glm::mat4 pvm = glm::ortho(-1.f,1.f,-1.f,1.f);

	m_cam.setTiltable(false);
	m_cam.setPos(vec3(0.,0.,3.));
	m_cam.setDir(vec3(0.,0.,-1.));

	GLFWfpsLimiter fpsLimiter;
	//glfwGetFramebufferSize(window, &width, &height);
	ivec2 prevRes = {width,height};

	GLFrameBuffer::Opt fbOpt {prevRes,GL_RGBA8,GL_LINEAR};

	GLFrameBuffer fbr1(fbOpt);
	GLFrameBuffer fbr2(fbOpt);
	
	//fbOpt = GLFrameBuffer::Opt{prevRes/2,GL_RGBA8,GL_LINEAR};
	fbOpt = GLFrameBuffer::Opt{prevRes,GL_RGBA8,GL_LINEAR};
#if 0
	GLFrameBuffer fbrc1(fbOpt);
	GLFrameBuffer fbrc2(fbOpt);
#else
	GLFrameBuffer fbrc[7] = {
		GLFrameBuffer(fbOpt),
		GLFrameBuffer(fbOpt),
		GLFrameBuffer(fbOpt),
		GLFrameBuffer(fbOpt),
		GLFrameBuffer(fbOpt),
		GLFrameBuffer(fbOpt),
		GLFrameBuffer(fbOpt)
	};
#endif
	
	GLFrameBuffer fbJumpFlood(prevRes);
	GLFrameBuffer fbJumpFlood2(prevRes);

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
	bool pencilVisible = true;
	bool linearFilter = true;
	bool linearFilterPrev = true;

	int rayCount = 4;
	int raySteps = 100;
	float rayNoise = 1.;
	float rayDist = 1.;
	int jfPassCount = 11;
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
	GPUTimer gpuTimer;

	while (!glfwWindowShouldClose(window)) {
		
		gpuTimer.start();

		bool hoveredImgui = ImGui::IsAnyItemHovered() || ImGui::IsAnyItemActive() || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
		glfwGetFramebufferSize(window, &width, &height);
		width = max(1,width);
		height = max(1,height);

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
					glBindAttribLocation(glp.getProgramID(),oglMesh.getAttribPos(),"vPos");
					if (oglMesh.getAttribNrm()!=-1)
						glBindAttribLocation(glp.getProgramID(),oglMesh.getAttribNrm(),"vNrm");
					ehj_gl_err_continue();
				};

				reloadPass(glpPencil,"scenes/RC/pencil.fs");
				reloadPass(glpDO,"scenes/RC/dynamicObj.fs");
				reloadPass(glpRC,"scenes/RC/rc.fs");
				reloadPass(glpJumpFlood,"scenes/RC/jumpflood.fs");
				//
				//frame = 0;
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
			glUniform1i(glp.getUnfLoc("u_pencilVisible"),pencilVisible);
			glUniform1i(glp.getUnfLoc("u_rayCount"),rayCount);
			glUniform1i(glp.getUnfLoc("u_raySteps"),raySteps);
			glUniform1f(glp.getUnfLoc("u_rayNoise"),rayNoise);
			glUniform1f(glp.getUnfLoc("u_rayDist"),rayDist);
			glUniform1i(glp.getUnfLoc("u_viewPass"),viewPass);
			glUniform1f(glp.getUnfLoc("u_lightStr"),lightStr);
			glUniform1i(glp.getUnfLoc("u_cascadeCount"),cascadeCount);
			glUniform1i(glp.getUnfLoc("u_viewCascade"),viewCascade);
		};

		{ // draw pencil
			glBindFramebuffer(GL_FRAMEBUFFER,fbr1.getFBO());
			glViewport(0, 0, width, height);
			if (widthPrev != width || height != heightPrev) {
				frame = 0;
				
				GLFrameBuffer::Opt fbOpt {ivec2(width,height),GL_RGBA8};
				fbr1 = GLFrameBuffer(fbOpt);
				fbr2 = GLFrameBuffer(fbOpt);
				//glClearColor(0.,0.,0.,0.);
				//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			} else {
				glClear(GL_DEPTH_BUFFER_BIT);
			}
			glpPencil.bind();
			setCMNuniforms(glpPencil);
			glDrawElements(GL_TRIANGLES,oglMesh.getEBOsize(),GL_UNSIGNED_INT,0);
			glBindFramebuffer(GL_FRAMEBUFFER,0);
		}

		struct Bl {
			vec2 p,d;
		};
		static std::vector<Bl> bounceLightsC;
		for (int i=0; i < bounceLightsC.size(); ++i) {
			auto& b = bounceLightsC[i];
			b.p += b.d * deltaTime * .2f;
			if (b.p.x > float(width)/height || b.p.x < 0.) {
				b.p.x = clamp(b.p.x,0.f,float(width)/height);
				b.d.x *= -1;
			}
			if (b.p.y > 1. || b.p.y < 0.) {
				b.p.y = clamp(b.p.y,0.f,1.f);
				b.d.y *= -1;
			}
		}

		{ // draw temporary elements / dynamic objects
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,fbr1.getTexCol());
			glBindFramebuffer(GL_FRAMEBUFFER,fbr2.getFBO());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glpDO.bind();
			setCMNuniforms(glpDO);

			std::vector<vec2> bounceLights;
			for (int i=0;i<bounceLightsC.size();++i)
				bounceLights.push_back(bounceLightsC[i].p);

			glUniform1i(glpDO.getUnfLoc("u_bounceLightCount"),bounceLights.size());
			glUniform2fv(glpDO.getUnfLoc("u_bounceLights"), bounceLights.size(), (float*) bounceLights.data());

			glDrawElements(GL_TRIANGLES,oglMesh.getEBOsize(),GL_UNSIGNED_INT,0);
		}

		// flood fill pass, requires for loop passes in order to cover whole screen
		int jfPassCountOrig = ceil(glm::log2((float) fmax(width,height)));
		//if (frame%2==0)
		{ // JFA
			//TODO lower res super good but has flickering see JFA channel
			int wd2 = width/4;
			int hd2 = height/4;
			if (widthPrev != width || height != heightPrev) {
				//fbJumpFlood = GLFrameBuffer(ivec2(width,height));
				//fbJumpFlood2 = GLFrameBuffer(ivec2(width,height));
				GLFrameBuffer::Opt opt = {ivec2(wd2,hd2),GL_RGBA32F,GL_LINEAR};
				fbJumpFlood = GLFrameBuffer(opt);
				fbJumpFlood2 = GLFrameBuffer(opt);
			}

			//glViewport(0, 0, width, height);
			glViewport(0, 0, wd2, hd2);
			glBindFramebuffer(GL_FRAMEBUFFER,fbJumpFlood2.getFBO());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glBindFramebuffer(GL_FRAMEBUFFER,fbJumpFlood.getFBO());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glpJumpFlood.bind();
			setCMNuniforms(glpJumpFlood);
			
			glUniform2f(glpJumpFlood.getUnfLoc("u_resolution"), wd2,hd2);

			// already bound glBindFramebuffer(GL_FRAMEBUFFER,fbJumpFlood.getFBO());
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,fbr2.getTexCol());
			glUniform1i(glGetUniformLocation(glpJumpFlood.getProgramID(), "u_tex"), 0);           // texture unit 0
			// render uv
			glUniform1f(glpJumpFlood.getUnfLoc("u_jfOffset"),0.);
			
			glDrawElements(GL_TRIANGLES,oglMesh.getEBOsize(),GL_UNSIGNED_INT,0);
			for (int i=0;i< jfPassCount;++i) {
				auto* fbJFfrom = &fbJumpFlood;
				auto* fbJFto = &fbJumpFlood2;
				if (i%2==1)
					std::swap(fbJFfrom,fbJFto);
				
				glBindFramebuffer(GL_FRAMEBUFFER,fbJFto->getFBO());
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D,fbJFfrom->getTexCol());
				glUniform1i(glGetUniformLocation(glpJumpFlood.getProgramID(), "u_texJumpFlood"), 1);  // texture unit 1
				glUniform1f(glpJumpFlood.getUnfLoc("u_jfOffset"),pow(2,jfPassCount-i-1));

				glDrawElements(GL_TRIANGLES,oglMesh.getEBOsize(),GL_UNSIGNED_INT,0);
			}

			if (jfPassCount %2==0) {
				// blit into fbJFto
				glBlitNamedFramebuffer(fbJumpFlood2.getFBO(),fbJumpFlood.getFBO(),
					//0,0,width,height,0,0,width,height,
					0,0,wd2,hd2,0,0,wd2,hd2,
					GL_COLOR_BUFFER_BIT,GL_NEAREST);
			}
		}

		{ // RC 2D pass, TODO upscale with FSR2
			if (widthPrev != width || height != heightPrev || linearFilter != linearFilterPrev) {
				//GLFrameBuffer::Opt fbOpt {ivec2(width,height)/2,GL_RGBA8};
				GLFrameBuffer::Opt fbOpt {ivec2(width,height),GL_RGBA8};
				fbOpt.texturefilter = linearFilter ? GL_LINEAR : GL_NEAREST;
#if 0
				fbrc1 = GLFrameBuffer(fbOpt);
				fbrc2 = GLFrameBuffer(fbOpt);
#else
				for (int i=0;i<7;++i)
					fbrc[i] = GLFrameBuffer(fbOpt);
#endif
			}

			glViewport(0, 0, width, height);
#if 0
			glBindFramebuffer(GL_FRAMEBUFFER,fbrc1.getFBO());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glBindFramebuffer(GL_FRAMEBUFFER,fbrc2.getFBO());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
			for (int i=0;i<7;++i) {
				//if (frame%(i*1+1)==0)
				//if (frame%(max(0,i-1)+1)==0)
				{
					glBindFramebuffer(GL_FRAMEBUFFER,fbrc[i].getFBO());
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				}
			}
			//else {
			//	glBindFramebuffer(GL_FRAMEBUFFER,fbrc[0].getFBO());
			//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//}
#endif

			glpRC.bind();
			setCMNuniforms(glpRC);
			//glUniform2f(glpRC.getUnfLoc("u_resolution"), float(width)/2,float(height)/2);
			glUniform2f(glpRC.getUnfLoc("u_resolution"), width,height);

			// already bound glBindFramebuffer(GL_FRAMEBUFFER,fbRC.getFBO());
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,fbr2.getTexCol());
			glUniform1i(glGetUniformLocation(glpRC.getProgramID(), "u_tex"), 0);           // texture unit 0
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D,fbJumpFlood.getTexCol());
			glUniform1i(glGetUniformLocation(glpRC.getProgramID(), "u_texJumpFlood"), 1);  // texture unit 1
			// render uv
			//glUniform1f(glpRC.getUnfLoc("u_jfOffset"),0.);
			
			for (int i=cascadeCount-1;i>=0;--i) {
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
				auto* fbFrom = &fbrc[i+1];
				auto* fbTo = &fbrc[i];
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
				glUniform1i(glGetUniformLocation(glpRC.getProgramID(), "u_texPrev"), 2);           // texture unit 0

				glDrawElements(GL_TRIANGLES,oglMesh.getEBOsize(),GL_UNSIGNED_INT,0);
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

			{ // blit rc result into buf 0 
				glBindFramebuffer(GL_FRAMEBUFFER,0);
				glViewport(0, 0, width, height);
				//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // shouldnt be required
#if 0
				glBlitNamedFramebuffer(fbrc1.getFBO(),0,
					0,0,width/2,height/2,0,0,width,height,
					GL_COLOR_BUFFER_BIT,GL_NEAREST);
#else
				glBlitNamedFramebuffer(fbrc[0].getFBO(),0,
					//0,0,width/2,height/2,0,0,width,height,
					0,0,width,height,0,0,width,height,
					GL_COLOR_BUFFER_BIT,GL_NEAREST);
#endif
			}
		}

		gpuTimer.end();
		
		ehj_gl_err();
		
		{ // imgui
			linearFilterPrev = linearFilter;
			
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			{
				ImGui::Begin("RC 2D");
				float ms1 = gpuTimer.getMS();
				static float ms = 1.;
				ms = ms*.99 + ms1*.01;
				std::string strMS = "MS: " + std::to_string(ms);
				ImGui::Text("%s", strMS.c_str());
				std::string strFPS = "FPS: " + std::to_string(1000./ms);
				ImGui::Text("%s", strFPS.c_str());
				ImGui::Checkbox("show pencil",&pencilVisible);
				ImGui::DragFloat("pencilSize",&pencilSize,0.001,0.001,10.);
				ImGui::ColorEdit4("pencilColor",&pencilColor[0]);
				ImGui::DragInt("rayCount",&rayCount,1,1,16);
				ImGui::DragInt("raySteps",&raySteps,1,1,100);
				ImGui::DragFloat("rayNoise",&rayNoise,0.001,0.,1.);
				ImGui::DragFloat("rayDist",&rayDist,0.001,0.,1.);
				ImGui::SliderInt("jfPassCountMod",&jfPassCount,0,jfPassCountOrig);
				ImGui::DragFloat("lightStr",&lightStr,0.001);
				ImGui::Separator();
				{
					ImGui::Combo("view pass",&viewPass,viewPassStr,VP_COUNT);
				}
				ImGui::SliderInt("cascade Count",&cascadeCount,0,6);
				ImGui::SliderInt("viewCascade",&viewCascade,0,cascadeCount);
				ImGui::Checkbox("linear texture filter",&linearFilter);

				if(ImGui::Button("add bounce light")) {
					Bl b = {vec2(float(rand())/RAND_MAX,float(rand())/RAND_MAX),
							vec2(float(rand())/RAND_MAX,float(rand())/RAND_MAX)};
					bounceLightsC.push_back(b);
				}
				if(ImGui::Button("clear bounce lights")) {
					bounceLightsC.clear();
				}
				ImGui::DragFloat("rayOverlap",&rayOverlap,0.001);
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
