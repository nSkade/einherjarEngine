#include "Mesh.hpp"
#include <stdexcept>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include <fstream>
#include <sstream>

//#include <tiny_obj_loader.h>
#include <rapidobj/rapidobj.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

//#include <unordered_map>
#include <numeric> // for reduce

namespace ehj {

void VertexData::computeNormals(std::vector<Face>& faces) {
	if (positions.empty()) return;

	std::size_t vertexCount = positions.size();
	normals.clear();
	normals.resize(vertexCount, glm::vec4(0.0f));

	for (auto& face : faces) {
		face.normalI = face.possI;
	}

	for (const auto& face : faces) {
		int i0 = face.possI.x;
		int i1 = face.possI.y;
		int i2 = face.possI.z;

		if (i0 < 0 || i1 < 0 || i2 < 0 || 
			i0 >= vertexCount || i1 >= vertexCount || i2 >= vertexCount) 
		{
			continue;
		}

		glm::vec3 v0 = positions[i0];
		glm::vec3 v1 = positions[i1];
		glm::vec3 v2 = positions[i2];

		glm::vec3 edge1 = v1 - v0;
		glm::vec3 edge2 = v2 - v0;

		glm::vec3 faceNormal = glm::cross(edge1, edge2);

		normals[i0] += glm::vec4(faceNormal, 0.0f);
		normals[i1] += glm::vec4(faceNormal, 0.0f);
		normals[i2] += glm::vec4(faceNormal, 0.0f);
	}

	for (auto& normalVec4 : normals) {
		if (glm::length2(glm::vec3(normalVec4)) > 1e-6) {
			glm::vec3 normalized = glm::normalize(glm::vec3(normalVec4));
			normalVec4 = glm::vec4(normalized, 0.0f);
		} else {
			normalVec4 = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
		}
	}
}

Mesh::Mesh() {
	
}

Mesh::Mesh(std::string path) {
	loadOBJ(path);
}

void VertexData::clear() {
	positions.clear();
	normals.clear();
	colors.clear();
	texUVs.clear();
	merged.clear();
}

//TODOff implement with rapidobj
void Mesh::storeOBJ(std::string path) {
	if (!m_hasVertexData) {
		throw std::runtime_error("cannot store obj without VertexData");
	}

	//TODO rewrite with rapidobj
	std::ofstream file(path, std::ostream::out | std::ostream::trunc);
	std::string ret;

	auto& vertices = m_vertexData.positions;
	
	for (uint32_t i=0;i<vertices.size();i++) {
		if (i==0)
			ret.append("v ");
		else
			ret.append("\nv ");
		ret.append(std::to_string(vertices[i][0]).append(" "));
		ret.append(std::to_string(vertices[i][1]).append(" "));
		ret.append(std::to_string(vertices[i][2]));
	}

	for (uint32_t i=0;i<m_faces.size();i++) {
		ret.append("\nf ");
		for (uint32_t j=0;j<3;j++) {
			ret.append(std::to_string(m_faces[i].possI[j]+1)).append(" ");
		}
		if (m_MP & MP_QUAD)
			ret.append(std::to_string(m_faces[i].possI[3]+1)).append(" ");
	}
	ret.append("\n");
	file << ret;
}

#if 1 // load using rapidobj
void Mesh::loadOBJ(std::string path) {
	rapidobj::Result result = rapidobj::ParseFile(path);
	if (result.error) {
		return;
	}

	//TODO result.materials[0].diffuse_texname
	
	const rapidobj::Attributes& attrib = result.attributes;

	if (result.shapes[0].mesh.num_face_vertices[0]==4) //TODO
		m_MP |= MP_QUAD;

	m_hasVertexData=true;
	auto& vertices = m_vertexData.positions;
	auto& normals = m_vertexData.normals;
	auto& texUVs = m_vertexData.texUVs;
	
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
		m_vertexData.m_VP |= VertexData::VP_NRM;
	}

	texUVs.reserve(attrib.texcoords.size() / 2);
	for (size_t i = 0; i < attrib.texcoords.size(); i += 2) {
		const auto& t = attrib.texcoords;
		texUVs.emplace_back(t[i + 0], t[i + 1], 0., 0.);
	}
	if (!attrib.texcoords.empty()) {
		m_vertexData.m_VP |= VertexData::VP_UV;
	}

