#include "Mesh.hpp"

#include <fstream>
#include <sstream>

#include <iostream>

//#include <tiny_obj_loader.h>
#include <rapidobj/rapidobj.hpp>

namespace ehj {

Mesh::Mesh() {
	
}

Mesh::Mesh(Mesh& mesh) {
	this->m_MP = mesh.m_MP;
	this->m_colors = mesh.m_colors;
	this->m_Dim = mesh.m_Dim;
	this->m_faces = mesh.m_faces;
	this->m_normals = mesh.m_normals;
	this->m_texUVs = mesh.m_texUVs;
	this->m_vertices = mesh.m_vertices;
}

Mesh::Mesh(std::string path) {
	loadOBJ(path);
}

void Mesh::clear() {
	m_vertices.clear();
	m_normals.clear();
	m_colors.clear();
	
	m_faces.clear();
}

uint32_t Mesh::getMP() {
	return m_MP;
}
uint32_t Mesh::getDim() {
	return m_Dim;
}

void Mesh::storeOBJ(std::string path) {
	//TODO rewrite with rapidobj
	std::ofstream file(path, std::ostream::out | std::ostream::trunc);
	std::string ret;
	
	for (uint32_t i=0;i<m_vertices.size();i++) {
		if (i==0)
			ret.append("v ");
		else
			ret.append("\nv ");
		ret.append(std::to_string(m_vertices[i][0]).append(" "));
		ret.append(std::to_string(m_vertices[i][1]).append(" "));
		ret.append(std::to_string(m_vertices[i][2]));
	}

	for (uint32_t i=0;i<m_faces.size();i++) {
		ret.append("\nf ");
		for (uint32_t j=0;j<3;j++) {
			ret.append(std::to_string(m_faces[i].vertsI[j]+1)).append(" ");
		}
		if (m_MP & MP_QUAD)
			ret.append(std::to_string(m_faces[i].vertsI[3]+1)).append(" ");
	}
	ret.append("\n");
	file << ret;
}

#if 1
void Mesh::loadOBJ(std::string path) {
	rapidobj::Result result = rapidobj::ParseFile(path);
	if (result.error) {
		return;
	}

	//TODO result.materials[0].diffuse_texname
	
	const rapidobj::Attributes& attrib = result.attributes;

	if (result.shapes[0].mesh.num_face_vertices[0]==4) //TODO
		m_MP |= MP_QUAD;
	
	m_vertices.reserve(attrib.positions.size() / 3);
	for (size_t i = 0; i < attrib.positions.size(); i += 3) {
		const auto& v = attrib.positions;
		m_vertices.emplace_back(v[i + 0], v[i + 1], v[i + 2], 1.);
	}

	m_normals.reserve(attrib.normals.size() / 3);
	for (size_t i = 0; i < attrib.normals.size(); i += 3) {
		const auto& n = attrib.normals;
		m_normals.emplace_back(n[i + 0], n[i + 1], n[i + 2], 1.);
	}
	if (!attrib.normals.empty()) {
		m_MP |= MP_NORMAL;
	}

	m_texUVs.reserve(attrib.texcoords.size() / 2);
	for (size_t i = 0; i < attrib.texcoords.size(); i += 2) {
		const auto& t = attrib.texcoords;
		m_texUVs.emplace_back(t[i + 0], t[i + 1], 0., 0.);
	}
	if (!attrib.texcoords.empty()) {
		m_MP |= MP_UV;
	}

	for (const auto& shape : result.shapes) {
		size_t index_offset = 0;
		for (size_t fv_count : shape.mesh.num_face_vertices) {
			Face newFace;
			for (size_t v = 0; v < fv_count; v++) {
				const rapidobj::Index idx = shape.mesh.indices[index_offset + v];
				if (v < 4) {
					newFace.vertsI[v] = idx.position_index;
					newFace.normalI[v] = idx.normal_index;
					newFace.texUVI[v] = idx.texcoord_index;
				}
			}
			m_faces.push_back(newFace);
			index_offset += fv_count;
		}
	}
}
#endif
#if 0
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
					newFace.vertsI[v] = idx.vertex_index;
					newFace.normalI[v] = idx.normal_index;
					newFace.texUVI[v] = idx.texcoord_index;
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

