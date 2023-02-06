#include "AABB.hpp"

template <class T>
class Octree {
public:
	struct Cell {
		Cell* parent;
		Cell* children[8];
		AABB aabb;
		T* data;
	};
private:
	Cell* m_root;
};
