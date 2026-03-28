#include "Model.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <rapidobj/rapidobj.hpp>

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>

#include <stdexcept>

namespace ehj {

using namespace glm;

Model::Model(std::string path) {
	//TODO split for 2 support
	loadGltf(path);
	//loadOBJ(path);
}

void Model::loadGltf(const std::string& path) {
	auto to_glm_vec4_pos = [](const fastgltf::math::fvec3& v) -> vec4 {
		return glm::vec4(v.x(), v.y(), v.z(), 1.0f);
	};
	auto to_glm_vec4_uv = [](const fastgltf::math::fvec2& v) -> vec4 {
		return glm::vec4(v.x(), v.y(), 0.0f, 0.0f);
	};
	auto to_glm_vec4_normal = [](const fastgltf::math::fvec3& v) -> vec4 {
		return glm::vec4(v.x(), v.y(), v.z(), 0.0f);
	};
	
	std::filesystem::path fsPath(path);
	//if (!std::filesystem::exists(fsPath)) {
	//	std::cout << "Failed to find " << path << '\n';
	//	return;
	//}

	static constexpr auto supportedExtensions =
		fastgltf::Extensions::KHR_mesh_quantization |
		fastgltf::Extensions::KHR_texture_transform |
		fastgltf::Extensions::KHR_materials_variants;

	fastgltf::Parser parser(supportedExtensions);

	constexpr auto gltfOptions =
		fastgltf::Options::DontRequireValidAssetMember
		| fastgltf::Options::LoadExternalBuffers
		//| fastgltf::Options::LoadExternalImages //TODO I am using stb instead (GLTexture)
		| fastgltf::Options::GenerateMeshIndices;

	fastgltf::Expected<fastgltf::MappedGltfFile> gltfFile = fastgltf::MappedGltfFile::FromPath(fsPath);
	if (!bool(gltfFile)) {
		std::cerr << "Failed to open glTF file: " << fastgltf::getErrorMessage(gltfFile.error()) << '\n';
		return;
	}
	
	fastgltf::Expected<fastgltf::Asset> asset = parser.loadGltf(gltfFile.get(), fsPath.parent_path(), gltfOptions);
	if (asset.error() != fastgltf::Error::None) {
		std::cerr << "Failed to load glTF: " << fastgltf::getErrorMessage(asset.error()) << '\n';
		return;
	}
	
	fastgltf::Asset loadedAsset = std::move(asset.get());

	m_materials.reserve(loadedAsset.materials.size());

	for (const auto& gltfMaterial : loadedAsset.materials) {
		Model::Material material;
		
		const auto& pbr = gltfMaterial.pbrData;
			
		material.baseColorFactor = glm::make_vec4(pbr.baseColorFactor.data());
		material.metallicFactor = pbr.metallicFactor;
		material.roughnessFactor = pbr.roughnessFactor;
		
		if (pbr.baseColorTexture.has_value()) {
			material.baseColorTextureIndex = static_cast<int>(pbr.baseColorTexture.value().textureIndex);
		}
		
		if (pbr.metallicRoughnessTexture.has_value()) {
			material.metallicRoughnessTextureIndex = static_cast<int>(pbr.metallicRoughnessTexture.value().textureIndex);
		}
		
		if (gltfMaterial.normalTexture.has_value()) {
			material.normalTextureIndex = static_cast<int>(gltfMaterial.normalTexture.value().textureIndex);
		}
		
		if (gltfMaterial.occlusionTexture.has_value()) {
			material.occlusionTextureIndex = static_cast<int>(gltfMaterial.occlusionTexture.value().textureIndex);
		}

		if (gltfMaterial.emissiveTexture.has_value()) {
			material.emissiveTextureIndex = static_cast<int>(gltfMaterial.emissiveTexture.value().textureIndex);
		}
		material.emissiveFactor = glm::make_vec3(gltfMaterial.emissiveFactor.data());

		material.isDoubleSided = gltfMaterial.doubleSided;
		material.alphaCutoff = gltfMaterial.alphaCutoff;
		
		switch (gltfMaterial.alphaMode) {
			case fastgltf::AlphaMode::Opaque:
				material.alphaMode = Model::Material::AlphaMode::AM_OPAQ;
				break;
			case fastgltf::AlphaMode::Mask:
				material.alphaMode = Model::Material::AlphaMode::AM_MASK;
				break;
			case fastgltf::AlphaMode::Blend:
				material.alphaMode = Model::Material::AlphaMode::AM_BLEND;
				break;
			default:
				material.alphaMode = Model::Material::AlphaMode::AM_OPAQ;
				break;
		}
		
		m_materials.push_back(std::move(material));
	}

	//TODO remove
	//m_textureFilePaths.reserve(loadedAsset.images.size());

	m_textureInfos.clear();
	m_textureInfos.reserve(loadedAsset.textures.size());

	for (const auto& gltfTexture : loadedAsset.textures) {
		TextureInfo info;

		// 1. Get the Path via the Image index
		if (gltfTexture.imageIndex.has_value()) {
			const auto& image = loadedAsset.images[gltfTexture.imageIndex.value()];
			
			if (auto* uriSource = std::get_if<fastgltf::sources::URI>(&image.data)) {
				 // Resolve relative path based on the .gltf location
				 auto fullPath = fsPath.parent_path() / uriSource->uri.path();
				 info.path = fullPath.string();
			}
		}

		// 2. Map Sampler Settings
		if (gltfTexture.samplerIndex.has_value()) {
			const auto& sampler = loadedAsset.samplers[gltfTexture.samplerIndex.value()];

			// Map Filters (Handling min/mag separately as you requested)
			if (sampler.magFilter.has_value()) {
				info.magFilter = (sampler.magFilter.value() == fastgltf::Filter::Nearest) 
					? TextureInfo::Filter::Nearest : TextureInfo::Filter::Linear;
			}

			if (sampler.minFilter.has_value()) {
				auto gltfMin = sampler.minFilter.value();
				// glTF has 4 mipmap variants for minFilter; we simplify to your Linear/Nearest
				bool isNearest = (gltfMin == fastgltf::Filter::Nearest || 
								  gltfMin == fastgltf::Filter::NearestMipMapNearest || 
								  gltfMin == fastgltf::Filter::NearestMipMapLinear);
				
				info.minFilter = isNearest ? TextureInfo::Filter::Nearest : TextureInfo::Filter::Linear;
			}

			// Map Wrap Modes
			auto mapWrap = [](fastgltf::Wrap mode) {
				switch (mode) {
					case fastgltf::Wrap::ClampToEdge:    return TextureInfo::WrapMode::ClampToEdge;
					case fastgltf::Wrap::MirroredRepeat: return TextureInfo::WrapMode::MirroredRepeat;
					case fastgltf::Wrap::Repeat:         return TextureInfo::WrapMode::Repeat;
					default:                             return TextureInfo::WrapMode::Repeat;
				}
			};

			info.wrapS = mapWrap(sampler.wrapS);
			info.wrapT = mapWrap(sampler.wrapT);
		}

		m_textureInfos.push_back(std::move(info));

		//std::visit(fastgltf::visitor {
		//	[](const std::monostate&) {
		//		std::cout << "[Error] Image data is EMPTY (monostate). The parser couldn't find the file or skipped it.\n";
		//	},
		//	[](const fastgltf::sources::URI& uriSource) {
		//		std::cout << "URI/Path: " << uriSource.uri.path() << "\n";
		//	},
		//	[](const fastgltf::sources::Vector& vectorSource) {
		//		std::cout << "Data in Vector. Size: " << vectorSource.bytes.size() << "\n";
		//	},
		//	[](const fastgltf::sources::Array& arraySource) {
		//		// THIS is likely where Sponza is hiding if you used LoadExternalImages
		//		std::cout << "Data in Array. Size: " << arraySource.bytes.size() << "\n";
		//	},
		//	[](const fastgltf::sources::BufferView& view) {
		//		std::cout << "Data in BufferView (GLB style).\n";
		//	},
		//	[](const auto& other) {
		//		// This will print the internal index of the variant type to help us identify it
		//		// 0 = monostate, 2 = URI, 4 = Vector, etc.
		//		// Look at fastgltf/types.hpp to match the index if this triggers.
		//		// std::variant::index() is very useful here.
		//		// Note: we can't easily print 'other' directly, but we know it's one of the types.
		//		std::cout << "Hit unhandled variant type.\n";
		//	}
		//}, gltfImage.data);
	}
	
	for (const auto& gltfMesh : loadedAsset.meshes) {
		for (const auto& gltfPrimitive : gltfMesh.primitives) {
			Mesh mesh;

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
			std::size_t baseIndex = m_vertexData.positions.size();
			assert(baseIndex==m_vertexData.normals.size());
			assert(baseIndex==m_vertexData.texUVs.size());

			// Reserve/Resize for all attributes (positions, normals, UVs)
			auto& positions=m_vertexData.positions;
			auto& normals=m_vertexData.normals;
			auto& texUVs=m_vertexData.texUVs;

			positions.resize(baseIndex + vertexCount);
			normals.resize(baseIndex + vertexCount);
			texUVs.resize(baseIndex + vertexCount);

			// 1. **Positions (m_vertices)**
			fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(loadedAsset, positionAccessor, 
				[&](fastgltf::math::fvec3 pos, std::size_t idx) {
					positions[baseIndex + idx] = to_glm_vec4_pos(pos);
				});

			// 2. **Normals (m_normals)**
			const auto* normalIt = gltfPrimitive.findAttribute("NORMAL");
			if (normalIt != gltfPrimitive.attributes.end()) {
				auto& normalAccessor = loadedAsset.accessors[normalIt->accessorIndex];
				if (normalAccessor.bufferViewIndex.has_value()) {
					// Populate m_normals
					fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(loadedAsset, normalAccessor, 
						[&](fastgltf::math::fvec3 normal, std::size_t idx) {
							normals[baseIndex + idx] = to_glm_vec4_normal(normal);
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
							texUVs[baseIndex + idx] = to_glm_vec4_uv(uv);
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
			mesh.m_faces.reserve(mesh.m_faces.size() + faceCount);

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
				ehj::Face f;
				// Vertices are relative to the start of the entire Mesh's vertex array
				f.possI.x = static_cast<int>(baseIndex + indices[i * 3 + 0]);
				f.possI.y = static_cast<int>(baseIndex + indices[i * 3 + 1]);
				f.possI.z = static_cast<int>(baseIndex + indices[i * 3 + 2]);
				f.possI.w = -1;

				// Populate normal indices (same as position)
				f.normalI.x = f.possI.x;
				f.normalI.y = f.possI.y;
				f.normalI.z = f.possI.z;
				f.normalI.w = -1;
				
				// Populate UV indices (same as position)
				f.texUVI.x = f.possI.x;
				f.texUVI.y = f.possI.y;
				f.texUVI.z = f.possI.z;
				f.texUVI.w = -1;

				mesh.m_faces.push_back(f);
			}
			if (gltfPrimitive.materialIndex.has_value())
				m_meshesMaterialIDs.push_back(gltfPrimitive.materialIndex.value());
			else
				m_meshesMaterialIDs.push_back(-1);
			m_meshes.emplace_back(std::move(mesh));
		} // for primitives

		m_vertexData.m_VP |= VertexData::VP_UV;
		m_vertexData.m_VP |= VertexData::VP_NORMAL;
	} // for meshes
}

void Model::loadOBJ(std::string path) {
	throw std::runtime_error("not yet implemented");

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
		mp |= VertexData::VP_NORMAL;
	}

	texUVs.reserve(attrib.texcoords.size() / 2);
	for (size_t i = 0; i < attrib.texcoords.size(); i += 2) {
		const auto& t = attrib.texcoords;
		texUVs.emplace_back(t[i + 0], t[i + 1], 0., 0.);
	}
	if (!attrib.texcoords.empty()) {
		mp |= VertexData::VP_UV;
	}
	
	
	for (const auto& shape : result.shapes) {
		ehj::Mesh mesh;
		size_t index_offset = 0;

		//TODO
		//mesh.m_vertices=vertices;
		//mesh.m_normals=normals;
		//mesh.m_colors=colors;
		//mesh.m_texUVs=texUVs;
		//mesh.m_MP=mp;

		for (size_t fv_count : shape.mesh.num_face_vertices) {
			ehj::Face newFace;
			for (size_t v = 0; v < fv_count; v++) {
				const rapidobj::Index idx = shape.mesh.indices[index_offset + v];
				if (v < 4) {
					//TODO
					//newFace.possI[v] = idx.position_index;
					//newFace.normalI[v] = idx.normal_index;
					//newFace.texUVI[v] = idx.texcoord_index;
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
