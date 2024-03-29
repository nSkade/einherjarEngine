#include <cmath>
#include <iostream>

#include "Octree.hpp"

namespace ehj {

//TODO discard cornerSDV of split Cells? Split Cells children always contain 8 sdf corner values
template <typename T>
class ADF {
public:
	ADF() {
		m_root = m_octree.getRoot();
	}

	//               front back
	//cell order is: 2 3   6 7
	//               0 1   4 5
	//               x-> y^ z
	struct Data {
		bool set = false;
		float cornerSDV[8]; //TODO change to * for less memory consumption
		T data;
	};

	void sampleCell(typename Octree<Data>::Cell* c) {
		if (!m_sdfFunc) {
			std::cerr << "ADF::sampleCell : m_sdfFunc not set." << std::endl;
			return;
		}
		uint32_t ca = sizeof(c->child)/sizeof(c);
		for (uint32_t i=0;i<ca;++i) {
			glm::vec3 p;
			glm::vec3 d = c->aabb.max - c->aabb.min;
			p.x = c->aabb.min.x + d.x*float(bool(i&1));
			p.y = c->aabb.min.y + d.y*float(bool(i&2));
			p.z = c->aabb.min.z + d.z*float(bool(i&4));
			float sdf = m_sdfFunc(p);
			c->data.cornerSDV[i] = sdf;
		}
		c->data.set = true;
	}

	float evaluateCell(typename Octree<Data>::Cell* c, glm::vec3 p) {
		// compute position of p relative to c
		glm::vec3 rp = p - c->aabb.min;
		glm::vec3 diag = c->aabb.max-c->aabb.min;
		rp = rp / diag;
		
		//TODO determine in which child p is
		if (c->child[0]) {
			uint32_t idx = 0;
			idx |= (int(std::roundf(rp.x)) << 1);
			idx |= (int(std::roundf(rp.y)) << 2);
			idx |= (int(std::roundf(rp.z)) << 4);
			return evaluateCell(c->child[idx],p);
		}

		// trilinear interpolate sdf
		float* cv = c->data.cornerSDV;

		float sx01 = cv[0]+rp.x*(cv[1]-cv[0]);
		float sx12 = cv[1]+rp.x*(cv[2]-cv[1]);
		float sx34 = cv[3]+rp.x*(cv[4]-cv[3]);
		float sx56 = cv[5]+rp.x*(cv[6]-cv[5]);

		float sy0 = sx01+rp.y*(sx12-sx01);
		float sy1 = sx34+rp.y*(sx56-sx34);

		float sdf = sy0+rp.z*(sy1-sy0);

		return sdf;
	}

	void split(Octree<Data>& octree, typename Octree<Data>::Cell* c) {
		octree.splitCell(c);
		for (uint32_t i=0;i<8;++i) {
			sampleCell(c->child[i]);
		}
	}

	virtual bool determineSplit(typename Octree<Data>::Cell* c) {
		if(!c->data.set) {
			std::cout << "determineSplit: cell is not sampled.\n";
			return false;
		}
		bool sc = false;

		glm::vec3 d = c->aabb.max-c->aabb.min;
		// check sdf on all midpoints (mp edges, mp faces and center)
		for (uint32_t i=0;i<3*3*3;++i) {
			glm::vec3 p = c->aabb.min;

			// values are 0, 1 or 2 for min, mid, max
			uint32_t x = (i%3);
			uint32_t y = ((i/3)%3);
			uint32_t z = (i/9);

			//TODO skip corner values
			if (!(x==1||y==1||z==1))
				continue;
			
			float mx = x*0.5f;
			float my = y*0.5f;
			float mz = z*0.5f;
			
			p.x += d.x*mx;
			p.y += d.y*my;
			p.z += d.z*mz;
			
			if (std::abs(evaluateCell(c,p)-m_sdfFunc(p)) > m_tolerance)
				sc = true;
		}
		return sc;
	}

	virtual void constructADF(typename Octree<Data>::Cell* c) {
		if (!c) {
			std::cout << "constructADF error: parameter Cell (c) is nullptr.\n";__debugbreak();exit(1);
		}
		sampleCell(c);
		//TODO determine if cell must be split
		bool sc = determineSplit(c);

		if (c->depth < m_maxDepth && sc) {
			m_octree.splitCell(c);
			for (uint32_t i=0;i<8;++i) {
				if (!c->child[i]) {
					std::cout << "constructADF error: child is nullptr.\n";__debugbreak();exit(1);
				}
				constructADF(c->child[i]);
			}
		}
	}

//	float sdfCircle(glm::vec3 pos, glm::vec3 c, float r) {
//		return glm::length(pos-c)-r;
//	}
//
//	float sdfFuncTest(glm::vec3 p) {
//		return sdfCircle(p, glm::vec3(0.0f,0.0f,0.0f),1.0f);
//	}
//
//	void test() {
//		m_root->aabb = {glm::vec4(-1.5f,-1.5f,-1.5f,0.0f),glm::vec4(1.5f,1.5f,1.5f,0.0f)};
//		//m_octree.splitCell(m_root);
//
//		constructADF(m_root);
//		
//		std::cout << "finished\n";
//	}

	void setSDFFunc(float (*sdfFunc)(glm::vec3 p)) {
		m_sdfFunc = sdfFunc;
	}
	
	void setAABB(AABB aabb) {
		m_root->aabb = aabb;
	}

	void setTolerance(float t) {
		m_tolerance = t;
	}
	
	void setMaxDepth(uint32_t d) {
		m_maxDepth = d;
	}

	float (*getSDFFunc())(glm::vec3) { return m_sdfFunc; };

	typename Octree<Data>::Cell* getRoot() { return m_root; };

private:
	uint32_t m_maxDepth = 8;
	float m_tolerance = 0.03;

	float (*m_sdfFunc)(glm::vec3 p) = nullptr;

	Octree<Data> m_octree;
	typename Octree<Data>::Cell* m_root;
};

}//ehj
