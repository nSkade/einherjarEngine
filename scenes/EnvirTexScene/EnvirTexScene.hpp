#include "suCMN.hpp"
#include <GLFW/glfw3.h>
#include <suOGL.hpp>
#include <Input/GLFW/GLFWKeyboard.hpp>
#include <Input/GLFW/GLFWMouse.hpp>

#include <Utility/ConfigFile.hpp>
//TODO pch, remove, #include <GLFW/glfw3.h>

#include <Input/GLFW/GLFWKeyboardCache.hpp>

#include <thread>

using namespace ehj;

#define FULLSCREEN false

#define SCENETYPE EnvirTexScene
class EnvirTexScene : IScene {
public:
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
	}
	static void processInput(GLFWwindow *window) {
		if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
	}
	~EnvirTexScene() {}
	void setup() {
		if (!glfwInit())
			exit(EXIT_FAILURE);
	
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

		m_winRes.x = 640;
		m_winRes.y = 480;
		#if FULLSCREEN
			m_windowRes.x = 1920;
			m_windowRes.y = 1080;
		#endif
		m_configFile.baseLoad();
		m_configFile.load("winRes",&m_winRes);
		m_configFile.load("winPos",&m_winPos);
		
		//m_winMaximized=false;
		//m_configFile.load("winMaximized",&m_winMaximized);
		//if (m_winMaximized)
		//	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

		m_pWindow = glfwCreateWindow(m_winRes.x,m_winRes.y, "ehjE EnvirTexScene", NULL, NULL);
		glfwSetWindowPos(m_pWindow,m_winPos.x,m_winPos.y);
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

		m_kb = ehj::GLFWKeyboard::instance();
		m_mouse = ehj::GLFWMouse::instance();

		glfwSetCursorPosCallback(m_pWindow, m_mouse->mouse_callback);
		glfwSetMouseButtonCallback(m_pWindow, m_mouse->mouse_button_callback);
		glfwSetKeyCallback(m_pWindow, m_kb->key_callback);

		glfwMakeContextCurrent(m_pWindow);
		gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
		ehj_gl_err_callback();
		glViewport(0,0, m_winRes.x, m_winRes.y);
	}

	int run() {
		//TODO remove other pointless glTexParameteri calls in other setup files
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
		Timer t1;
		ehj::Model model("myModels/sponza/gltf/Sponza.gltf");
		std::cout << "loaded model in: " << t1.endTimer() << "ms\n"; t1.startTimer();
		
		model.assembleVertexBuffer();
		std::cout << "assembled vertex buffer in: " << t1.endTimer() << "ms\n"; t1.startTimer();
		
		std::vector<std::unique_ptr<GLMesh>> glMeshes;
		glMeshes.reserve(model.m_meshes.size());
		for (Mesh& m : model.m_meshes)
			glMeshes.emplace_back(std::make_unique<GLMesh>(m));
		std::cout << "assembled glMeshes in: " << t1.endTimer() << "ms\n"; t1.startTimer();
		
		GLMesh m(model.m_meshes[0]);
		GLVertexBuffer glVb(model.m_vertexData);
		glVb.bind(0);
		std::cout << "assembled glVertexBuffer in: " << t1.endTimer() << "ms\n"; t1.startTimer();

		// load textures
		std::vector<std::unique_ptr<GLTexture>> glTextures;
		std::vector<GLTexture::Opt> textureLoadOpts(model.m_textureInfos.size());
		std::vector<std::thread> loadThreads;
		for (int i=0;i<model.m_textureInfos.size();++i) {
			GLTexture::Opt& o = textureLoadOpts[i];
			loadThreads.emplace_back([&model, &o, i](){
				o.texturefilter=GL_LINEAR;
				auto& p = model.m_textureInfos[i];
				auto wm = [](Model::TextureInfo::WrapMode wm) {
					switch (wm) {
						case Model::TextureInfo::WrapMode::ClampToBorder:
							return GL_CLAMP_TO_BORDER;
						case Model::TextureInfo::WrapMode::ClampToEdge:
							return GL_CLAMP_TO_EDGE;
						case Model::TextureInfo::WrapMode::MirroredRepeat:
							return GL_MIRRORED_REPEAT;
						case Model::TextureInfo::WrapMode::Repeat:
							return GL_REPEAT;
						default:
							break;
					}
					return GL_CLAMP_TO_BORDER;
				};
				o.wrapS=wm(p.wrapS);
				o.wrapT=wm(p.wrapT);
				{
					int width, height, nrChannels;
					o.data = stbi_load(p.path.c_str(), &width, &height, &nrChannels, 0);
					o.width=width;
					o.height=height;
					o.nrChannels=nrChannels;
				}
			});
		}
		for (auto& t : loadThreads)
			t.join();

		for (auto& o : textureLoadOpts) {
			glTextures.emplace_back(std::make_unique<GLTexture>(o));
		}
		std::cout << "loaded textures in: " << t1.endTimer() << "ms\n"; t1.startTimer();
		
		GLProgram glp;
		glp.addSourceFromFile(EHJ_THIS_FOLDER()+"v.vert");
		glp.addSourceFromFile(EHJ_THIS_FOLDER()+"f.frag");
		glp.createProgram();
		glp.bind();
		
		glBindAttribLocation(glp.getID(),glVb.getAttribPos(),"vPos");
		glBindAttribLocation(glp.getID(),glVb.getAttribNrm(),"vNrm");
		glBindAttribLocation(glp.getID(),glVb.getAttribUV(),"vUV");

		GPUTimer fragSTimer;

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		float time = 0.0f;

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		m_cam.setPos({0.,0.,0.});

		for (auto& tlo : textureLoadOpts)
			stbi_image_free(tlo.data);

		GLFWfpsLimiter m_fpsLimiter;
		
		while (!glfwWindowShouldClose(m_pWindow)) {
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
						//glBindAttribLocation(glp.getID(),glVb.getAttribPos(),"vPos");
						//glBindAttribLocation(glp.getID(),glVb.getAttribNrm(),"vNrm");
						//glBindAttribLocation(glp.getID(),glVb.getAttribUV(),"vUV");
					};

					//reloadPass(glp,EHJ_THIS_FOLDER()+"f.frag");
					
					suc &= glp.addSourceFromFile(EHJ_THIS_FOLDER()+"v.vert");
					suc &= glp.addSourceFromFile(EHJ_THIS_FOLDER()+"f.frag");
					glp.createProgram();

					suc = sucTmp;
				}
				if (!suc) {
					glfwSwapBuffers(m_pWindow);
					glfwPollEvents();
					processInput(m_pWindow); // TODO check esc close window
					m_fpsLimiter.wait();
					continue;
				}
			}
			glp.bind();

			float deltaTime = m_clock.update();
			time += deltaTime;
			
			m_cam.update(deltaTime);

			int width, height;
			glfwGetFramebufferSize(m_pWindow, &width, &height);
			m_winRes = glm::ivec2(width,height);
			
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			m_cam.setProj(glm::perspective(glm::radians(90.0f), std::fmax(0.00001f,(float)m_winRes.x/(float)m_winRes.y),0.01f,100.0f));
			mat4 m=glm::scale(mat4(1.),vec3(0.005));
			mat4 pvm = m_cam.getPV()*m;
		
			glUniformMatrix4fv(glp.getUnfLoc("u_m"), 1, GL_FALSE, &m[0][0]);
			glUniformMatrix4fv(glp.getUnfLoc("u_pvm"), 1, GL_FALSE, &pvm[0][0]);

			mat4 u_imtn = glm::transpose(glm::inverse(m));
			glUniformMatrix4fv(glp.getUnfLoc("u_imtn"), 1, GL_FALSE, &u_imtn[0][0]);

			glUniform1f(glp.getUnfLoc("u_time"), time);
			glUniform2f(glp.getUnfLoc("u_resolution"), width, height);
	
			fragSTimer.start();
				int i=0;
				for (auto& m : glMeshes) {
					m->bind();
					glActiveTexture(GL_TEXTURE0); 
					glBindTexture(GL_TEXTURE_2D, glTextures[model.m_materials[model.m_meshesMaterialIDs[i]].baseColorTextureIndex]->getTex());
					glActiveTexture(GL_TEXTURE1);
					int normalTexIdx=model.m_materials[model.m_meshesMaterialIDs[i]].normalTextureIndex;
					if (normalTexIdx != -1)
						glBindTexture(GL_TEXTURE_2D, glTextures[normalTexIdx]->getTex());
					i++;
					m->draw();
				}
			fragSTimer.end();

			glfwSwapBuffers(m_pWindow);
			glfwPollEvents();
			processInput(m_pWindow);
		}
		glfwGetWindowPos(m_pWindow,&m_winPos.x,&m_winPos.y);
		//m_winMaximized =(bool) glfwGetWindowAttrib(m_pWindow, GLFW_MAXIMIZED);

		m_configFile.store("winRes",m_winRes);
		m_configFile.store("winPos",m_winPos);
		//m_configFile.store("winMaximized",m_winMaximized);
		m_configFile.baseStore();
		
		return 0;
	}
	void cleanup() {
		glfwDestroyWindow(m_pWindow);
		glfwTerminate();

		ImGui::DestroyContext();
	}
private:
	GLFWwindow* m_pWindow;
	ConfigFile m_configFile;
	std::vector<bool> m_kkTap = std::vector<bool>(IBCodes::KK_COUNT,true);

	glm::ivec2 m_winRes;
	glm::ivec2 m_winPos;
	//bool m_winMaximized;

	std::shared_ptr<ehj::GLFWKeyboard> m_kb;
	std::shared_ptr<ehj::GLFWMouse> m_mouse;

	FreeFlyCamera m_cam;
	Clock m_clock;
};
