#include "GAPI/OGL/GLMesh.hpp"

#include "../bvh/bvh.hpp"
//tinybvh::BVH4_GPU

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <suOGL.hpp>

#include <Input/GLFW/GLFWKeyboardCache.hpp>
#include <Utility/ConfigFile.hpp>

#include "../../EnvirTexScene/GLEntity.hpp"
#include "suCMN.hpp"

#include "../../voxelizer/GLTexture3D.hpp"

using namespace ehj;

#define SCENETYPE EnvirPThybridTexScene
class EnvirPThybridTexScene : IScene {
public:
	~EnvirPThybridTexScene() {}
	void setup() {
		m_glWindow.setup(&m_configFile, "ehjE EnvirPTScene");
	}

	int run() {
		GLEntity::Opt sponzaOpt;
		sponzaOpt.textureAlbedoInternalFormat=GL_SRGB8_ALPHA8;
		sponzaOpt.path = "myModels/sponza/gltf/Sponza.gltf";
		//sponzaOpt.path = "myModels/SunTemple_v4/SunTemple/gltf_try2/Untitled.gltf";
		//sponzaOpt.path = "myModels/SunTemple_v4/UnrealSunTemple_Blender/gltf_test/SunTemple_BlendScene.gltf";
		GLEntity sponza;
		sponza.load(sponzaOpt);
	
		GLProgram glpRaster;
		auto glpRasterbuild = [&]() -> bool {
			bool r= glpRaster.addSourceFromFileRecursive(std::string("scenes/EnvirTexScene/")+"v.vert");
			r&= glpRaster.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"f.frag");
			glpRaster.createProgram();
			return r;
		}; glpRasterbuild();