	if (file.good()) {
		while (getline(file,line)) {
			if (line[0]=='#') continue; // comment
			else if (line.substr(0,2).compare("vn")==0) { // face normal?
				m_MP |= MP_NORMAL;
				std::istringstream iss(line);
				std::string n;
				float v1,v2,v3,v4 = 0.0f;
				iss >> n >> v1 >> v2 >> v3;
				m_normals.emplace_back(v1,v2,v3,v4);
			}
			else if (line.substr(0,2).compare("vt")==0) { // face normal?
				m_MP |= MP_UV;
				std::istringstream iss(line);
				std::string n;
				float v1,v2,v3,v4 = 0.0f;
				iss >> n >> v1 >> v2;
				m_texUVs.emplace_back(v1,v2,v3,v4);
			}
			else if (line[0]=='v') { //vertex
				std::istringstream iss(line);
				char n;
				float v1,v2,v3,v4 = 0.0f;
				iss >> n >> v1 >> v2 >> v3;
				m_vertices.emplace_back(v1,v2,v3,v4);
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

				if (m_MP & MP_NORMAL)
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
					f.vertsI[i] = v[i]-1;
					if (m_MP & MP_NORMAL)
						f.normalI[i] = n[i]-1;
				}
				m_faces.push_back(f);
			}
		}
		file.close();
	}
}

#if 1
std::vector<float> Mesh::getVertexBuffer() {
	std::vector<float> res;
	std::vector<int> nIdxLuNRM(m_vertices.size(), -1);
	std::vector<int> nIdxLuUV(m_vertices.size(), -1);

	// reserve size
	int resSize = m_Dim;
	resSize += m_MP & MP_NORMAL ? m_Dim : 0;
	resSize += m_MP & MP_UV ? 2 : 0;
	res.reserve(m_vertices.size() * resSize);

	if (m_MP & MP_NORMAL) {
		for (const auto& f : m_faces)
			for (uint32_t l = 0; l < 3; ++l) {
				int vIdx = f.vertsI[l];
				if (vIdx >= 0 && vIdx < m_vertices.size() && nIdxLuNRM[vIdx] == -1)
					nIdxLuNRM[vIdx] = f.normalI[l];
			}
	}
	if (m_MP & MP_UV) {
		for (const auto& f : m_faces)
			for (uint32_t l = 0; l < 2; ++l) { //TODO this could be 3
				int vIdx = f.vertsI[l];
				if (vIdx >= 0 && vIdx < m_vertices.size() && nIdxLuUV[vIdx] == -1)
					nIdxLuUV[vIdx] = f.texUVI[l];
			}
	}

	for (uint32_t i = 0; i < m_vertices.size(); ++i) {
		for (uint32_t j = 0; j < m_Dim; ++j) {
			res.emplace_back(m_vertices[i][j]);
		}
		if (m_MP & MP_NORMAL) {
			int nIdx = nIdxLuNRM[i];
			if (nIdx >= 0 && nIdx < m_normals.size()) {
				for (uint32_t j = 0; j < m_Dim; ++j) {
					res.emplace_back(m_normals[nIdx][j]);
				}
			} else {
				for (uint32_t j = 0; j < m_Dim; ++j) {
					res.emplace_back(m_normals[0][j]);
				}
			}
		}
		if (m_MP & MP_UV) {
			int nIdx = nIdxLuUV[i];
			if (nIdx >= 0 && nIdx < m_texUVs.size()) {
				for (uint32_t j = 0; j < 2; ++j) { //TODO could be 3
					res.emplace_back(m_texUVs[nIdx][j]);
				}
			} else {
				for (uint32_t j = 0; j < 2; ++j) { //TODO could be 3
					res.emplace_back(m_texUVs[0][j]);
				}
			}
		}
	}
	
	return res;
}
#else
std::vector<float> Mesh::getVertexBuffer() {
	std::vector<float> res;
	for (uint32_t i=0;i<m_vertices.size();++i) {
		for (uint32_t j=0;j<m_Dim;++j) {
			res.emplace_back(m_vertices[i][j]);
		}
		bool nrmFound = false;
		if (m_MP & MP_NORMAL) {
			for (uint32_t k=0;k<m_faces.size();++k) {
				for (uint32_t l=0;l<3;++l) {
					if (m_faces[k].normalI[l]==i) {
						for (uint32_t j=0;j<m_Dim;++j)
							res.emplace_back(m_normals[m_faces[k].normalI[l]][j]);
						nrmFound = true;
						break;
					}
				}
				if (nrmFound) break;
			}
			if (!nrmFound) {
				for (uint32_t j=0;j<m_Dim;++j)
					res.emplace_back(m_normals[0][j]); //TODO fix
			}
		}
	}

	//TODO
	//for (uint32_t i=0;i<m_faces.size();++i) {
	//	for (uint32_t j=0;j<4;++j) {
	//		for (uint32_t k=0;k<m_Dim;++k)
	//			res.push_back(m_vertices[m_faces[i].vertsI[j]][k]);
	//		if (m_MP & MP_NORMAL)
	//			for (uint32_t k=0;k<m_Dim;++k)
	//				res.push_back(m_normals[m_faces[i].normalI[j]][k]);
	//	}
	//}

	return res;
}
#endif

