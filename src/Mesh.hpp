#pragma once

namespace ehj {

struct Face {
	glm::ivec4 mergedI  = {-1,-1,-1,-1};
	glm::ivec4 possI = {-1,-1,-1,-1};
	glm::ivec4 normalI = {-1,-1,-1,-1};
	glm::ivec4 colorI = {-1,-1,-1,-1};
	glm::ivec4 texUVI = {-1,-1,-1,-1};
};

struct Mesh;

struct VertexData {
	std::vector<glm::vec4> positions;
	std::vector<glm::vec4> normals;
	std::vector<glm::vec4> colors;
	std::vector<glm::vec4> texUVs;

	void clear();

	/**
	 * @brief assembles merged vertex buffer from all attributes: [P0, N0, U0, P1...
	 */
	void assembleVertexBuffer(std::vector<Mesh*> ms);
	std::vector<float> merged;

	void computeNormals(std::vector<Face>& faces);

	enum VertexProperty {
		VP_NORMAL   = 1 << 0,
		VP_COLOR    = 1 << 1,
		VP_UV       = 1 << 2,
	};
	uint32_t m_VP = 0;
	uint32_t m_Dim = 3; // dimension of object, assume 3 for now
};

struct Mesh {
	Mesh();
	//Mesh(Mesh& mesh);
	Mesh(std::string path);
	void clear();

	std::vector<int> getIndexBuffer();

	void storeOBJ(std::string path);
	void loadOBJ(std::string path);
	void loadOBJcust(std::string path);

	void toTriangles();
	
	// Buffer Flags
	enum MeshProperty {
		MP_QUAD     = 1,		//TODO index property
	};
	uint32_t m_MP = 0;

	std::vector<Face> m_faces;
	
	bool m_hasVertexData = false;
	VertexData m_vertexData;
};

/**
 * @brief SceenSpace Mesh
*/
struct SSMesh : public Mesh {
	SSMesh(bool triangles = false);
};

}//ehj

