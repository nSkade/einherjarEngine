#pragma once
#include <vector>
#include "../lib/glm/glm.hpp"
#include <string>

namespace ehj {

class Mesh {
public:
	Mesh();
	Mesh(Mesh& mesh);
	Mesh(std::string path);
	void clear();

	void storeOBJ(std::string path);
	void loadOBJ(std::string path);

	std::vector<float> getVertexBuffer();
	std::vector<int> getIndexBuffer();
	
	struct Face {
		glm::ivec4 vertsI = {-1,-1,-1,-1};
		glm::ivec4 normalI = {-1,-1,-1,-1};
		glm::ivec4 colorI = {-1,-1,-1,-1};
		glm::ivec4 texUVI = {-1,-1,-1,-1};
	};
	
	// Buffer Flags
	enum MeshProperty {
		MP_QUAD     = 1,
		MP_NORMAL   = 2,
		MP_VRTNRM   = 4,
		MP_COLOR    = 8,
	};
	
	uint32_t getMP();
	uint32_t getDim();

	void toTriangles();
	
protected:
	std::vector<glm::vec4> m_vertices;
	std::vector<glm::vec4> m_normals;
	std::vector<glm::vec4> m_colors;
	std::vector<glm::vec4> m_texUVs;
	std::vector<Face> m_faces;
	
	// Buffer Properties
	uint32_t m_MP = 0;
	uint32_t m_Dim = 3; // dimension of object
};


class SSMesh : public Mesh {
public:
	SSMesh(bool triangles = false);
};

}//ehj

