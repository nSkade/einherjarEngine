#include <vector>
#include "../lib/glm/glm.hpp"
#include <string>

namespace ehj {
	class Mesh {
	public:
		struct Face {
			uint32_t verts[4];
		};

		std::string convertOBJ();
	private:
		std::vector<glm::vec4> vertices;
		std::vector<Face> faces;
	};
}