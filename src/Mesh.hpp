#pragma once

using namespace glm;

namespace ehj {

// vertex buffer indices
struct Face {
	ivec4 mergedI  = {-1,-1,-1,-1};
	ivec4 possI    = {-1,-1,-1,-1};
	ivec4 normalI  = {-1,-1,-1,-1};
	ivec4 colorI   = {-1,-1,-1,-1};
	ivec4 texuvI   = {-1,-1,-1,-1};
	ivec4 tangI    = {-1,-1,-1,-1};
};

struct Mesh;

struct VertexData {
	std::vector<vec4> positions;
	std::vector<vec4> normals;
	std::vector<vec4> colors;
	std::vector<vec4> texUVs;
	std::vector<vec4> tangents;

	void clear();

	/**
	 * @brief assembles merged vertex buffer from all attributes: [P0, N0, U0, P1...
	 */
	void assembleVertexBuffer(std::vector<Mesh*> ms);
	std::vector<float> merged;

	void computeNormals(std::vector<Face>& faces);

	enum VertexProperty {
		VP_NRM = 1 << 0, // normal
		VP_COL = 1 << 1, // color
		VP_UV  = 1 << 2, // uv
		VP_TAN = 1 << 3, // tangent
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
	void assembleVertexBuffer() {
			if (m_hasVertexData)
				m_vertexData.assembleVertexBuffer({this});
			else
				std::cerr << "Mesh: Warning: tried to assemble Vertex Buffer on Mesh without Vertex Buffer\n";
			};
	
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

