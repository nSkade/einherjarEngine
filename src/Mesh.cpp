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

		if (file.is_open()) {
			while (getline(file,line)) {
				if (line[0]=='#') continue; // comment
				else if (line[0]=='v') { //vertex
					std::istringstream iss(line);
					char n;
					float v1,v2,v3,v4 = 0.0f;
					iss >> n >> v1 >> v2 >> v3;
					m_vertices.emplace_back(v1,v2,v3,v4);
				}
				else if (line[0]=='f') { // face
					std::istringstream iss(line);
					char n;
					int32_t v[4];
					v[3] = -1;
					iss >> n >> v[0] >> v[1] >> v[2];
					if (!iss.tellg()==line.length()) {
						m_BP |= BP_QUAD;
						iss >> v[3];
					}
					Face f;
					for (uint32_t i=0;i<4;++i) {
						f.vertsI[i] = v[i];
					}
					m_faces.push_back(f);
				}
			}
			file.close();
		}
	}
}