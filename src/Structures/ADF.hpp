#include "Octree.hpp"

class ADF {
public:
	//               front back
	//cell order is: 2 3   6 7
	//               0 1   4 5
	//               x-> y^ z
	struct Data {
		float cornerSDV[8];
	};

	float sdfFunc(glm::vec3 pos, glm::vec3 c, float r) {
		return glm::length(pos-c)-r;
	}

	void sampleCell(Octree<Data>::Cell* c) {
		uint32_t ca = sizeof(c->child)/sizeof(c);
		for (uint32_t i=0;i<ca;++i) {
			glm::vec3 p;
			glm::vec3 d = c->aabb.max - c->aabb.min;
			p.x = c->aabb.min.x + d.x*float(bool(i&1));
			p.y = c->aabb.min.y + d.y*float(bool(i&2));
			p.z = c->aabb.min.z + d.z*float(bool(i&4));
			float sdf = sdfFunc(p,glm::vec3(0.0f,0.0f,0.0f),1.0f);
			c->data.cornerSDV[i] = sdf;
		}
	}

	void split(Octree<Data>::Cell* c) {
		sampleCell(c);
		//if ()
	}

	void test() {
		Octree<Data> m_octree;
		m_octree.getRoot()->aabb = {glm::vec4(-1.5f,-1.5f,-1.5f,0.0f),glm::vec4(1.5f,1.5f,1.5f,0.0f)};
		sampleCell(m_octree.getRoot());
		m_octree.splitCell(m_octree.getRoot());
		for (uint32_t i=0;i<8;++i) {
			sampleCell(m_octree.getRoot()->child[i]);
		}
		std::cout << "finished\n";
	}

private:
	uint32_t m_maxDepth = 8;
};