std::vector<int> Mesh::getIndexBuffer() {
	std::vector<int> res;
	for (uint32_t i=0;i<m_faces.size();++i) {
		uint32_t s = 3; // triangle std
		if (m_MP & MP_QUAD)
			s = 4;
		for (uint32_t j=0;j<s;++j) {
			res.emplace_back(m_faces[i].vertsI[j]);
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
		std::cout << "toTriangles: " << i << "/" << mesh.m_faces.size() << " " << float(i)/mesh.m_faces.size()*100. << "%" << std::endl;
		Face fq = mesh.m_faces[i];
		Face t1,t2;
		t1.vertsI = {fq.vertsI[0],fq.vertsI[1],fq.vertsI[2],-1};
		t2.vertsI = {fq.vertsI[2],fq.vertsI[3],fq.vertsI[0],-1};

		t1.normalI = {fq.normalI[0],fq.normalI[1],fq.normalI[2],-1};
		t2.normalI = {fq.normalI[2],fq.normalI[3],fq.normalI[0],-1};

		t1.colorI = {fq.colorI[0],fq.colorI[1],fq.colorI[2],-1};
		t2.colorI = {fq.colorI[2],fq.colorI[3],fq.colorI[0],-1};

		t1.texUVI = {fq.texUVI[0],fq.texUVI[1],fq.texUVI[2],-1};
		t2.texUVI = {fq.texUVI[2],fq.texUVI[3],fq.texUVI[0],-1};

		this->m_faces.push_back(t1);
		this->m_faces.push_back(t2);
	}
	m_MP &= ~MP_QUAD;
}

SSMesh::SSMesh(bool triangles) {
	m_MP |= MP_VRTNRM;
	m_MP |= MP_NORMAL;
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
			m_vertices.push_back(glm::vec4(vertices[i].x,vertices[i].y,0.0,1.0));
			m_normals.push_back(glm::vec4(0.0f,0.0f,1.0f,1.0f));
			m_texUVs.push_back(glm::vec4(vertices[i].u,vertices[i].v,0.0,1.0));
		}
		for (uint32_t i=0;i<2;++i) {
			Face f;
			f.vertsI = f.texUVI = f.normalI = {indices[i*3],indices[i*3+1],indices[i*3+2],0};
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
			m_vertices.push_back(glm::vec4(vertices[i].x,vertices[i].y,0.0,1.0));
			m_normals.push_back(glm::vec4(0.0f,0.0f,1.0f,1.0f));
			m_texUVs.push_back(glm::vec4(vertices[i].u,vertices[i].v,0.0,1.0));
		}
		Face f;
		f.vertsI = f.texUVI = f.normalI = {0,1,2,3};
		m_faces.push_back(f);
	}
}

}//ehj
