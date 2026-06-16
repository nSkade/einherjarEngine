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

#define SCENETYPE EnvirPTScene
class EnvirPTScene : IScene {
public:
	~EnvirPTScene() {}
	void setup() {
		m_glWindow.setup(&m_configFile, "ehjE EnvirPTScene");
	}

	int run() {
		GLEntity sponza;
		sponza.load("myModels/sponza/gltf/Sponza.gltf");
	
		GLProgram glpRaster;
		auto glpRasterbuild = [&]() -> bool {
			bool r= glpRaster.addSourceFromFileRecursive(std::string("scenes/EnvirTexScene/")+"v.vert");
			r&= glpRaster.addSourceFromFileRecursive(std::string("shaders/")+"basic_f.frag");
			glpRaster.createProgram();
			return r;
		}; glpRasterbuild();

		GLProgram glpRT;
		auto glpRTbuild = [&]() -> bool {
			//bool r= glpRT.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"bvhTest.comp");
			bool r= glpRT.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"bvhTest.comp");
			glpRT.createProgram();
			return r;
		}; glpRTbuild();

		float time = 0.0f;

		{ // gl params
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
		}

		m_cam.setPos({0.,0.,0.});

		GLFrameBuffer::Opt fbRTopt;
		GLFrameBuffer fbRT(fbRTopt);

		GLFWfpsLimiter m_fpsLimiter;
		
		tinybvh::BVH_GPU bvh;
		//GLuint bvhEBO;
		//GLuint bvhEBOsize;

		GLuint bvhSSBOnode;
		GLuint bvhSSBOidx;
		GLuint bvhSSBOtri;
		{ // build bvh
			std::vector<uint32_t> indexBuffer;  // build complete index buffer from all meshes

			int totalBroken = 0;
			for (auto& m : sponza.model.m_meshes) {
				auto ib = m.getIndexBuffer();
				for (auto i : ib)
					if (i >= sponza.model.m_vertexData.positions.size())
						totalBroken++;// std::cerr << "uh: " << i << std::endl;
				indexBuffer.insert(indexBuffer.end(),ib.begin(),ib.end());
			}
			std::cerr << "total borked: " << totalBroken;
			
			// using EBO instead?
			//bvhEBOsize= indexBuffer.size();
			//glCreateBuffers(1,&bvhEBO);
			//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bvhEBO);
			//glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size()*sizeof(int), &indexBuffer[0], GL_DYNAMIC_DRAW);
			//TODO glBufferData deprecated, use glNamedBufferStorage instead for fixed size data

			//std::vector<tinybvh::bvhvec4> vertices;
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

			//GLuint hitData;

			// Create the buffer objects directly using Direct State Access (DSA)
			glCreateBuffers( 1, &bvhSSBOnode );     // BVH node data 
			glCreateBuffers( 1, &bvhSSBOidx );      // triangle index data 
			glCreateBuffers( 1, &bvhSSBOtri );      // triangle vertex data, 3 * float4 per triangle 
			//glCreateBuffers( 1, &hitData );      // buffer to return intersection results in, one float4 per pixel 

			// Allocate storage directly via the buffer handles
			glNamedBufferStorage( bvhSSBOnode, bvh.usedNodes * sizeof( tinybvh::BVH_GPU::BVHNode ),
				bvh.bvhNode, GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT );
			glNamedBufferStorage( bvhSSBOidx, bvh.idxCount * sizeof( uint32_t ),
			 	bvh.bvh.primIdx, GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT );
			glNamedBufferStorage( bvhSSBOtri, bvh.triCount * sizeof( tinybvh::bvhvec4 ) * 3,
				bvh.bvh.verts.data, GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT );

			//hits = (float4*)MALLOC64( SCRWIDTH * SCRHEIGHT * sizeof( float4 ) ); 
			//glNamedBufferStorage( hitData, SCRWIDTH * SCRHEIGHT * sizeof( float4 ), hits,
			//	GL_MAP_PERSISTENT_BIT | GL_MAP_READ_BIT );

			// Bind the buffers to their respective shader storage binding points
			//glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, bvhSSBOnode);
			//glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, bvhSSBOidx);
			//glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 2, bvhSSBOtri);
			//glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 3, hitData );
		}
		
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
			mat4 u_imtn = glm::transpose(glm::inverse(m));

