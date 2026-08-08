#include "GAPI/OGL/GLMesh.hpp"
#include "../bvh/bvh.hpp"

#include <GLFW/glfw3.h>
#include <suOGL.hpp>
#include <Input/GLFW/GLFWKeyboardCache.hpp>
#include <Utility/ConfigFile.hpp>
#include "../../EnvirTexScene/GLEntity.hpp"
#include "suCMN.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace ehj;

void buildBlas(std::vector<uint32_t>& indexBuffer, VertexData& vertexData, tinybvh::BVH_GPU& bvh) {

	std::vector<float> mPos;
	int attribSize = 4;
	attribSize += vertexData.m_VP & VertexData::VP_NRM ? 4 : 0;
	attribSize += vertexData.m_VP & VertexData::VP_COL ? 4 : 0;
	attribSize += vertexData.m_VP & VertexData::VP_UV  ? 4 : 0;
	attribSize += vertexData.m_VP & VertexData::VP_TAN ? 4 : 0;
	
	auto& mVmerg = vertexData.merged;
	for (size_t i = 0; i < mVmerg.size(); i += attribSize) {
		mPos.emplace_back(mVmerg[i]);
		mPos.emplace_back(mVmerg[i+1]);
		mPos.emplace_back(mVmerg[i+2]);
		mPos.emplace_back(0.0f);
	}

	tinybvh::bvhvec4* vertices = (tinybvh::bvhvec4*)tinybvh::malloc64(mPos.size() * sizeof(float));
	for (size_t i = 0; i < mPos.size(); i += 4)
		vertices[i / 4] = tinybvh::bvhvec4(mPos[i], mPos[i+1], mPos[i+2], mPos[i+3]);

	uint* indices = (uint*)tinybvh::malloc64(indexBuffer.size() * sizeof(uint32_t));
	for (size_t i = 0; i < indexBuffer.size(); ++i) {
		indices[i] = indexBuffer[i];
	}

	// 1. Build BLAS
	bvh.Build(vertices, indices, indexBuffer.size() / 3);
}

void buildBlas(Model& model, tinybvh::BVH_GPU& bvh) {
	std::vector<uint32_t> indexBuffer;
	for (auto& m : model.m_meshes) {
		auto ib = m.getIndexBuffer();
		indexBuffer.insert(indexBuffer.end(), ib.begin(), ib.end());
	}
	buildBlas(indexBuffer,model.m_vertexData,bvh);
}

void buildBlas(Mesh& mesh, tinybvh::BVH_GPU& bvh) {
	std::vector<uint32_t> indexBuffer;
	auto ib = mesh.getIndexBuffer();
	indexBuffer.insert(indexBuffer.end(), ib.begin(), ib.end());
	buildBlas(indexBuffer,mesh.m_vertexData,bvh);
}

#define SCENETYPE TlasTestScene
class TlasTestScene : IScene {
public:
	~TlasTestScene() {}
	void setup() {
		m_glWindow.setup(&m_configFile, "ehjE TlasTestScene");
	}

