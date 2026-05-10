#include "Mesh.hpp"
#include <GLFW/glfw3.h>
#include <suOGL.hpp>

#include <Input/GLFW/GLFWKeyboardCache.hpp>
#include <Utility/ConfigFile.hpp>

#include "GLTexture3D.hpp"

#include "DrawCommandIndirect.glsl"

using namespace ehj;

#define SCENETYPE VoxelTestScene
class VoxelTestScene : IScene {
public:
	~VoxelTestScene() {}
	void setup() {
		m_glWindow.setup(&m_configFile, "ehjE VoxelTestScene");
	}

	int run() {
		SSMesh ssm;
		ssm.toTriangles();
		ssm.assembleVertexBuffer();
		GLVertexBuffer glVbSSM(ssm.m_vertexData);
		glVbSSM.bind(0);
		GLMesh glSSM(ssm);
		glSSM.bind();

		std::vector<std::string> paths;
		for (int i=0;i<99;++i)
			paths.push_back("myModels/VolumeScanCTHead/cthead-8bit"
				+ std::string(3 - std::to_string(i+1).length(), '0') + std::to_string(i+1)
				+ ".jpg"
			);
		GLTexture3D glTex3D(paths);
		
		//GLProgram glpSSM; {
		//	//glp.addSourceFromFile("shaders/PathTracing/ssq.vs");
		//	glpSSM.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"f.frag");
		//	glpSSM.createProgram();
		//	glpSSM.bind();
		//}

		GLProgram glpMain; {
			//glp.addSourceFromFile("shaders/PathTracing/ssq.vs");
			glpMain.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"v.vert");
			glpMain.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"f.frag");
			glpMain.createProgram();
			glpMain.bind();
		}

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		float time = 0.0f;

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		m_cam.setPos({0.,0.,0.});

		GLFWfpsLimiter m_fpsLimiter;

		// unit cube to draw
		Mesh unitCube("models/unitCube.obj");
		unitCube.m_vertexData.assembleVertexBuffer({&unitCube});
		GLVertexBuffer glVbUnitCube(unitCube.m_vertexData);
		GLMesh glUnitCube(unitCube, GL_DYNAMIC_DRAW);

		GLProgram glpCompIndirDraw;
		{ // indirect draw buffer
			glpCompIndirDraw.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"DrawListFrom3Dtex.comp");
			glpCompIndirDraw.createProgram();
			glpCompIndirDraw.bind();
		}
		
		GLProgram glpCompIndirDrawFinalize;
		{ // indirect draw buffer
			glpCompIndirDrawFinalize.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"DrawListFinalize.comp");
			glpCompIndirDrawFinalize.createProgram();
			glpCompIndirDrawFinalize.bind();
		}

		GLuint ssboInstances;
		GLsizei maxInstances=256*256*256;
		//GLsizei maxInstances=16*16*16;
		GLuint ssboCounter;
		GLuint indirectBuf;
		{ // ssbo
			glGenBuffers(1,&ssboInstances); glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboInstances);
			glBufferData(GL_SHADER_STORAGE_BUFFER, maxInstances * sizeof(EHJSD_Instance), nullptr, GL_DYNAMIC_DRAW);

			glGenBuffers(1,&ssboCounter); glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCounter);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
			GLuint zero = 0; glBufferSubData(GL_SHADER_STORAGE_BUFFER,0,sizeof(GLuint), &zero);

			unsigned int cubeIndexCount=unitCube.getIndexBuffer().size(); //TODO
			glGenBuffers(1,&indirectBuf); glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuf);
			EHJSD_DrawElementsIndirectCommand cmd = { cubeIndexCount, 0, 0, 0, 0 };
			glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(cmd), &cmd, GL_DYNAMIC_DRAW);
		}
		
		while (m_glWindow.stillOpen()) {

			{ // dispatch compute to build drawlist
				{ // reset atomic counter
					GLuint zero = 0;
					glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCounter);
					glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), &zero);
				}

				glpCompIndirDraw.bind();
				//glBindImageTexture or bind texture unit 0 for sampler3D;
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboInstances);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboCounter);
				//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, indirectBuf); // optional

				ivec2 res = glTex3D.getRes();
				int depth=glTex3D.m_opt.paths.size();

				glUniform3i(glGetUniformLocation(glpCompIndirDraw.getID(),"u_volumeDim"),res.x,res.y,depth);
				//glUniform1f(glGetUniformLocation(glpCompIndirDraw.getID(),"u_densityThreshold"),.5f);
				glUniform1f(glGetUniformLocation(glpCompIndirDraw.getID(),"u_densityThreshold"),sin(time*.5)*.25+.5);
				glBindTextureUnit(0, glTex3D.getTex());
				glActiveTexture(GL_TEXTURE0 + 0);
				glBindTexture(GL_TEXTURE_3D, glTex3D.getTex());

				 //TODO since local_size depends on shader -> find way to get shader size info
				glDispatchCompute((res.x+7)/8, (res.y+7)/8, (depth+7)/8);
				//glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
				glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			}

			{// indirect draw list finalize
				glpCompIndirDrawFinalize.bind();
				glUniform1ui(glGetUniformLocation(glpCompIndirDrawFinalize.getID(), "cubeIndexCount"), unitCube.getIndexBuffer().size());
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboCounter);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, indirectBuf);
				glDispatchCompute(1,1,1);
				glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
			}

			// main draw

			{ // reload shader
				static bool suc = true;
				if (GLFWKeyboardCache::keyReleased(IBCodes::KK_KEY_R))
					m_kkTap[IBCodes::KK_KEY_R] = true;
				if (GLFWKeyboardCache::keyPressed(IBCodes::KK_KEY_R) && m_kkTap[IBCodes::KK_KEY_R]) {
					bool sucTmp = true;
					m_kkTap[IBCodes::KK_KEY_R] = false;
					glUseProgram(0);
					
					//suc &= glpMain.addSourceFromFile("shaders/PathTracing/ssq.vs");
					suc &= glpMain.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"v.vert");
					suc &= glpMain.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"f.frag");
					
					glpMain.createProgram();

					suc = sucTmp;
				}
				if (!suc) {
					m_glWindow.pollInput(); // TODO check esc close window
					m_fpsLimiter.wait();
					continue;
				}
			}

			float deltaTime = m_clock.update();
			time += deltaTime;
			
			m_cam.update(deltaTime);

			int width, height;
			glfwGetFramebufferSize(m_glWindow.m_pWindow, &width, &height);
			auto& m_winRes=m_glWindow.m_winRes;
			m_winRes = ivec2(width,height);
			
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			ivec2 volRes = glTex3D.getRes();
			int volDepth=glTex3D.m_opt.paths.size();

			mat4 m,v,p,pvm;
			{ // 3d cam
				m_cam.setProj(glm::perspective(glm::radians(90.0f), std::fmax(0.00001f,(float)m_winRes.x/(float)m_winRes.y),0.01f,100.0f));
				//m=glm::scale(mat4(1.),vec3(0.01));
				m=glm::scale(mat4(1.),vec3(.1));
				m*=glm::scale(mat4(1.),vec3(1.,1.,float(.25*volRes.x)/volDepth));
				m*=glm::rotate(mat4(1.),3.141592f*.5f,vec3(1.,0.,0.));

				pvm = m_cam.getPV()*m;
				p=m_cam.getProj();
				v=m_cam.getView();
			}
			
			//{// orthogonal cam
			//	m = glm::mat4(1.0f); // identity
			//	v = glm::mat4(1.0f); // identity
			//	p = glm::ortho(-1.f,1.f,-1.f,1.f);
			//	pvm=p*v*m;
			//}
		
			//glUniformMatrix4fv(glp.getUnfLoc("u_p"), 1, GL_FALSE, &p[0][0]);
			//glUniformMatrix4fv(glp.getUnfLoc("u_v"), 1, GL_FALSE, &v[0][0]);
			//glUniformMatrix4fv(glp.getUnfLoc("u_m"), 1, GL_FALSE, &m[0][0]);
			//glUniformMatrix4fv(glp.getUnfLoc("u_pvm"), 1, GL_FALSE, &pvm[0][0]);

			//glUniform1f(glp.getUnfLoc("u_time"), time);
			//glUniform2f(glp.getUnfLoc("u_resolution"), width, height);

			//{ // draw texture directly
			//	glBindTextureUnit(0, glTex3D.getTex());
			//	glActiveTexture(GL_TEXTURE0 + 0);
			//	glBindTexture(GL_TEXTURE_3D, glTex3D.getTex());
			//
			//	glSSM.bind();
			//	glSSM.draw();
			//}
			
			glpMain.bind();
			glUniformMatrix4fv(glpMain.getUnfLoc("u_p"), 1, GL_FALSE, &p[0][0]);
			glUniformMatrix4fv(glpMain.getUnfLoc("u_v"), 1, GL_FALSE, &v[0][0]);
			glUniformMatrix4fv(glpMain.getUnfLoc("u_m"), 1, GL_FALSE, &m[0][0]);
			glUniformMatrix4fv(glpMain.getUnfLoc("u_pvm"), 1, GL_FALSE, &pvm[0][0]);

			glUniform1f(glpMain.getUnfLoc("u_time"), time);
			glUniform2f(glpMain.getUnfLoc("u_resolution"), width, height);

			glUniform3i(glGetUniformLocation(glpMain.getID(),"u_volumeDim"),volRes.x,volRes.y,volDepth);
			
			{ // draw indirect cube
				//glBindVertexArray(cubeVAO); // cube mesh with indices
				glVbUnitCube.bind(0);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboInstances); // instance data accessible in vertex shader
				glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuf);
				glUnitCube.bind();
				glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0);
				//glUnitCube.draw();
			}

			m_glWindow.swapBuffers();

			m_fpsLimiter.wait();
			m_glWindow.pollInput();
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