			if (1) { // pt
				{ // uniforms
					glpRT.bind();
					//glm::mat4 camPV = glm::scale(glm::mat4(1.f),glm::vec3(float(width)/height,1.,1.))*m_cam.getPV();
					//camPV = camPV*m;
					//glUniformMatrix4fv(glpRT.getUnfLoc("u_m"),1,GL_TRUE,&camPV[0][0]);
					glUniform1f(glpRT.getUnfLoc("u_time"), time);
					glUniform2f(glpRT.getUnfLoc("u_resolution"), width, height);

					glm::vec3 camPos = m_cam.getPos(); 
					glUniform3fv(glpRT.getUnfLoc("u_cPos"), 1, &camPos[0]);
					glm::mat4 invVP = glm::inverse(p * v);
					glUniformMatrix4fv(glpRT.getUnfLoc("u_invVP"), 1, GL_FALSE, &invVP[0][0]);
					glUniformMatrix4fv(glpRT.getUnfLoc("u_m"), 1, GL_FALSE, &m[0][0]);
				}
				fbRT.resize(m_winRes);

				{ // bind buffers
					glBindImageTexture(0, fbRT.getTexCol(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

					glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, bvhSSBOnode);
					glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, bvhSSBOidx);
					glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 2, bvhSSBOtri);
				}

				{ // dispatch compute thread for each pixel
					ivec3 ls; // local work group size
					glGetProgramiv(glpRT.getID(), GL_COMPUTE_WORK_GROUP_SIZE, &ls[0]);
					ivec3 ds = (ivec3(width, height, 1) + ls - ivec3(1)) / ls;  // dispatch size
					glDispatchCompute(ds[0],ds[1],ds[2]);
				}

				// push to backbuffer
				glBindFramebuffer(GL_READ_FRAMEBUFFER, fbRT.getFBO());
				glReadBuffer(GL_COLOR_ATTACHMENT0); 
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				glDrawBuffer(GL_BACK);
				glBlitFramebuffer(
					0,0,m_winRes.x,m_winRes.y,  // Source rectangle
					0,0,m_winRes.x,m_winRes.y,  // Destination rectangle
					GL_COLOR_BUFFER_BIT,         // Clear/copy mask
					GL_NEAREST                   // Filtering (GL_NEAREST or GL_LINEAR)
				);
			}

			if (0) { // rasterize
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				{ // uniforms
					glpRaster.bind();
					glUniformMatrix4fv(glpRaster.getUnfLoc("u_p"), 1, GL_FALSE, &p[0][0]);
					glUniformMatrix4fv(glpRaster.getUnfLoc("u_v"), 1, GL_FALSE, &v[0][0]);
					glUniformMatrix4fv(glpRaster.getUnfLoc("u_m"), 1, GL_FALSE, &m[0][0]);
					glUniformMatrix4fv(glpRaster.getUnfLoc("u_imtn"), 1, GL_FALSE, &u_imtn[0][0]);
					glUniform1f(glpRaster.getUnfLoc("u_time"), time);
					glUniform2f(glpRaster.getUnfLoc("u_resolution"), width, height);
				}
				fbRT.resize(m_winRes);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbRT.getFBO());
				GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
				glDrawBuffers(1,drawBuffers);
				
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				sponza.glVb->bind(0);
				//int i=0;
				for (auto& m : sponza.glMeshes) {
					m->bind();
				//	//glActiveTexture(GL_TEXTURE0); 
				//	//glBindTexture(GL_TEXTURE_2D, sponza.glTextures[sponza.model.m_materials[sponza.model.m_meshesMaterialIDs[i]].baseColorTextureIndex]->getTex());
				//	//glActiveTexture(GL_TEXTURE1);
				//	//int normalTexIdx=sponza.model.m_materials[sponza.model.m_meshesMaterialIDs[i]].normalTextureIndex;
				//	//if (normalTexIdx != -1)
				//	//	glBindTexture(GL_TEXTURE_2D, sponza.glTextures[normalTexIdx]->getTex());
				//	//glActiveTexture(GL_TEXTURE2);
				//	//int matTexIdx=sponza.model.m_materials[sponza.model.m_meshesMaterialIDs[i]].metallicRoughnessTextureIndex;
				//	//if (matTexIdx != -1)
				//	//	glBindTexture(GL_TEXTURE_2D, sponza.glTextures[matTexIdx]->getTex());
					m->draw();
					//i++;
				}
				
				glBindFramebuffer(GL_READ_FRAMEBUFFER, fbRT.getFBO());
				glReadBuffer(GL_COLOR_ATTACHMENT0); 
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				glDrawBuffer(GL_BACK);
				glBlitFramebuffer(
					0,0,m_winRes.x,m_winRes.y,  // Source rectangle
					0,0,m_winRes.x,m_winRes.y,  // Destination rectangle
					GL_COLOR_BUFFER_BIT,         // Clear/copy mask
					GL_NEAREST                   // Filtering (GL_NEAREST or GL_LINEAR)
				);
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
		//ImGui::DestroyContext();
	}
private:
	GLFWWindowGL m_glWindow;
	ConfigFile m_configFile;
	std::vector<bool> m_kkTap = std::vector<bool>(IBCodes::KK_COUNT,true);

	FreeFlyCamera m_cam;
	Clock m_clock;
};