		GLProgram glpRT;
		auto glpRTbuild = [&]() -> bool {
			//bool r= glpRT.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"bvhTest.comp");
			//bool r= glpRT.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"../envir/"+"bvhPTimp.comp");
			bool r= glpRT.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"bvhPTimpTex.comp");
			glpRT.createProgram();
			return r;
		}; glpRTbuild();
		
		GLProgram glpPP;
		auto glpPPbuild = [&]() -> bool {
			bool r= glpPP.addSourceFromFile("shaders/ssq.vs");
			r &= glpPP.addSourceFromFile(EHJ_THIS_FOLDER()+"ppHybrid.fs");
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

		m_cam.setPos({-6.7,.8,-.15});
		m_cam.setDir({0.99,0.005,0.03});

		GLFrameBuffer::Opt fbRTopt;
		fbRTopt.texturefilter=GL_NEAREST;
		//fbRTopt.res={480.*.5,270.*.5};
		//fbRTopt.res={480.*1.0,270.*1.0};
		fbRTopt.res={480.*1.5,270.*1.5};
		//fbRTopt.res={480.*2.0,270.*2.0};
		//fbRTopt.res={480.*3.0,270.*3.0};
		//fbRTopt.res={480.*4.0,270.*4.0};
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
		//GLuint bvhSSBOtri;
		GLuint bvhSSBOmat;
		{ // build bvh

			tinybvh::bvhvec4* vertices = nullptr;
#if 1 // use new vertex buffer
			{ // new vertex buffer
				std::vector<float> mPos;
				{
#if 0
					int attribSize = 3; // pos
					attribSize += sponza.model.m_vertexData.m_VP & VertexData::VP_NRM ? 3 : 0;
					attribSize += sponza.model.m_vertexData.m_VP & VertexData::VP_COL ? 3 : 0;
					attribSize += sponza.model.m_vertexData.m_VP & VertexData::VP_UV  ? 2 : 0;
					attribSize += sponza.model.m_vertexData.m_VP & VertexData::VP_TAN ? 4 : 0;
#endif
#if 1 //aligned
					int attribSize = 4; // pos
					attribSize += sponza.model.m_vertexData.m_VP & VertexData::VP_NRM ? 4 : 0;
					attribSize += sponza.model.m_vertexData.m_VP & VertexData::VP_COL ? 4 : 0;
					attribSize += sponza.model.m_vertexData.m_VP & VertexData::VP_UV  ? 4 : 0;
					attribSize += sponza.model.m_vertexData.m_VP & VertexData::VP_TAN ? 4 : 0;
#endif
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
				
				vertices = (tinybvh::bvhvec4*)tinybvh::malloc64(paddedBytes);

				int vIdx = 0; // Index for the bvhvec4 array
				for (int i = 0; i < mPos.size(); i += 4) {
					// bvhvec4 takes x, y, z, w in its constructor
					vertices[vIdx] = tinybvh::bvhvec4(mPos[i], mPos[i+1], mPos[i+2], mPos[i+3]);
					vIdx++;
				}
				//vertices.emplace_back(p.x,p.y,p.z,p.w);
			}
#endif
#if 0 // reuse rasterizer vertex buffer

			std::unique_ptr<tinybvh::bvhvec4slice> vertexSlice;
			{
				struct RRRT_vertex { // rasterizer RT shared vertex
					vec3 p;
					vec3 n;
					vec2 uv;
					vec4 tan;
				};

				RRRT_vertex* rrrtV = (RRRT_vertex*)&sponza.model.m_vertexData.merged[0];
				int size = sponza.model.m_vertexData.merged.size()/(3+3+2+4);
				
				std::vector<tinybvh::bvhvec4> tightPositions(size);
				
				for (size_t i = 0; i < size; ++i) 
					tightPositions[i] = tinybvh::bvhvec4(rrrtV[i].p.x, rrrtV[i].p.y, rrrtV[i].p.z, 1.0f);

				vertexSlice = std::make_unique<tinybvh::bvhvec4slice>(tightPositions.data(), sizeof(tinybvh::bvhvec4));
				
				//vertexSlice = std::make_unique<tinybvh::bvhvec4slice>(
				//	(tinybvh::bvhvec4*)&rrrtV[0].p,
				//	sizeof(RRRT_vertex)
				//);
			}
#endif

			uint* indices = nullptr;
			std::vector<uint32_t> indexBuffer;  // build complete index buffer from all meshes
			{ // index buffer

				int totalBroken = 0;
				for (auto& m : sponza.model.m_meshes) {
					auto ib = m.getIndexBuffer();
					for (auto i : ib)
						if (i >= sponza.model.m_vertexData.positions.size())
							totalBroken++;// std::cerr << "uh: " << i << std::endl;
					indexBuffer.insert(indexBuffer.end(),ib.begin(),ib.end());
				}
				
				size_t bytesNeeded = indexBuffer.size() * sizeof(uint);

				// Bitwise magic to round up to the nearest multiple of 64
				size_t paddedBytes = (bytesNeeded + 63) & ~63;
				indices = (uint*)tinybvh::malloc64(paddedBytes);
				for (int i=0;i<indexBuffer.size();++i)
					indices[i]=indexBuffer[i];
			}

			bvh.Build(vertices,indices,indexBuffer.size()/3);
			//bvh.Build(*vertexSlice.get(),indices,indexBuffer.size()/3);

			// Create the buffer objects directly using Direct State Access (DSA)
			glCreateBuffers( 1, &bvhSSBOnode );     // BVH node data 
			glCreateBuffers( 1, &bvhSSBOidx );      // triangle index data 
			//glCreateBuffers( 1, &bvhSSBOtri );      // triangle vertex data, 3 * float4 per triangle 
			glCreateBuffers( 1, &bvhSSBOmat );      // triangle index data 

			// Allocate storage directly via the buffer handles
			glNamedBufferStorage( bvhSSBOnode, bvh.usedNodes * sizeof( tinybvh::BVH_GPU::BVHNode ),
				bvh.bvhNode,
				0);// GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT );
			glNamedBufferStorage( bvhSSBOidx, bvh.idxCount * sizeof( uint32_t ),
			 	bvh.bvh.primIdx,
				0);// GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT );
			//glNamedBufferStorage( bvhSSBOtri, bvh.triCount * sizeof( tinybvh::bvhvec4 ) * 3,
			//	bvh.bvh.verts.data, GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT );

			{
//glActiveTexture(GL_TEXTURE0); 
//glBindTexture(GL_TEXTURE_2D, glTextures[model.m_materials[model.m_meshesMaterialIDs[i]].baseColorTextureIndex]->getTex());
//glActiveTexture(GL_TEXTURE1);
//int normalTexIdx=model.m_materials[model.m_meshesMaterialIDs[i]].normalTextureIndex;
//	glBindTexture(GL_TEXTURE_2D, glTextures[normalTexIdx]->getTex());
//glActiveTexture(GL_TEXTURE2);
//int matTexIdx=model.m_materials[model.m_meshesMaterialIDs[i]].metallicRoughnessTextureIndex;
//	glBindTexture(GL_TEXTURE_2D, glTextures[matTexIdx]->getTex());

				
				// triangle to material id
				std::vector<uint32_t> matBuffer;
				int i = 0;
				for (auto& m : sponza.model.m_meshes) {
					auto ib = m.getIndexBuffer();
					int ida = sponza.model.m_materials[sponza.model.m_meshesMaterialIDs[i]].baseColorTextureIndex;
					int idn = sponza.model.m_materials[sponza.model.m_meshesMaterialIDs[i]].normalTextureIndex;
					int idm = sponza.model.m_materials[sponza.model.m_meshesMaterialIDs[i]].metallicRoughnessTextureIndex;
					++i;
					std::vector<uint32_t> b(ib.size()); //3 verts of tri lucky aligns with 3 slots
					for (int j=0;j<b.size();j+=3) {
						b[j]=ida;
						b[j+1]=idn;
						b[j+2]=idm;
					}
					matBuffer.insert(matBuffer.end(),b.begin(),b.end());
				}
				
				glNamedBufferStorage( bvhSSBOmat, matBuffer.size() * sizeof( uint32_t ),
					matBuffer.data(),
					0);// GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT );
			}
		}

		int frame = 0;

		GLTexture3D::Opt blueNoiseTexOpt;
		blueNoiseTexOpt.texturefilter = GL_NEAREST;
		blueNoiseTexOpt.wrapS=GL_REPEAT;
		blueNoiseTexOpt.wrapT=GL_REPEAT;

		for (int i=0;i<16;++i)
			blueNoiseTexOpt.paths.push_back("myModels/noise/blue noise/LDR_RGBA_"
				+ std::to_string(i)
				+ ".png"
			);
		//blueNoiseTexOpt.path = "myModels/noise/blue noise/LDR_RGBA_0.png";
		GLTexture3D blueNoiseTex(blueNoiseTexOpt);

#define EHJ_BINDLESS_TEX 1 // renderdoc sadly doesnt support bindless textures in opengl
#if EHJ_BINDLESS_TEX
		GLuint bindlessTexSSBO;
		{ // bindless texture buffer
			glCreateBuffers( 1, &bindlessTexSSBO );
			std::vector<GLuint64> textureHandles(sponza.glTextures.size());
			for (size_t i = 0; i < sponza.glTextures.size(); ++i) {
				textureHandles[i] = glGetTextureHandleARB(sponza.glTextures[i]->getTex());
				glMakeTextureHandleResidentARB(textureHandles[i]); // places texture in gpu memory, if it doesnt fit into memory we need to manage them
				// do // glMakeTextureHandleNonResidentARB() to unload texture from gpu memory
			}
			glNamedBufferStorage( bindlessTexSSBO, textureHandles.size() * sizeof(GLuint64),
				textureHandles.data(),
				0);// GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT );
		}
#endif
		
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
					frame=0;
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
			mat4 m=glm::scale(mat4(1.),vec3(0.005,0.005,0.005));
			mat4 u_imtn = glm::transpose(glm::inverse(m));

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
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_3D, blueNoiseTex.getTex());
					glUniform3f(glpRT.getUnfLoc("u_blueNoiseSize"),blueNoiseTex.getRes().x,blueNoiseTex.getRes().y
						,blueNoiseTexOpt.paths.size());

					glUniform1f(glpRT.getUnfLoc("u_time"), time);
					//glUniform2f(glpRT.getUnfLoc("u_resolution"), width, height);
					glUniform2f(glpRT.getUnfLoc("u_resolution"), fbRTopt.res.x, fbRTopt.res.y);

					glm::vec3 camPos = m_cam.getPos(); 
					glUniform3fv(glpRT.getUnfLoc("u_cPos"), 1, &camPos[0]);
					glm::mat4 invVP = glm::inverse(p * v);
					glUniformMatrix4fv(glpRT.getUnfLoc("u_invVP"), 1, GL_FALSE, &invVP[0][0]);
					glUniformMatrix4fv(glpRT.getUnfLoc("u_m"), 1, GL_FALSE, &m[0][0]);
					
					static mat4 prevVP = glm::identity<mat4>();
					glUniformMatrix4fv(glpRT.getUnfLoc("u_prevVP"), 1, GL_FALSE, &prevVP[0][0]);
					prevVP = p * v;

					// accumulate
					static vec3 prevDir;
					static vec3 prevPos;
					if (!all(epsilonEqual(m_cam.getDir(),prevDir,epsilon<float>()*2.f))
						|| !all(epsilonEqual(m_cam.getPos(),prevPos,epsilon<float>()*2.f))) {
						glUniform1f(glpRT.getUnfLoc("u_camChange"), 1.f);
						frame = -10;
					}
					else
						glUniform1f(glpRT.getUnfLoc("u_camChange"), 0.f);
					prevDir = m_cam.getDir();
					prevPos = m_cam.getPos();
					glUniform1f(glpRT.getUnfLoc("u_frame"), frame);
				}
				//fbRT.resize(m_winRes);

				{ // bind buffers
					//glBindImageTexture(0, fbRT.getTexCol(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

					glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, bvhSSBOnode);
					glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, bvhSSBOidx);
					//glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 2, bvhSSBOtri);
					glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, sponza.glVb->getVBO());
					glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 3, bvhSSBOmat);
				}

				{ // bind texture  uniforms
#if EHJ_BINDLESS_TEX // bindless texture, does not work on igpu
					glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, bindlessTexSSBO);
#else // uniform array, limited to 80
					for (int i = 0; i < sponza.glTextures.size(); ++i) {
						glActiveTexture(GL_TEXTURE0 + i + 2); // already use 2 tex
						glBindTexture(GL_TEXTURE_2D, sponza.glTextures[i]->getTex());
						std::string uniformName = "u_SceneTextures[" + std::to_string(i) + "]";
						GLint loc = glGetUniformLocation(glpRT.getID(), uniformName.c_str());
						glUniform1i(loc, i+2); // bind texture array slot i
					}
#endif
					// only use rotation part
					mat3 u_imtnu3 = mat3(u_imtn);
					//u_imtnu3 = glm::identity<mat3>();
					glUniformMatrix3fv(glpRT.getUnfLoc("u_imtn"), 1, GL_FALSE, &u_imtnu3[0][0]);
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

			//if (false) { // rasterize
			if (frame < 0) { // rasterize
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

				ssmGlVb.bind(0);
				ssmGl.bind();
				glDrawElements(GL_TRIANGLES,ssmGl.getEBOsize(),GL_UNSIGNED_INT,0);
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
