#pragma once

#include "Mesh.hpp"

namespace ehj {

struct Model {
	Model();
	Model(std::string path);

	std::vector<ehj::Mesh> m_meshes;
	std::vector<int> m_meshesMaterialIDs;

	struct Material {
		glm::vec4 baseColorFactor = glm::vec4(1.0f);
		int baseColorTextureIndex = -1;
		float metallicFactor = 1.0f;
		float roughnessFactor = 1.0f;

		int metallicRoughnessTextureIndex = -1;
		int normalTextureIndex = -1;
		int occlusionTextureIndex = -1;
		int emissiveTextureIndex = -1;

		glm::vec3 emissiveFactor = glm::vec3(0.0f);
		float alphaCutoff = 0.5f;
		bool isDoubleSided = false;
		enum AlphaMode {
			AM_OPAQ,
			AM_MASK,
			AM_BLEND
		} alphaMode = AlphaMode::AM_OPAQ;
	};
	
	VertexData m_vertexData;
	std::vector<Material> m_materials;

	struct TextureInfo {
		std::string path;
		enum class Filter {
			Nearest,
			Linear
		};
		Filter minFilter = Filter::Linear;
		Filter magFilter = Filter::Linear;

		enum class WrapMode {
			Repeat,
			MirroredRepeat,
			ClampToEdge,
			ClampToBorder
		};
		WrapMode wrapS = WrapMode::Repeat;
		WrapMode wrapT = WrapMode::Repeat;
	};
	std::vector<TextureInfo> m_textureInfos;

	void loadOBJ(std::string path);
	void loadGltf(const std::string& path);

	void assembleVertexBuffer() {
		std::vector<Mesh*> msp; for (Mesh& m : m_meshes) msp.push_back(&m);
		m_vertexData.assembleVertexBuffer(msp);
	}
};

}//ehj
