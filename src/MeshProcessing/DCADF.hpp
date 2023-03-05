#include "../Structures/ADF.hpp"
#include "../Mesh.hpp"

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

	Mesh getMesh() { return m_mesh; };

private:
	ADF<Data> m_adf;
	Mesh m_mesh;
};

}//ehj
