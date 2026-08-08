#include "GAPI/OGL/GLMesh.hpp"

#include "../bvh/bvh.hpp"
//tinybvh::BVH4_GPU

#include <GLFW/glfw3.h>
#include <suOGL.hpp>

#include <Input/GLFW/GLFWKeyboardCache.hpp>
#include <Utility/ConfigFile.hpp>

#include "../../EnvirTexScene/GLEntity.hpp"
#include "suCMN.hpp"

using namespace ehj;

#define SCENETYPE IBLtestScene
class IBLtestScene : IScene {
public:
	~IBLtestScene() {}
	void setup() {
		m_glWindow.setup(&m_configFile, "ehjE IBL test Scene");
	}

	int run() {
		GLEntity sponza;
		sponza.load("myModels/sponza/gltf/Sponza.gltf");
	
		GLProgram glpRaster;
		auto glpRasterbuild = [&]() -> bool {
			bool r= glpRaster.addSourceFromFileRecursive(std::string("scenes/EnvirTexScene/")+"v.vert");
			r&= glpRaster.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"f.frag");
			glpRaster.createProgram();
			return r;
		}; glpRasterbuild();

		GLProgram glpRT;
		auto glpRTbuild = [&]() -> bool {
			bool r= glpRT.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"bvhPTIBL.comp");
			glpRT.createProgram();
			return r;
		}; glpRTbuild();
		
		GLProgram glpPP;
		auto glpPPbuild = [&]() -> bool {
			bool r= glpPP.addSourceFromFile("shaders/ssq.vs");
			r &= glpPP.addSourceFromFile(EHJ_THIS_FOLDER()+"ppIBL.fs");
			glpPP.createProgram();
			return r;
		}; glpPPbuild();
		
		ehj::SSMesh ssm; {
			ssm.toTriangles();
			ssm.assembleVertexBuffer();
		}
		GLVertexBuffer ssmGlVb(ssm.m_vertexData);
		GLMesh ssmGl(ssm, GL_DYNAMIC_DRAW);

		float time = 0.0f;

		{ // gl params
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
		}

		m_cam.setPos({0.,0.,0.});

		GLFrameBuffer::Opt fbRTopt;
		fbRTopt.res={480.*2.0,270.*2.0};
		GLFrameBuffer fbRT(fbRTopt);
		
		GLFrameBuffer fbr1(fbRTopt);
		GLFrameBuffer fbr2(fbRTopt);
		bool ping = true;
		
		GLFrameBuffer::Opt fbRasterOpt;
		fbRasterOpt.internalformat=GL_RGBA8;
		GLFrameBuffer fbRaster(fbRasterOpt);

		GLFWfpsLimiter m_fpsLimiter;
		
		tinybvh::BVH_GPU bvh;

		GLuint bvhSSBOnode;
		GLuint bvhSSBOidx;
		GLuint bvhSSBOtri;
		if (1) { // build bvh
			std::vector<uint32_t> indexBuffer;  // build complete index buffer from all meshes

			int totalBroken = 0;
			for (auto& m : sponza.model.m_meshes) {
				auto ib = m.getIndexBuffer();
				for (auto i : ib)
					if (i >= sponza.model.m_vertexData.positions.size())
						totalBroken++;// std::cerr << "uh: " << i << std::endl;
				indexBuffer.insert(indexBuffer.end(),ib.begin(),ib.end());
			}

			std::vector<float> mPos;
			{
				//TODO implement alignment toggle
				//int attribSize = 3; // pos
				//attribSize += sponza.model.m_vertexData.m_VP & VertexData::VP_NRM ? 3 : 0;
				//attribSize += sponza.model.m_vertexData.m_VP & VertexData::VP_COL ? 3 : 0;
				//attribSize += sponza.model.m_vertexData.m_VP & VertexData::VP_UV  ? 2 : 0;
				//attribSize += sponza.model.m_vertexData.m_VP & VertexData::VP_TAN ? 4 : 0;
				
				int attribSize = 4; // pos
				attribSize += sponza.model.m_vertexData.m_VP & VertexData::VP_NRM ? 4 : 0;
				attribSize += sponza.model.m_vertexData.m_VP & VertexData::VP_COL ? 4 : 0;
				attribSize += sponza.model.m_vertexData.m_VP & VertexData::VP_UV  ? 4 : 0;
				attribSize += sponza.model.m_vertexData.m_VP & VertexData::VP_TAN ? 4 : 0;
				auto& mVmerg = sponza.model.m_vertexData.merged;
				for (int i=0;i<mVmerg.size();i+=attribSize) {
					mPos.emplace_back(mVmerg[i]);
					mPos.emplace_back(mVmerg[i+1]);
					mPos.emplace_back(mVmerg[i+2]);
					mPos.emplace_back(0);
				}
			}


			size_t bytesNeeded = mPos.size() * sizeof(float);

			// Bitwise magic to round up to the nearest multiple of 64
			size_t paddedBytes = (bytesNeeded + 63) & ~63;
			
			tinybvh::bvhvec4* vertices = (tinybvh::bvhvec4*)tinybvh::malloc64(paddedBytes);

			int vIdx = 0; // Index for the bvhvec4 array
			for (int i = 0; i < mPos.size(); i += 4) {
				// bvhvec4 takes x, y, z, w in its constructor
				vertices[vIdx] = tinybvh::bvhvec4(mPos[i], mPos[i+1], mPos[i+2], mPos[i+3]);
				vIdx++;
			}
			//vertices.emplace_back(p.x,p.y,p.z,p.w);
			
			bytesNeeded = indexBuffer.size() * sizeof(uint);

			// Bitwise magic to round up to the nearest multiple of 64
			paddedBytes = (bytesNeeded + 63) & ~63;
			uint* indices = (uint*)tinybvh::malloc64(paddedBytes);
			for (int i=0;i<indexBuffer.size();++i)
				indices[i]=indexBuffer[i];

			bvh.Build(vertices,indices,indexBuffer.size()/3);

			// Create the buffer objects directly using Direct State Access (DSA)
			glCreateBuffers( 1, &bvhSSBOnode );     // BVH node data 
			glCreateBuffers( 1, &bvhSSBOidx );      // triangle index data 
			glCreateBuffers( 1, &bvhSSBOtri );      // triangle vertex data, 3 * float4 per triangle 

			// Allocate storage directly via the buffer handles
			glNamedBufferStorage( bvhSSBOnode, bvh.usedNodes * sizeof( tinybvh::BVH_GPU::BVHNode ),
				bvh.bvhNode, GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT );
			glNamedBufferStorage( bvhSSBOidx, bvh.idxCount * sizeof( uint32_t ),
			 	bvh.bvh.primIdx, GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT );
			glNamedBufferStorage( bvhSSBOtri, bvh.triCount * sizeof( tinybvh::bvhvec4 ) * 3,
				bvh.bvh.verts.data, GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT );
		}

		int frame = 0;

		// skybox
		GLTexture::Opt skyboxOpt;
		skyboxOpt.path="myModels/skybox/venice_sunset_2k.hdr";
		GLTexture skybox(skyboxOpt);

		
		while (m_glWindow.stillOpen()) {
			{ // reload shader
				static bool suc = true;
				if (GLFWKeyboardCache::keyReleased(IBCodes::KK_KEY_R))
					m_kkTap[IBCodes::KK_KEY_R] = true;
				if (GLFWKeyboardCache::keyPressed(IBCodes::KK_KEY_R) && m_kkTap[IBCodes::KK_KEY_R]) {
					suc = true;
					m_kkTap[IBCodes::KK_KEY_R] = false;
					glUseProgram(0);
					
					suc &= glpRasterbuild();
					suc &= glpRTbuild();
					suc &= glpPPbuild();
				}
				if (!suc) {
					m_fpsLimiter.wait();
					m_glWindow.pollInput();
					continue;
				}
			}

			float deltaTime = m_clock.update();
			time += deltaTime;
			
			m_cam.update(deltaTime);

			int width, height;
			glfwGetFramebufferSize(m_glWindow.m_pWindow, &width, &height);
			auto& m_winRes=m_glWindow.m_winRes;
			m_winRes = glm::ivec2(width,height);

			m_cam.setProj(glm::perspective(glm::radians(90.0f), std::fmax(0.00001f,(float)m_winRes.x/(float)m_winRes.y),0.01f,100.0f));
			mat4 m=glm::scale(mat4(1.),vec3(0.005));

			mat4 p=m_cam.getProj();
			mat4 v=m_cam.getView();

			if (1) { // pt
				glpRT.bind();
				{ // uniforms
					{ // fb
						if (ping) {
							glBindImageTexture(0, fbr2.getTexCol(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
							glActiveTexture(GL_TEXTURE0);
							glBindTexture(GL_TEXTURE_2D,fbr1.getTexCol());
						} else {//pong
							glBindImageTexture(0, fbr1.getTexCol(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
							glActiveTexture(GL_TEXTURE0);
							glBindTexture(GL_TEXTURE_2D,fbr2.getTexCol());
						}
						ping = !ping;
					}

					glUniform1f(glpRT.getUnfLoc("u_time"), time);
					//glUniform2f(glpRT.getUnfLoc("u_resolution"), width, height);
					glUniform2f(glpRT.getUnfLoc("u_resolution"), fbRTopt.res.x, fbRTopt.res.y);

					glm::vec3 camPos = m_cam.getPos(); 
					glUniform3fv(glpRT.getUnfLoc("u_cPos"), 1, &camPos[0]);
					glm::mat4 invVP = glm::inverse(p * v);
					glUniformMatrix4fv(glpRT.getUnfLoc("u_invVP"), 1, GL_FALSE, &invVP[0][0]);
					glUniformMatrix4fv(glpRT.getUnfLoc("u_m"), 1, GL_FALSE, &m[0][0]);

					// accumulate
					static vec3 prevDir;
					static vec3 prevPos;
					if (!all(epsilonEqual(m_cam.getDir(),prevDir,epsilon<float>()*2.f))
						|| !all(epsilonEqual(m_cam.getPos(),prevPos,epsilon<float>()*2.f))) {
						glUniform1f(glpRT.getUnfLoc("u_camChange"), 1.f);
						frame = 0;
					}
					else
						glUniform1f(glpRT.getUnfLoc("u_camChange"), 0.f);
					prevDir = m_cam.getDir();
					prevPos = m_cam.getPos();
					glUniform1f(glpRT.getUnfLoc("u_frame"), frame);
				}
				//fbRT.resize(m_winRes);

				{
					glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_2D,skybox.getTex());
					
					{
						mat4 ip = glm::inverse(p);
						mat4 iv = glm::inverse(v);
						//iv[3] = vec4(0.,0.,0.,1.);
						glUniformMatrix4fv(glpRT.getUnfLoc("u_camInvProj"), 1, GL_FALSE, &ip[0][0]);
						glUniformMatrix4fv(glpRT.getUnfLoc("u_camInvView"), 1, GL_FALSE, &iv[0][0]);
					}
					
				}

				{ // bind buffers
					//glBindImageTexture(0, fbRT.getTexCol(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

					glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, bvhSSBOnode);
					glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, bvhSSBOidx);
					glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 2, bvhSSBOtri);
				}

				{ // dispatch compute thread for each pixel
					ivec3 ls; // local work group size
					glGetProgramiv(glpRT.getID(), GL_COMPUTE_WORK_GROUP_SIZE, &ls[0]);
					//ivec3 ds = (ivec3(width, height, 1) + ls - ivec3(1)) / ls;  // dispatch size
					ivec3 ds = (ivec3(fbRTopt.res.x, fbRTopt.res.y, 1) + ls - ivec3(1)) / ls;  // dispatch size
					glDispatchCompute(ds[0],ds[1],ds[2]);
					glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
				}

#if 0
				// push to backbuffer
				if (ping) {
					glBindFramebuffer(GL_READ_FRAMEBUFFER, fbr1.getFBO());
				} else {//pong
					glBindFramebuffer(GL_READ_FRAMEBUFFER, fbr2.getFBO());
				}
				//glBindFramebuffer(GL_READ_FRAMEBUFFER, fbRT.getFBO());
				glReadBuffer(GL_COLOR_ATTACHMENT0); 
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				glDrawBuffer(GL_BACK);
				glBlitFramebuffer(
					0,0,fbRTopt.res.x,fbRTopt.res.y,  // Source rectangle
					0,0,m_winRes.x,m_winRes.y,  // Destination rectangle
					GL_COLOR_BUFFER_BIT,         // Clear/copy mask
					GL_LINEAR // Filtering (GL_NEAREST or GL_LINEAR)
				);
#endif
			}

			if (1) { // rasterize
				glpRaster.bind();
				static int prevWidth=1;
				static int prevHeight=1;
				if (prevWidth != width || prevHeight != height) {
					fbRaster.resize(ivec2(width,height));
					prevWidth=width;
					prevHeight=height;
				}
				glBindFramebuffer(GL_FRAMEBUFFER,fbRaster.getFBO());
				glViewport(0, 0, width, height);
				
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
				glUniformMatrix4fv(glpRaster.getUnfLoc("u_p"), 1, GL_FALSE, &p[0][0]);
				glUniformMatrix4fv(glpRaster.getUnfLoc("u_v"), 1, GL_FALSE, &v[0][0]);
				glUniformMatrix4fv(glpRaster.getUnfLoc("u_m"), 1, GL_FALSE, &m[0][0]);

				mat4 u_imtn = glm::transpose(glm::inverse(m));
				glUniformMatrix4fv(glpRaster.getUnfLoc("u_imtn"), 1, GL_FALSE, &u_imtn[0][0]);

				glUniform1f(glpRaster.getUnfLoc("u_time"), time);
				glUniform2f(glpRaster.getUnfLoc("u_resolution"), width, height);

				sponza.draw();
			}
			
			if (1) { //PP
				glBindFramebuffer(GL_FRAMEBUFFER,0);
				glViewport(0, 0, width, height);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				glpPP.bind();
				glUniform1f(glpPP.getUnfLoc("u_time"), time);
				glUniform1f(glpPP.getUnfLoc("u_frame"), frame);
				glUniform2f(glpPP.getUnfLoc("u_resolution"), width,height);
				glm::mat4 pvm = glm::ortho(-1.f,1.f,-1.f,1.f);
				glUniformMatrix4fv(glpPP.getUnfLoc("u_pvm"), 1, GL_FALSE, &pvm[0][0]);

				if (ping) {
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D,fbr1.getTexCol());
				} else {//pong
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D,fbr2.getTexCol());
				}
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D,fbRaster.getTexCol());
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D,skybox.getTex());
				
				{
					mat4 ip = glm::inverse(p);
					mat4 iv = glm::inverse(v);
					//iv[3] = vec4(0.,0.,0.,1.);
					glUniformMatrix4fv(glpPP.getUnfLoc("u_camInvProj"), 1, GL_FALSE, &ip[0][0]);
					glUniformMatrix4fv(glpPP.getUnfLoc("u_camInvView"), 1, GL_FALSE, &iv[0][0]);
				}
				

				ssmGlVb.bind(0);
				ssmGl.bind();
				ssmGl.draw();
			}
			

			m_glWindow.swapBuffers();

			m_fpsLimiter.wait();
			m_glWindow.pollInput();
			frame++;
		}
		
		return 0;
	}
	void cleanup() {
		m_glWindow.close(&m_configFile);
		m_configFile.baseStore();
		//ImGui::DestroyContext();
	}
private:
	GLFWWindowGL m_glWindow;
	ConfigFile m_configFile;
	std::vector<bool> m_kkTap = std::vector<bool>(IBCodes::KK_COUNT,true);

	FreeFlyCamera m_cam;
	Clock m_clock;
};