	for (const auto& shape : result.shapes) {
		size_t index_offset = 0;
		for (size_t fv_count : shape.mesh.num_face_vertices) {
			Face newFace;
			for (size_t v = 0; v < fv_count; v++) {
				const rapidobj::Index idx = shape.mesh.indices[index_offset + v];
				if (v < 4) {
					newFace.possI[v] = idx.position_index;
					newFace.normalI[v] = idx.normal_index;
					newFace.texuvI[v] = idx.texcoord_index;
				}
			}
			m_faces.push_back(newFace);
			index_offset += fv_count;
		}
	}
}
#endif
#if 0 // load using tinyObj
void Mesh::loadOBJ(std::string path) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	//std::map<std::string, int> textures;
	
	std::string warn;
	std::string err;

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());
	if (!ret) {
		//TODO error
		return;
	}

	m_vertices.reserve(attrib.vertices.size() / 3);
	for (size_t i = 0; i < attrib.vertices.size(); i += 3) {
		auto& v = attrib.vertices;
		m_vertices.emplace_back(v[i+0],v[i+1],v[i+2],1.);
	}
	
	m_normals.reserve(attrib.normals.size() / 3);
	for (size_t i = 0; i < attrib.normals.size(); i += 3) {
		auto& n = attrib.normals;
		m_normals.emplace_back(n[i+0],n[i+1],n[i+2],1.);
	}
	m_MP |= MP_NORMAL;

	m_texUVs.reserve(attrib.texcoords.size() / 2);
	for (size_t i = 0; i < attrib.texcoords.size(); i += 2) {
		auto& t = attrib.texcoords;
		m_texUVs.emplace_back(t[i+0],t[i+1],0.,0.);
	}
	m_MP |= MP_UV;

	for (const auto& shape : shapes) {
		int io = 0; // index offset
		for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
			size_t fv = (size_t)shape.mesh.num_face_vertices[f];
			Face newFace;
			
			for (size_t v = 0; v < fv; v++) {
				tinyobj::index_t idx = shape.mesh.indices[io + v];

				if (v < 4) {
					newFace.possI[v] = idx.vertex_index;
					newFace.normalI[v] = idx.normal_index;
					newFace.texuvI[v] = idx.texcoord_index;
				}
			}
			
			m_faces.push_back(newFace);
			io += fv;
		}
	}
}
#endif

void Mesh::loadOBJcust(std::string path) {
	std::ifstream file(path, std::ifstream::in);
	std::string line;

	m_hasVertexData=true;
	auto& vertices=m_vertexData.positions;
	auto& normals=m_vertexData.normals;
	auto& texUVs=m_vertexData.texUVs;

	if (file.good()) {
		while (getline(file,line)) {
			if (line[0]=='#') continue; // comment
			else if (line.substr(0,2).compare("vn")==0) { // face normal?
				m_vertexData.m_VP |= VertexData::VP_NRM;
				std::istringstream iss(line);
				std::string n;
				float v1,v2,v3,v4 = 0.0f;
				iss >> n >> v1 >> v2 >> v3;
				normals.emplace_back(v1,v2,v3,v4);
			}
			else if (line.substr(0,2).compare("vt")==0) { // face normal?
				m_vertexData.m_VP |= VertexData::VP_UV;
				std::istringstream iss(line);
				std::string n;
				float v1,v2,v3,v4 = 0.0f;
				iss >> n >> v1 >> v2;
				texUVs.emplace_back(v1,v2,v3,v4);
			}
			else if (line[0]=='v') { //vertex
				std::istringstream iss(line);
				char n;
				float v1,v2,v3,v4 = 0.0f;
				iss >> n >> v1 >> v2 >> v3;
				vertices.emplace_back(v1,v2,v3,v4);
			}
			else if (line[0]=='f') { // face
				std::istringstream iss(line);
				char x;
				int32_t v[4];
				int32_t n[4];
				v[3] = -1;
				// face is vert/tex/nrm vert/tex/nrm vert/tex/nrm

				iss >> x; //TODO
				//for (uint32_t i=0;i<3;++i) {
				//	iss;
				//}

				if (m_vertexData.m_VP & VertexData::VP_NRM)
					iss >> x >> v[0] >> x >> x >> n[0] >> v[1] >> x >> x >> n[1] >> v[2] >> x >> x >> n[2];
				else
					iss >> x >> v[0] >> v[1] >> v[2];

				if (iss.tellg()!=line.length() && iss.tellg()!=-1) { // guys we have a quad!
					throw std::runtime_error("quads not supported");
					//std::cout << iss.tellg() << " " << line.length() << "\n";
					//m_MP |= MP_QUAD;
					//iss >> v[3];
					//if (m_MP & MP_NORMAL)
					//	iss >> x >> x >> n[3];
				}
				Face f;
				for (uint32_t i=0;i<4;++i) {
					f.possI[i] = v[i]-1;
					if (m_vertexData.m_VP & VertexData::VP_NRM)
						f.normalI[i] = n[i]-1;
				}
				m_faces.push_back(f);
			}
		}
		file.close();
	}
}