	int run() {
		GLEntity sponza;
		sponza.load("myModels/sponza/gltf/Sponza.gltf");

		GLProgram glpRaster;
		auto glpRasterbuild = [&]() -> bool {
			bool r = glpRaster.addSourceFromFileRecursive(std::string("scenes/EnvirTexScene/")+"v.vert");
			r &= glpRaster.addSourceFromFileRecursive(std::string("shaders/")+"basic_f.frag");
			glpRaster.createProgram();
			return r;
		}; glpRasterbuild();

		GLProgram glpRT;
		auto glpRTbuild = [&]() -> bool {
			bool r = glpRT.addSourceFromFileRecursive(EHJ_THIS_FOLDER()+"tlasTest.comp");
			glpRT.createProgram();
			return r;
		}; glpRTbuild();

		float time = 0.0f;

		{
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
		GLuint bvhSSBOnode;
		GLuint bvhSSBOidx;
		GLuint tlasNodeBuf, tlasIdxBuf, tlasInstancesBuf;
		
		// ref https://github.com/jbikker/tinybvh/blob/0e4584287823252cf83f0e9cd072848bec5f79c5/tmpl8/game.cpp#L85
		{
			buildBlas(sponza.model,bvh);

			tinybvh::BVH_GPU tlas;
			int INSTANCE_COUNT = 2;
			const int BLAS_COUNT = 2;
			tinybvh::BLASInstance instance[INSTANCE_COUNT];
			{ // build blas
				instance[0] = tinybvh::BLASInstance(0 /* BLAS index */);
				
				// Set identity transform for instance 0
				memset(&instance[0].transform, 0, sizeof(instance[0].transform));
				instance[0].transform[0] = 1.0f;
				instance[0].transform[5] = 1.0f;
				instance[0].transform[10] = 1.0f;
				instance[0].transform[15] = 1.0f;
				instance[0].dummy[0] = 0;

				// use single triangle blas for implicit spheres
				// in order to let the tlas.Build assign the aabb correctly
				// this is not bound as ssbo
				tinybvh::BVH_GPU sphereBlas;
				tinybvh::bvhvec4 sphereVerts[3] = {
					tinybvh::bvhvec4(-1.0f, -1.0f, -1.0f, 1.0f),
					tinybvh::bvhvec4( 1.0f, -1.0f, -1.0f, 1.0f),
					tinybvh::bvhvec4( 0.0f,  1.0f,  1.0f, 1.0f)
				};
				uint32_t sphereIndices[3] = { 0, 1, 2 };
				sphereBlas.Build(sphereVerts, sphereIndices, 1);
				
				instance[1] = tinybvh::BLASInstance(1 /* BLAS index */);
				
				// single sphere test
				memset(&instance[1].transform, 0, sizeof(instance[1].transform));
				instance[1].transform[0] = 10.f;
				instance[1].transform[5] = 10.f;
				instance[1].transform[10] = 10.f;
				instance[1].transform[15] = 1.0f;
				instance[1].transform[12] = 1000.0f; // TX
				instance[1].transform[13] = 0.0f; // TY
				instance[1].transform[14] = 0.0f; // TZ
				instance[1].InvertTransform();
				instance[1].dummy[0] = 1;

				tinybvh::BVHBase* blasList[BLAS_COUNT] = { &bvh, &sphereBlas};
				tlas.Build(instance, INSTANCE_COUNT, blasList, BLAS_COUNT);
			}

			// Create BLAS Buffers
			glCreateBuffers(1, &bvhSSBOnode);
			glCreateBuffers(1, &bvhSSBOidx);

			glNamedBufferStorage(bvhSSBOnode, bvh.usedNodes * sizeof(tinybvh::BVH_GPU::BVHNode), bvh.bvhNode, GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT);
			glNamedBufferStorage(bvhSSBOidx, bvh.idxCount * sizeof(uint32_t), bvh.bvh.primIdx, GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT);

			// Create TLAS and Instance Buffers
			glCreateBuffers(1, &tlasNodeBuf);
			glCreateBuffers(1, &tlasIdxBuf);
			glCreateBuffers(1, &tlasInstancesBuf);

			glNamedBufferStorage(tlasNodeBuf, tlas.allocatedNodes * sizeof(tinybvh::BVH_GPU::BVHNode), tlas.bvhNode, GL_DYNAMIC_STORAGE_BIT);
			glNamedBufferStorage(tlasIdxBuf, tlas.bvh.idxCount * sizeof(uint32_t), tlas.bvh.primIdx, GL_DYNAMIC_STORAGE_BIT);
			glNamedBufferStorage(tlasInstancesBuf, INSTANCE_COUNT * sizeof(tinybvh::BLASInstance), instance, GL_DYNAMIC_STORAGE_BIT);
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
			m_glWindow.m_winRes = glm::ivec2(width, height);

			m_cam.setProj(glm::perspective(glm::radians(90.0f), std::fmax(0.00001f, (float)width / height), 0.01f, 100.0f));
			mat4 m = glm::scale(mat4(1.0f), vec3(0.005f));

			mat4 p = m_cam.getProj();
			mat4 v = m_cam.getView();

			{ // Dispatch Ray Tracer Compute Shader
				glpRT.bind();
				glUniform1f(glpRT.getUnfLoc("u_time"), time);
				glUniform2f(glpRT.getUnfLoc("u_resolution"), width, height);

				glm::vec3 camPos = m_cam.getPos(); 
				glUniform3fv(glpRT.getUnfLoc("u_cPos"), 1, &camPos[0]);
				mat4 invVP = glm::inverse(p * v);
				glUniformMatrix4fv(glpRT.getUnfLoc("u_invVP"), 1, GL_FALSE, &invVP[0][0]);
				glUniformMatrix4fv(glpRT.getUnfLoc("u_m"), 1, GL_FALSE, &m[0][0]);

				fbRT.resize(m_glWindow.m_winRes);

				glBindImageTexture(0, fbRT.getTexCol(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
				
				// Bind Buffers according to layout bindings
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, tlasNodeBuf);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, tlasIdxBuf);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, tlasInstancesBuf);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, bvhSSBOnode);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, bvhSSBOidx);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, sponza.glVb->getVBO()); // Reusing existing VBO directly!

				ivec3 ls;
				glGetProgramiv(glpRT.getID(), GL_COMPUTE_WORK_GROUP_SIZE, &ls[0]);
				ivec3 ds = (ivec3(width, height, 1) + ls - ivec3(1)) / ls;
				glDispatchCompute(ds[0], ds[1], ds[2]);

				glBindFramebuffer(GL_READ_FRAMEBUFFER, fbRT.getFBO());
				glReadBuffer(GL_COLOR_ATTACHMENT0); 
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				glDrawBuffer(GL_BACK);
				glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
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
	}
private:
	GLFWWindowGL m_glWindow;
	ConfigFile m_configFile;
	std::vector<bool> m_kkTap = std::vector<bool>(IBCodes::KK_COUNT, true);
	FreeFlyCamera m_cam;
	Clock m_clock;
};