#include <GLFW/glfw3.h>
#include <suOGL.hpp>

#include <Input/GLFW/GLFWKeyboardCache.hpp>
#include <Utility/ConfigFile.hpp>

#include <thread>

#include "GLEntity.hpp"

using namespace ehj;

#define SCENETYPE EnvirTexScene
class EnvirTexScene : IScene {
public:
	~EnvirTexScene() {}
	void setup() {
		m_glWindow.setup(&m_configFile, "ehjE EnvirTexScene");
	}

	int run() {
		//TODO remove other pointless glTexParameteri calls in other setup files
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
		Timer t1;
		Timer t2; t2.startTimer();
		ehj::Model model("myModels/sponza/gltf/Sponza.gltf");
		std::cout << "loaded model in: " << t1.endTimer() << "ms\n"; t1.startTimer();
		
		model.assembleVertexBuffer();
		std::cout << "assembled vertex buffer in: " << t1.endTimer() << "ms\n"; t1.startTimer();
		
		std::vector<std::unique_ptr<GLMesh>> glMeshes;
		glMeshes.reserve(model.m_meshes.size());
		for (Mesh& m : model.m_meshes)
			glMeshes.emplace_back(std::make_unique<GLMesh>(m));
		std::cout << "assembled glMeshes in: " << t1.endTimer() << "ms\n"; t1.startTimer();
		
		GLVertexBuffer glVb(model.m_vertexData);
		glVb.bind(0);
		std::cout << "assembled glVertexBuffer in: " << t1.endTimer() << "ms\n"; t1.startTimer();

		std::vector<std::unique_ptr<GLTexture>> glTextures;
		std::vector<GLTexture::Opt> textureLoadOpts(model.m_textureInfos.size());
		{ // load textures
			std::vector<std::thread> loadThreads;
			for (int i=0;i<model.m_textureInfos.size();++i) {
				GLTexture::Opt& o = textureLoadOpts[i];
				loadThreads.emplace_back([&model, &o, i](){
					o.texturefilter=GL_LINEAR;
					//o.internalformat=GL_COMPRESSED_RGBA; // takes significantly longer to load
					o.internalformat=GL_RGBA;
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
		}
		std::cout << "loaded textures in: " << t1.endTimer() << "ms\n"; t1.startTimer();

		for (auto& o : textureLoadOpts) {
			glTextures.emplace_back(std::make_unique<GLTexture>(o));
		}
		std::cout << "send textures to gpu in: " << t1.endTimer() << "ms\n"; t1.startTimer();
		
		GLEntity glEmonkey;
		//glEmonkey.load("myModels/monkeyTex/monkeyTex.gltf");
		
		GLProgram glp;
		glp.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"v.vert");
		glp.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"f.frag");
		std::cout << "add shaded in: " << t1.endTimer() << "ms\n"; t1.startTimer();
		glp.createProgram(); // TODO takes 1.6 sec on lap
		std::cout << "send shader to gpu in: " << t1.endTimer() << "ms\n"; t1.startTimer();
		glp.bind();

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
		
		model.m_meshes.clear();
		model.m_vertexData.clear();

		std::cout << "loaded scene in: " << t2.endTimer() << "ms\n";
		
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
					suc &= glp.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"f.frag");
					glp.createProgram();

					suc = sucTmp;
				}
				if (!suc) {
					m_glWindow.pollInput(); // TODO check esc close window
					m_fpsLimiter.wait();
					continue;
				}
			}
			glp.bind();

			float deltaTime = m_clock.update();
			time += deltaTime;
			
			m_cam.update(deltaTime);

			int width, height;
			glfwGetFramebufferSize(m_glWindow.m_pWindow, &width, &height);
			auto& m_winRes=m_glWindow.m_winRes;
			m_winRes = glm::ivec2(width,height);
			
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			m_cam.setProj(glm::perspective(glm::radians(90.0f), std::fmax(0.00001f,(float)m_winRes.x/(float)m_winRes.y),0.01f,100.0f));
			mat4 m=glm::scale(mat4(1.),vec3(0.005));

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
	
			fragSTimer.start();
				glVb.bind(0);
				int i=0;
				for (auto& m : glMeshes) {
					m->bind();
					glActiveTexture(GL_TEXTURE0); 
					glBindTexture(GL_TEXTURE_2D, glTextures[model.m_materials[model.m_meshesMaterialIDs[i]].baseColorTextureIndex]->getTex());
					glActiveTexture(GL_TEXTURE1);
					int normalTexIdx=model.m_materials[model.m_meshesMaterialIDs[i]].normalTextureIndex;
					if (normalTexIdx != -1)
						glBindTexture(GL_TEXTURE_2D, glTextures[normalTexIdx]->getTex());
					glActiveTexture(GL_TEXTURE2);
					int matTexIdx=model.m_materials[model.m_meshesMaterialIDs[i]].metallicRoughnessTextureIndex;
					if (matTexIdx != -1)
						glBindTexture(GL_TEXTURE_2D, glTextures[matTexIdx]->getTex());
					m->draw();
					i++;
				}
			fragSTimer.end();
			
			if (0) { // monkey
				mat4 m2=glm::scale(mat4(1.),vec3(.5));
				m2 = glm::rotate(m2,time*.1f, glm::vec3(0.f,1.f,0.f));
				m2 = glm::translate(m2,glm::vec3(0.f,1.f,0.f));
				glUniformMatrix4fv(glp.getUnfLoc("u_m"), 1, GL_FALSE, &m2[0][0]);
				//glEmonkey.draw();
			}

			//fragSTimer.print();

			m_glWindow.swapBuffers();

			m_glWindow.pollInput(); //TODO polling should happen during wait?
			m_fpsLimiter.wait();
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
