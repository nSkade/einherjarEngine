#include <vector>
#include "../lib/glm/glm.hpp"
#include <string>

namespace ehj {
	class Mesh {
	public:
		struct Face {
			uint32_t vertsI[4];
			uint32_t colorI;
			uint32_t normalI;
		};

		std::string convertOBJ();
	private:
		std::vector<glm::vec4> vertices;
		std::vector<glm::vec4> normals;
		std::vector<glm::vec4> colors;
		std::vector<Face> faces;
	};
}