// using the fat vertex approach
//TODOff  (for a simple one vao vertex buffer we need to look which of the vertex attribute vector is largest and take that as reference)
void VertexData::assembleVertexBuffer(std::vector<Mesh*> ms) {
	std::vector<float>& res = merged;
	res.clear();

	// unify normals and uvs to position
	std::vector<int> nIdxLuNRM(positions.size(), -1);
	std::vector<int> nIdxLuUV(positions.size(), -1);

	//TODO incorporate meshoptimizer here
	// can use index to compress vertex buffer, most of the time probably not worth it
	// std::unordered_map<glm::ivec4, uint32_t> vertex_to_index;

	//TODO make Opt struct to align or not align
	// reserve size
#if 0// unaligned
	int attribSize = m_Dim;
	attribSize += m_VP & VP_NRM ? m_Dim : 0;
	attribSize += m_VP & VP_COL ? 3 : 0;
	attribSize += m_VP & VP_UV  ? 2 : 0;
	attribSize += m_VP & VP_TAN ? 4: 0;
#endif
#if 1 //aligned
	int attribSize = 4;
	attribSize += m_VP & VP_NRM ? 4 : 0;
	attribSize += m_VP & VP_COL ? 4 : 0;
	attribSize += m_VP & VP_UV  ? 4 : 0;
	attribSize += m_VP & VP_TAN ? 4: 0;
#endif
	//res.reserve(positions.size() * resSize);

	std::vector<int> meshTriCount;
	meshTriCount.resize(ms.size());
	for (int i=0;i<ms.size();++i)
		meshTriCount[i]=ms[i]->m_faces.size();

	res.resize(std::reduce(meshTriCount.begin(),meshTriCount.end()) * 3 * attribSize);

	std::vector<int> meshTriCountPrefixSum(meshTriCount.size());
	std::partial_sum(meshTriCount.begin(), meshTriCount.end(), meshTriCountPrefixSum.begin());
	meshTriCountPrefixSum.insert(meshTriCountPrefixSum.begin(),0);

#if 1 // aligned
	for (int mI=0;mI<ms.size(); ++mI) { // for each mesh
		int sI = meshTriCountPrefixSum[mI]*3 * attribSize; // start index
		for (int fI=0;fI<ms[mI]->m_faces.size();++fI) { // for each face
			Face& f = ms[mI]->m_faces[fI];
			for (int vI=0;vI<3;++vI) { // for each face vertex //TODOff could be 4?
				f.mergedI[vI] = meshTriCountPrefixSum[mI]*3 + fI*3 + vI;
				int attribIdx=sI+fI*3*attribSize+vI*attribSize;
				
				int offset=0; // offset inside attrib
				for (int j = 0; j < 3; ++j)
					res[attribIdx+j]=(positions[f.possI[vI]][j]);
				res[attribIdx+3]=0;
				offset+=4;
				if (m_VP & VP_NRM) {
					for (int j = 0; j < 3; ++j)
						res[attribIdx+offset+j]=(normals[f.normalI[vI]][j]);
					res[attribIdx+offset+3]=0;
					offset+=4;
				}
				if (m_VP & VP_COL) {
					for (int j = 0; j < 3; ++j)
						res[attribIdx+offset+j]=(colors[f.colorI[vI]][j]);
					res[attribIdx+offset+3]=0;
					offset+=4;
				}
				if (m_VP & VP_UV) {
					for (int j = 0; j < 2; ++j) //TODOff could be 3?
						res[attribIdx+offset+j]=(texUVs[f.texuvI[vI]][j]);
					for (int j = 2; j < 4; ++j) //TODOff could be 3?
						res[attribIdx+offset+j]=0.;
					offset+=4;
				}
				if (m_VP & VP_TAN) {
					for (int j = 0; j < 4; ++j)
						res[attribIdx+offset+j]=(tangents[f.tangI[vI]][j]);
				}
			} // vertices
		} // faces
	}// mesh
#endif

#if 0 // already improved performance //TODOff maybe parallelize
	for (int mI=0;mI<ms.size(); ++mI) { // for each mesh
		int sI = meshTriCountPrefixSum[mI]*3 * attribSize; // start index
		for (int fI=0;fI<ms[mI]->m_faces.size();++fI) { // for each face
			Face& f = ms[mI]->m_faces[fI];
			for (int vI=0;vI<3;++vI) { // for each face vertex //TODOff could be 4?
				f.mergedI[vI] = meshTriCountPrefixSum[mI]*3 + fI*3 + vI;
				int attribIdx=sI+fI*3*attribSize+vI*attribSize;
				
				int offset=0; // offset inside attrib
				for (int j = 0; j < m_Dim; ++j)
					res[attribIdx+j]=(positions[f.possI[vI]][j]);
				offset+=m_Dim;
				if (m_VP & VP_NRM) {
					for (int j = 0; j < m_Dim; ++j)
						res[attribIdx+offset+j]=(normals[f.normalI[vI]][j]);
					offset+=m_Dim;
				}
				if (m_VP & VP_COL) {
					for (int j = 0; j < 3; ++j)
						res[attribIdx+offset+j]=(colors[f.colorI[vI]][j]);
					offset+=3;
				}
				if (m_VP & VP_UV) {
					for (int j = 0; j < 2; ++j) //TODOff could be 3?
						res[attribIdx+offset+j]=(texUVs[f.texuvI[vI]][j]);
					offset+=2;
				}
				if (m_VP & VP_TAN) {
					for (int j = 0; j < 4; ++j)
						res[attribIdx+offset+j]=(tangents[f.tangI[vI]][j]);
				}
			} // vertices
		} // faces
	}// mesh
#endif
#if 0
	int vertId=0;
	//int total=0;
	for (Mesh* m : ms) {
		for (Face& f : m->m_faces) {
			for (int i=0;i<4;++i) {
				//total++;
				glm::ivec4 key(f.possI[i], f.normalI[i], f.texuvI[i], f.colorI[i]);
				//if (vertex_to_index.find(key) == vertex_to_index.end()) { // new unique vertex
					f.mergedI[i] = vertId;
					
					for (uint32_t j = 0; j < m_Dim; ++j) {
						res.emplace_back(positions[key.x][j]);
					}
					if (m_VP & VP_NRM) {
						for (uint32_t j = 0; j < m_Dim; ++j)
							res.emplace_back(normals[key.y][j]);
					}
					if (m_VP & VP_COL) {
						for (uint32_t j = 0; j < m_Dim; ++j)
							res.emplace_back(colors[key.w][j]);
					}
					if (m_VP & VP_UV) {
						for (uint32_t j = 0; j < 2; ++j) //TODO could be 3
							res.emplace_back(texUVs[key.z][j]);
					}
					if (m_VP & VP_TAN) {
						for (uint32_t j = 0; j < 4; ++j)
							res.emplace_back(tangents[f.tangI[i]][j]);
					}
					//vertex_to_index[key]=vertId;
					vertId++;
				//} else { // vertex already indexed
				//	f.mergedI[i] = vertex_to_index[key];
				//}
			} // vertices
		} // faces
	}// mesh
#endif
	//std::cout << "new vert count: " << vertId+1 << std::endl;
	//std::cout << "old vert count: " << total+1 << std::endl;
	//std::cout << "%  " << float(vertId+1)/(total+1)*100. << std::endl;
}

