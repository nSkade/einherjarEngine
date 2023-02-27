#include "../Structures/ADF.hpp"
#include "../Mesh.hpp"

namespace ehj {

/**
 * @brief Dual Contouring of ADFs
*/
class DCADF {
public:
	DCADF(float (*sdfFunc)(glm::vec3 p)) {
		
		// createADF
		m_adf.setSDFFunc(sdfFunc);
		AABB aabb = {glm::vec4(-1.5f,-1.5f,-1.5f,0.0f),glm::vec4(1.5f,1.5f,1.5f,0.0f)};
		m_adf.setAABB(aabb);
		m_adf.constructADF(m_adf.getRoot());
	
	}

	//TODO doual conturing options
	void createMesh() {
		
	};

	Mesh getMesh() { return m_mesh; };

private:
	ADF m_adf;
	Mesh m_mesh;
};

}//ehj
