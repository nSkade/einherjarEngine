#include "Input/Input.hpp"

#include <glfw/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>

namespace ehj {

class ICameraController;

class Camera {

	glm::mat4 getView();

private:
	std::vector<ICameraController> m_controller;
	glm::vec3 m_pos;
	glm::vec3 m_dir;
	glm::vec3 m_up;
};

class ICameraController {
public:
protected:
	Camera* m_pCam;
};

class CCmouse : public ICameraController, public ICBlistener<MouseData> {
public:
	CCmouse(Camera* cam) { m_pCam = cam; }
	void callback(MouseData* md) {
		//TODO implement cam movement;
	}
};

class CCkb : public ICameraController, public ICBlistener<KeyboardData> {
public:
	CCkb(Camera* cam) { m_pCam = cam; }
	void callback(KeyboardData* md) {
		//TODO implement cam movement;
	}
};

}//ehj
