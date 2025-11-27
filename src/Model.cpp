#include "Model.hpp"

#include <rapidobj/rapidobj.hpp>

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>

#include <iostream> //TODO

namespace ehj {

Model::Model(std::string path) {
	//TODO split for 2 support
	loadGltf(path);
	//loadOBJ(path);
}

// Helper lambdas to convert fastgltf types to glm types (and add a 4th component)
auto to_glm_vec4_pos = [](const fastgltf::math::fvec3& v) -> glm::vec4 {
	return glm::vec4(v.x(), v.y(), v.z(), 1.0f); // Position gets w=1.0
};
auto to_glm_vec4_uv = [](const fastgltf::math::fvec2& v) -> glm::vec4 {
	return glm::vec4(v.x(), v.y(), 0.0f, 0.0f); // UV gets z=0.0, w=0.0
};
auto to_glm_vec4_normal = [](const fastgltf::math::fvec3& v) -> glm::vec4 {
	return glm::vec4(v.x(), v.y(), v.z(), 0.0f); // Normal gets w=0.0 (directional vector)
};

void Model::loadGltf(const std::string& path) {
	std::filesystem::path fsPath(path);
	if (!std::filesystem::exists(fsPath)) {
		std::cout << "Failed to find " << path << '\n';
		return;
	}

	std::cout << "Loading " << path << '\n';

	// --- glTF Loading setup ---
	static constexpr auto supportedExtensions =
		fastgltf::Extensions::KHR_mesh_quantization |
		fastgltf::Extensions::KHR_texture_transform |
		fastgltf::Extensions::KHR_materials_variants;

	fastgltf::Parser parser(supportedExtensions);

	constexpr auto gltfOptions =
		fastgltf::Options::DontRequireValidAssetMember |
		fastgltf::Options::LoadExternalBuffers |
		fastgltf::Options::LoadExternalImages |
		fastgltf::Options::GenerateMeshIndices;

	auto gltfFile = fastgltf::MappedGltfFile::FromPath(fsPath);
	if (!bool(gltfFile)) {
		std::cerr << "Failed to open glTF file: " << fastgltf::getErrorMessage(gltfFile.error()) << '\n';
		return;
	}

	// 'asset' is an Expected<Asset>, which is not a pointer.
	auto asset = parser.loadGltf(gltfFile.get(), fsPath.parent_path(), gltfOptions);
	if (asset.error() != fastgltf::Error::None) {
		std::cerr << "Failed to load glTF: " << fastgltf::getErrorMessage(asset.error()) << '\n';
		return;
	}
	
	// --- FIX: Extract the Asset object from the Expected wrapper ---
	// Move the loaded Asset object out of the Expected wrapper.
	fastgltf::Asset loadedAsset = std::move(asset.get());

	// --- Mesh Population ---
	
	// Iterate over all meshes in the loaded glTF asset
	for (const auto& gltfMesh : loadedAsset.meshes) {
		Mesh currentMesh;

		// Iterate over all primitives in the current glTF mesh
		for (const auto& gltfPrimitive : gltfMesh.primitives) {
			
			// A primitive must have positions to be valid, so we use it to determine vertexCount.
			const auto* positionIt = gltfPrimitive.findAttribute("POSITION");
			if (positionIt == gltfPrimitive.attributes.end()) {
				std::cerr << "Primitive missing required POSITION attribute.\n";
				continue;
			}

			// Use 'loadedAsset'
			auto& positionAccessor = loadedAsset.accessors[positionIt->accessorIndex];
			if (!positionAccessor.bufferViewIndex.has_value()) continue;
			
			std::size_t vertexCount = positionAccessor.count;
			std::size_t baseIndex = currentMesh.m_vertices.size();

			// Reserve/Resize for all attributes (positions, normals, UVs)
			currentMesh.m_vertices.reserve(currentMesh.m_vertices.size() + vertexCount);
			currentMesh.m_normals.reserve(currentMesh.m_normals.size() + vertexCount); 
			currentMesh.m_texUVs.reserve(currentMesh.m_texUVs.size() + vertexCount);
			
			currentMesh.m_vertices.resize(baseIndex + vertexCount);
			currentMesh.m_normals.resize(baseIndex + vertexCount);
			currentMesh.m_texUVs.resize(baseIndex + vertexCount);

			// 1. **Positions (m_vertices)**
			fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(loadedAsset, positionAccessor, 
				[&](fastgltf::math::fvec3 pos, std::size_t idx) {
					currentMesh.m_vertices[baseIndex + idx] = to_glm_vec4_pos(pos);
				});

			// 2. **Normals (m_normals)**
			const auto* normalIt = gltfPrimitive.findAttribute("NORMAL");
			if (normalIt != gltfPrimitive.attributes.end()) {
				auto& normalAccessor = loadedAsset.accessors[normalIt->accessorIndex];
				if (normalAccessor.bufferViewIndex.has_value()) {
					// Populate m_normals
					fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(loadedAsset, normalAccessor, 
						[&](fastgltf::math::fvec3 normal, std::size_t idx) {
							currentMesh.m_normals[baseIndex + idx] = to_glm_vec4_normal(normal);
						});
				}
			}

			// 3. **UV Coordinates (m_texUVs)**
			std::string texcoordAttribute = "TEXCOORD_0";
			const auto* texcoordIt = gltfPrimitive.findAttribute(texcoordAttribute);
			
			if (texcoordIt != gltfPrimitive.attributes.end()) {
				// Use 'loadedAsset'
				auto& texCoordAccessor = loadedAsset.accessors[texcoordIt->accessorIndex];
				if (texCoordAccessor.bufferViewIndex.has_value()) {
					// Populate m_texUVs
					fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>(loadedAsset, texCoordAccessor, 
						[&](fastgltf::math::fvec2 uv, std::size_t idx) {
							currentMesh.m_texUVs[baseIndex + idx] = to_glm_vec4_uv(uv);
						});
				}
			}
			
			// 4. **Indices (m_faces)**
			if (!gltfPrimitive.indicesAccessor.has_value()) continue;

			// Use 'loadedAsset'
			auto& indexAccessor = loadedAsset.accessors[gltfPrimitive.indicesAccessor.value()];
			if (!indexAccessor.bufferViewIndex.has_value()) continue;
			
			if (indexAccessor.count % 3 != 0) {
				 std::cerr << "Warning: Index count is not a multiple of 3 (non-triangle mesh?)\n";
			}
			std::size_t faceCount = indexAccessor.count / 3;
			currentMesh.m_faces.reserve(currentMesh.m_faces.size() + faceCount);

			std::vector<std::uint32_t> indices(indexAccessor.count);
			
			if (indexAccessor.componentType == fastgltf::ComponentType::UnsignedByte || 
				indexAccessor.componentType == fastgltf::ComponentType::UnsignedShort) 
			{
				std::vector<std::uint16_t> tempIndices(indexAccessor.count);
				// PASS 'loadedAsset' BY REFERENCE
				fastgltf::copyFromAccessor<std::uint16_t>(loadedAsset, indexAccessor, tempIndices.data());
				
				for (std::size_t i = 0; i < indexAccessor.count; ++i) {
					indices[i] = tempIndices[i];
				}
			} else { // fastgltf::ComponentType::UnsignedInt
				// PASS 'loadedAsset' BY REFERENCE
				fastgltf::copyFromAccessor<std::uint32_t>(loadedAsset, indexAccessor, indices.data());
			}
			
			// Populate m_faces
			for (std::size_t i = 0; i < faceCount; ++i) {
				Mesh::Face face;
				// Vertices are relative to the start of the entire Mesh's vertex array
				face.vertsI.x = static_cast<int>(baseIndex + indices[i * 3 + 0]);
				face.vertsI.y = static_cast<int>(baseIndex + indices[i * 3 + 1]);
				face.vertsI.z = static_cast<int>(baseIndex + indices[i * 3 + 2]);
				face.vertsI.w = -1;

				// Populate normal indices (same as position)
				face.normalI.x = face.vertsI.x;
				face.normalI.y = face.vertsI.y;
				face.normalI.z = face.vertsI.z;
				face.normalI.w = -1;
				
				// Populate UV indices (same as position)
				face.texUVI.x = face.vertsI.x;
				face.texUVI.y = face.vertsI.y;
				face.texUVI.z = face.vertsI.z;
				face.texUVI.w = -1;

				currentMesh.m_faces.push_back(face);
			}
		} // end gltfPrimitive loop

		currentMesh.m_MP |= Mesh::MP_UV;
		currentMesh.m_MP |= Mesh::MP_NORMAL;
		m_meshes.emplace_back(std::move(currentMesh));
	} // end gltfMesh loop
}

void Model::loadOBJ(std::string path) {
	rapidobj::Result result = rapidobj::ParseFile(path);
	if (result.error) {
		return;
	}
	
	const rapidobj::Attributes& attrib = result.attributes;
	
	std::vector<glm::vec4> vertices;
	std::vector<glm::vec4> normals;
	std::vector<glm::vec4> colors;
	std::vector<glm::vec4> texUVs;
	uint32_t mp; // mesh properties

	if (result.shapes[0].mesh.num_face_vertices[0]==4)
		mp |= Mesh::MP_QUAD;

	vertices.reserve(attrib.positions.size() / 3);
	for (size_t i = 0; i < attrib.positions.size(); i += 3) {
		const auto& v = attrib.positions;
		vertices.emplace_back(v[i + 0], v[i + 1], v[i + 2], 1.);
	}

	normals.reserve(attrib.normals.size() / 3);
	for (size_t i = 0; i < attrib.normals.size(); i += 3) {
		const auto& n = attrib.normals;
		normals.emplace_back(n[i + 0], n[i + 1], n[i + 2], 1.);
	}
	if (!attrib.normals.empty()) {
		mp |= Mesh::MP_NORMAL;
	}

	texUVs.reserve(attrib.texcoords.size() / 2);
	for (size_t i = 0; i < attrib.texcoords.size(); i += 2) {
		const auto& t = attrib.texcoords;
		texUVs.emplace_back(t[i + 0], t[i + 1], 0., 0.);
	}
	if (!attrib.texcoords.empty()) {
		mp |= Mesh::MP_UV;
	}
	
	
	for (const auto& shape : result.shapes) {
		ehj::Mesh mesh;
		size_t index_offset = 0;

		mesh.m_vertices=vertices;
		mesh.m_normals=normals;
		mesh.m_colors=colors;
		mesh.m_texUVs=texUVs;
		mesh.m_MP=mp;
		for (size_t fv_count : shape.mesh.num_face_vertices) {
			Mesh::Face newFace;
			for (size_t v = 0; v < fv_count; v++) {
				const rapidobj::Index idx = shape.mesh.indices[index_offset + v];
				if (v < 4) {
					newFace.vertsI[v] = idx.position_index;
					newFace.normalI[v] = idx.normal_index;
					newFace.texUVI[v] = idx.texcoord_index;
				}
			}
			mesh.m_faces.push_back(newFace);
			
			//m_vertices.reserve(attrib.positions.size() / 3);
			//for (size_t i = 0; i < attrib.positions.size(); i += 3) {
			//	const auto& v = attrib.positions;
			//	m_vertices.emplace_back(v[i + 0], v[i + 1], v[i + 2], 1.);
			//}

			//m_normals.reserve(attrib.normals.size() / 3);
			//for (size_t i = 0; i < attrib.normals.size(); i += 3) {
			//	const auto& n = attrib.normals;
			//	m_normals.emplace_back(n[i + 0], n[i + 1], n[i + 2], 1.);
			//}
			//if (!attrib.normals.empty()) {
			//	m_MP |= Mesh::MP_NORMAL;
			//}

			//m_texUVs.reserve(attrib.texcoords.size() / 2);
			//for (size_t i = 0; i < attrib.texcoords.size(); i += 2) {
			//	const auto& t = attrib.texcoords;
			//	m_texUVs.emplace_back(t[i + 0], t[i + 1], 0., 0.);
			//}
			//if (!attrib.texcoords.empty()) {
			//	m_MP |= Mesh::MP_UV;
			//}

		}

		m_meshes.push_back(mesh);
		// Store mesh
		//TODO materialMap[shape.material_name] = meshes.size() - 1; // map material name to mesh index
	}
}

}//ehj
