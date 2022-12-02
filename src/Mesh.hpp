#include <vector>
#include "../lib/glm/glm.hpp"
#include <string>

namespace ehj {
	class Mesh {
	public:
		Mesh();
		void clear();

		std::string convertOBJ();
		void loadOBJ(std::string path);

		void buildBuffer();
		float* getVertBuffer();
		

		struct Face {
			int32_t vertsI[4];
			int32_t normalI;
			int32_t colorI;
			int32_t texUVI;
		};

	private:
		std::vector<glm::vec4> m_vertices;
		std::vector<glm::vec4> m_normals;
		std::vector<glm::vec4> m_colors;
		std::vector<glm::vec4> m_texUVs;
		std::vector<Face> m_faces;

		// Buffer Flags
		enum BufferProperty {
			BP_QUAD     = 1,
			BP_NORMAL   = 2,
			BP_VRTNRM   = 3,
			BP_COLOR    = 4,
		};
		
		// Buffer Properties
		uint32_t m_BP = 0;
		
		float* m_pBuffer;
		uint32_t m_pBufferSize;

		int32_t* m_pIndices;
		uint32_t m_pIndicesSize = 0;
	};
}