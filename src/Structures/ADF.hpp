#include "Octree.hpp"

class ADF {
public:
	struct Data {
		float cornerSDV[8];
	};
private:
	Octree<Data> m_octree;
};
