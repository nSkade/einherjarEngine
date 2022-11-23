#include "Mesh.hpp"

namespace ehj {
	std::string Mesh::convertOBJ() {
		std::string ret;
		
		for (uint32_t i=0;i<vertices.size();i++) {
			
			ret.append("\nv ");
			ret.append(std::to_string(vertices[i][0]).append(" "));
			ret.append(std::to_string(vertices[i][1]).append(" "));
			ret.append(std::to_string(vertices[i][2]));
		}

		for (uint32_t i=0;i<faces.size();i++) {
			ret.append("\nf");
			for (uint32_t j=0;j<3;j++) {
				ret.append(std::to_string(faces[i].vertsI[j]));
			}
		}
	}

	//void Mesh::loadOBJ() {
	//	
	//}
}