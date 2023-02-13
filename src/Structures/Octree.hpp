#include "AABB.hpp"

template <class T>
class Octree {
public:

	struct Cell {
		Cell* parent;
		Cell* child[8];
		uint32_t depth;
		AABB aabb;
		T data;

		uint32_t idx;
	};

	Cell* getRoot() {
		return m_root;
	}

	Octree() {
		m_root = new Cell();
		m_root->parent = nullptr;
		uint32_t ca = sizeof(m_root->child)/sizeof(m_root);
		for (uint32_t i=0;i<ca;++i)
			m_root->child[i] = nullptr;
		m_root->depth = 0;
		m_root->idx = 0;
	}

	~Octree() {
		destruct(m_root);
	}

	//               front back
	//cell order is: 2 3   6 7
	//               0 1   4 5
	//               x-> y^ z
	void splitCell(Cell* c) {
		uint32_t ca = sizeof(c->child)/sizeof(c);
		for(uint32_t i=0;i<ca;++i) {
			Cell* nc = new Cell();
			nc->parent = c;
			nc->depth = c->depth+1;
			nc->idx = i;

			AABB bb;
			glm::vec3 diag = c->aabb.max-c->aabb.min;
			glm::vec3 off = glm::vec3(bool(i&1),bool(i&2),bool(i&4));
			bb.min = c->aabb.min + diag*0.5f*off;
			bb.max = bb.min + diag*0.5f;
			
			nc->aabb = bb;

			c->child[i] = nc;
		}
	}

	void test() {
		std::cout << "sizeof(m_root): " << sizeof(m_root->child)/sizeof(m_root) << "\n";
	}

private:
	void destruct(Cell* c) {
		if (!c)
			return;
		uint32_t ca = sizeof(c->child)/sizeof(c);
		for (uint32_t i=0;i<ca;++i) {
			if (c->child[i])
				destruct(c->child[i]);
		}
		delete c;
	}

	Cell* m_root;
};
