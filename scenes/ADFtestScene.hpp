#include "../src/suOGL.hpp"
#include "../src/Structures/ADF.hpp"

class ADFtestScene : IScene {
public:
	void setup() {

	}
	int run() {
		
		//ehj::Mesh mesh("models/ssn4.obj");
		//mesh.toTriangles();
		ehj::Mesh mesh;
		
		
		ADF adf;
		adf.test();
		
		
		
		//mesh.storeOBJ("modelsOut/sdfc.obj");
		return 0;
	}
	void cleanup() {
	
	}
};
