#pragma once

#include "Mesh.hpp"

#include <string>

namespace ehj {

struct Model {
	Model();
	Model(std::string path);

	void loadOBJ(std::string path);
	void loadGltf(const std::string& path);

	std::vector<ehj::Mesh> m_meshes;
private:
	uint32_t m_MP = 0; // mesh properties
};

}//ehj
