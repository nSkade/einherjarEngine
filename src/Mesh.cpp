#include "Mesh.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

namespace ehj {
	Mesh::Mesh() {
		
	}

	Mesh::Mesh(Mesh& mesh) {
		this->m_BP = mesh.m_BP;
		this->m_colors = mesh.m_colors;
		this->m_Dim = mesh.m_Dim;
		this->m_faces = mesh.m_faces;
		this->m_normals = mesh.m_normals;
		this->m_texUVs = mesh.m_texUVs;
		this->m_vertices = mesh.m_vertices;
	}

	void Mesh::clear() {
		m_vertices.clear();
		m_normals.clear();
		m_colors.clear();
		
		m_faces.clear();
	}
	
	uint32_t Mesh::getBP() {
		return m_BP;
	}
	uint32_t Mesh::getDim() {
		return m_Dim;
	}

	std::string Mesh::convertOBJ() {
		std::string ret;
		
		for (uint32_t i=0;i<m_vertices.size();i++) {
			
			ret.append("\nv ");
			ret.append(std::to_string(m_vertices[i][0]).append(" "));
			ret.append(std::to_string(m_vertices[i][1]).append(" "));
			ret.append(std::to_string(m_vertices[i][2]));
		}

		for (uint32_t i=0;i<m_faces.size();i++) {
			ret.append("\nf");
			for (uint32_t j=0;j<3;j++) {
				ret.append(std::to_string(m_faces[i].vertsI[j]));
			}
		}
		return "";
	}

	void Mesh::loadOBJ(std::string path) {
		std::ifstream file(path, std::ifstream::in);
		std::string line;
		m_BP -= BP_QUAD;
		bool normals = false;

		if (file.good()) {
			while (getline(file,line)) {
				if (line[0]=='#') continue; // comment
				else if (line.substr(0,2).compare("vn")==0) { // face normal?
					normals = true;
					std::istringstream iss(line);
					std::string n;
					float v1,v2,v3,v4 = 0.0f;
					iss >> n >> v1 >> v2 >> v3;
					m_normals.emplace_back(v1,v2,v3,v4);
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
					if (normals)
						iss >> x >> v[0] >> x >> x >> n[0] >> v[1] >> x >> x >> n[1] >> v[2] >> x >> x >> n[2];
					else
						iss >> x >> v[0] >> v[1] >> v[2];
					if (iss.tellg()!=line.length()) { // guys we have a quad!
						m_BP |= BP_QUAD;
						iss >> v[3];
						if (normals)
							iss >> x >> x >> n[3];
					}
					Face f;
					for (uint32_t i=0;i<4;++i) {
						f.vertsI[i] = v[i]-1;
						if (normals)
							f.normalI[i] = n[i]-1;
					}
					m_faces.push_back(f);
				}
			}
			file.close();
		}
	}

	std::vector<float> Mesh::getVAOBuffer() {
		std::vector<float> res;
		for (uint32_t i=0;i<m_vertices.size();++i) {
			for (uint32_t j=0;j<m_Dim;++j) {
				res.emplace_back(m_vertices[i][j]);
			}
			if (m_BP & BP_NORMAL) {
				for (uint32_t j=0;j<m_Dim;++j) {
					res.emplace_back(m_normals[0][j]); //TODO fix
				}
			}
		}
		return res;
	}

	std::vector<int> Mesh::getIndexBuffer() {
		std::vector<int> res;
		for (uint32_t i=0;i<m_faces.size();++i) {
			uint32_t s = 3; // triangle std
			if (m_BP & BP_QUAD)
				s = 4;
			for (uint32_t j=0;j<s;++j) {
				res.emplace_back(m_faces[i].vertsI[j]);
			}
		}
		return res;
	}

	void Mesh::toTriangles() {
		Mesh mesh = Mesh(*this);
		m_faces.clear();
		for (uint32_t i=0;i<mesh.m_faces.size();++i) {
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
		m_BP -= BP_QUAD;
	}

	SSMesh::SSMesh(bool triangles) {
		m_BP |= BP_VRTNRM;
		m_BP |= BP_NORMAL;
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
			m_BP |= BP_QUAD;
			static const struct
			{
				float x, y;
				float u, v;
				float n1,n2,n3;
			} vertices[4] =
			{
				{ -1.0f, -1.0f, 0.f, 0.f,     0.0f,0.0f,1.0f },
				{  1.0f, -1.0f, 1.f, 0.f,     0.0f,0.0f,1.0f },
				{  1.0f,  1.0f, 1.f, 1.f,     0.0f,0.0f,1.0f },
				{ -1.0f,  1.0f, 0.f, 1.f,     0.0f,0.0f,1.0f },
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
}