std::vector<int> Mesh::getIndexBuffer() {
	std::vector<int> res;
	for (uint32_t i=0;i<m_faces.size();++i) {
		uint32_t s = 3; // triangle std
		if (m_MP & MP_QUAD)
			s = 4;
		for (uint32_t j=0;j<s;++j) {
			res.emplace_back(m_faces[i].mergedI[j]);
		}
	}
	return res;
}

void Mesh::toTriangles() {
	if (!(m_MP & MP_QUAD))
		return;
	Mesh mesh = Mesh(*this);
	m_faces.clear();
	for (uint32_t i=0;i<mesh.m_faces.size();++i) {
		//std::cout << "toTriangles: " << i << "/" << mesh.m_faces.size() << " " << float(i)/mesh.m_faces.size()*100. << "%" << std::endl;
		Face fq = mesh.m_faces[i];
		Face t1,t2;
		t1.possI = {fq.possI[0],fq.possI[1],fq.possI[2],-1};
		t2.possI = {fq.possI[2],fq.possI[3],fq.possI[0],-1};

		t1.normalI = {fq.normalI[0],fq.normalI[1],fq.normalI[2],-1};
		t2.normalI = {fq.normalI[2],fq.normalI[3],fq.normalI[0],-1};

		t1.colorI = {fq.colorI[0],fq.colorI[1],fq.colorI[2],-1};
		t2.colorI = {fq.colorI[2],fq.colorI[3],fq.colorI[0],-1};

		t1.texuvI = {fq.texuvI[0],fq.texuvI[1],fq.texuvI[2],-1};
		t2.texuvI = {fq.texuvI[2],fq.texuvI[3],fq.texuvI[0],-1};

		this->m_faces.push_back(t1);
		this->m_faces.push_back(t2);
	}
	m_MP &= ~MP_QUAD;
}

