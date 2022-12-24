#include "Mesh.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

namespace ehj {
	Mesh::Mesh() {
		
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
		
		std::ifstream file(path);
		std::string line;
		m_BP -= BP_QUAD;
		bool normals = false;

		if (file.is_open()) {
			while (getline(file,line)) {
				if (line[0]=='#') continue; // comment
				else if (line.substr(0,2).compare("vn")==0) { // face normal?
					normals = true;
					std::istringstream iss(line);
					char n;
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
}