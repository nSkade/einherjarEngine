
#include "GAPI/OGL/GLMesh.hpp"
#include <suOGL.hpp>

using namespace ehj;

struct GLEntity {
	Model model;
	std::vector<std::unique_ptr<GLMesh>> glMeshes;
	std::unique_ptr<GLVertexBuffer> glVb;
	std::vector<std::unique_ptr<GLTexture>> glTextures;

	struct Opt {
		std::string path;
		GLint textureAlbedoInternalFormat=GL_RGBA8;
	};

	void load(std::string path) {
		Opt o;
		o.path = path;
		load(o);
	}

	void load(Opt opt) {
		model = Model(opt.path);
		model.assembleVertexBuffer();
		
		glMeshes.reserve(model.m_meshes.size());
		for (Mesh& m : model.m_meshes)
			glMeshes.emplace_back(std::make_unique<GLMesh>(m));
		
		glVb = std::make_unique<GLVertexBuffer>(model.m_vertexData);
		glVb->bind(0);

		std::vector<GLTexture::Opt> textureLoadOpts(model.m_textureInfos.size());

		// collect info which textures are albedo using materials
		std::vector<bool> textureIsAlbedo(model.m_textureInfos.size(),false);
		for (int i=0;i<model.m_materials.size();++i)
			if (model.m_materials[i].baseColorTextureIndex != -1)
				textureIsAlbedo[model.m_materials[i].baseColorTextureIndex]=true;
		
		{ // load textures
			std::vector<std::thread> loadThreads;
			for (int i=0;i<model.m_textureInfos.size();++i) {
				GLTexture::Opt& o = textureLoadOpts[i];
				loadThreads.emplace_back([this,&o, i,&opt,&textureIsAlbedo](){
					o.texturefilter=GL_LINEAR;
					if (textureIsAlbedo[i])
						o.internalformat=opt.textureAlbedoInternalFormat;
					else {
						o.internalformat=GL_RGBA8;
						//o.internalformat=GL_COMPRESSED_RGBA; // takes significantly longer to load
					}
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

		for (auto& o : textureLoadOpts)
			glTextures.emplace_back(std::make_unique<GLTexture>(o));
	}

	void draw() {
		glVb->bind(0);
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
	}
};