SSMesh::SSMesh(bool triangles) {
	m_vertexData.m_VP |= VertexData::VP_NRM;
	m_vertexData.m_VP |= VertexData::VP_UV;
	m_hasVertexData=true;
	if (triangles) {
		static const struct
		{
			float x, y;
			float u, v;
		} vertices[6] =
		{
			{ -1.0f, -1.0f, 0.f, 0.f },
			{  1.0f,  1.0f, 1.f, 1.f },
			{ -1.0f,  1.0f, 0.f, 1.f },
			{ -1.0f, -1.0f, 0.f, 0.f },
			{  1.0f, -1.0f, 1.f, 0.f },
			{  1.0f,  1.0f, 1.f, 1.f }
		};
		
		static const int indices[6] = {
			0,1,2,
			3,4,5
		};

		// add vertices
		for (uint32_t i=0;i<6;++i) {
			m_vertexData.positions.push_back(glm::vec4(vertices[i].x,vertices[i].y,0.0,1.0));
			m_vertexData.normals.push_back(glm::vec4(0.0f,0.0f,1.0f,1.0f));
			m_vertexData.texUVs.push_back(glm::vec4(vertices[i].u,vertices[i].v,0.0,1.0));
		}
		for (uint32_t i=0;i<2;++i) {
			Face f;
			f.possI = f.texuvI = f.normalI = {indices[i*3],indices[i*3+1],indices[i*3+2],0};
			m_faces.push_back(f);
		}

	} else {
		m_MP |= MP_QUAD;
		static const struct
		{
			float x, y;
			float u, v;
			float n1,n2,n3;
		} vertices[4] =
		{
			{ -1.0f, -1.0f, 0.f, 0.f,	  0.0f,0.0f,1.0f },
			{  1.0f, -1.0f, 1.f, 0.f,	  0.0f,0.0f,1.0f },
			{  1.0f,  1.0f, 1.f, 1.f,	  0.0f,0.0f,1.0f },
			{ -1.0f,  1.0f, 0.f, 1.f,	  0.0f,0.0f,1.0f },
		};
		// add vertices
		for (uint32_t i=0;i<4;++i) {
			m_vertexData.positions.push_back(glm::vec4(vertices[i].x,vertices[i].y,0.0,1.0));
			m_vertexData.normals.push_back(glm::vec4(0.0f,0.0f,1.0f,1.0f));
			m_vertexData.texUVs.push_back(glm::vec4(vertices[i].u,vertices[i].v,0.0,1.0));
		}
		Face f;
		f.possI = f.texuvI = f.normalI = {0,1,2,3};
		m_faces.push_back(f);
	}
}

}//ehj
