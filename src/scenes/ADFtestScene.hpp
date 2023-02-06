#include "sceneUtils.hpp"

class ADFtestScene : IScene {
public:
	void setup() {

	}
	int run() {
		
		ehj::Mesh mesh("models/ssn4.obj");
		mesh.storeOBJ("modelsOut/ssn4.obj");
		return 0;
	}
	void cleanup() {
	
	}
};
