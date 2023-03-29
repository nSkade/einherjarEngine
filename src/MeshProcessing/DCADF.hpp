#include "../Structures/ADF.hpp"
#include "../Mesh.hpp"

#include <glm/gtx/vec_swizzle.hpp>

namespace ehj {

/**
 * @brief Dual Contouring of ADFs
*/
class DCADF {
public:
	struct Data {
		glm::vec3* samples = nullptr; // one sample for every edge
		~Data() {
			if (samples)
				delete samples;
		}
	};

	void setAABB(AABB aabb) {
		m_adf.setAABB(aabb);
	}
	void setSDF(float (*sdfFunc)(glm::vec3 p)) {
		m_adf.setSDFFunc(sdfFunc);
	}
	void process() {
		m_adf.setTolerance(0.1f);
		m_adf.constructADF(m_adf.getRoot());
		//TODO dual contouring
		
	}
	glm::vec3 calcNormal(glm::vec3 p) {
		float (*sdfFunc)(glm::vec3 p) = m_adf.getSDFFunc();
		glm::vec3 small_step = glm::vec3(0.01,0.0,0.0);
		float gradX = sdfFunc(p + glm::xxy(small_step)) - sdfFunc(p - glm::xyy(small_step));
		float gradY = sdfFunc(p + glm::yxy(small_step)) - sdfFunc(p - glm::yxy(small_step));
		float gradZ = sdfFunc(p + glm::yyx(small_step)) - sdfFunc(p - glm::yyx(small_step));
		
		return glm::normalize(glm::vec3(gradX, gradY, gradZ));
	}

	Mesh getMesh() { return m_mesh; };

private:
	ADF<Data> m_adf;
	Mesh m_mesh;
};

}//ehj
