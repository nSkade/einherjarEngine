#include "Input/Input.hpp"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <memory>

//TODO abstract
#include "Input/GLFW/GLFWKeyboard.hpp"
#include "Input/GLFW/GLFWMouse.hpp"

namespace ehj {

class ICameraController;

class Camera {
public:
	void addCameraController(ICameraController* cc);
	void removeCameraController(ICameraController* cc);
	/**
	 * @brief checks states of camera controllers and updates camera on change
	*/
	void update(float deltaTime);
	
	glm::mat4 getView() { return m_view; }; //TODO only update mat when getView

	void setPos(glm::vec3 p) { m_pos = p; m_viewUpdated = false; };
	void setDir(glm::vec3 d) { m_dir = d; m_viewUpdated = false; m_isPerp = false; }; //TODO calculate target
	//TODO not implemented yet
	void setTarget(glm::vec3 t) { m_target = t; m_viewUpdated = false; }; //TODO calculate dir
	void setUp(glm::vec3 u) { m_up = u; m_viewUpdated = false; m_isPerp = false; };
	
	glm::vec3 getPos() { return m_pos; };
	glm::vec3 getDir() { return m_dir; };
	glm::vec3 getTarget() { return m_target; };
	glm::vec3 getUp() { return m_up; };
	float getFocus() { return m_proj[1][1]; };

	glm::vec3 getRight() { return m_right; };

	void makePerpendicular();

	glm::mat4 getPV() {
		if (!m_projUpdated || !m_viewUpdated) {
			m_pv = m_proj*m_view;
			m_projUpdated = true;
		}
		return m_pv;
	};
	void setProj(glm::mat4 p) { m_proj = p; m_projUpdated = false; };

protected:
	/**
	 * @brief signals Camera that its parameters have changed so the matrix gets updated
	*/
	void signalChange() { m_viewUpdated = false; };

	bool m_noTilt = true;
	// variables calculated by class
	bool m_viewUpdated = false;
	glm::mat4 m_view = glm::mat4(1.0f);
	bool m_isPerp = false; // true if dir, up and right are perpendicular
	glm::vec3 m_right = glm::vec3(1.0f,0.0f,0.0f);

	// variables to set by controllers or manualy
	glm::vec3 m_pos = glm::vec3(0.0f);
	bool m_hasTarget = false;
	glm::vec3 m_dir = glm::vec3(0.0f,0.0f,-1.0f);
	glm::vec3 m_target = glm::vec3(0);
	glm::vec3 m_up = glm::vec3(0.0f,1.0f,0.0f);

	bool m_projUpdated = false;
	glm::mat4 m_pv = glm::mat4(1.0f);
	glm::mat4 m_proj = glm::mat4(1.0f);
	
	std::vector<ICameraController*> m_controllers;
};

class ICameraController {
public:
	virtual void update(float deltaTime) = 0;
protected:
	ICameraController(Camera* cam) : m_pCam(cam) {};
	Camera* m_pCam;
};

/**
 * @brief Camera Controller Mouse - rotation of camera
*/
class CCmouse : public ICameraController, public ICBlistener<MouseData> {
public:
	CCmouse(Camera* cam) : ICameraController(cam) {
		m_pMouse = GLFWMouse::instance();
		m_pMouse->addListener(this);
	}
	~CCmouse() { GLFWMouse::instance()->removeListener(this); }
	void update(float deltaTime) {
		if (!oldPosSet) {
			oldPos = pos;
			oldPosSet = true;
		}
		if (m_captured || m_nonCapDrag) {
			glm::ivec2 m = pos - oldPos;
			if (m.x == 0 && m.y == 0) // no change no update, might happen
				return;
			yaw += float(m.x)*m_sens.x;
			pitch -= float(m.y)*m_sens.y;
			if(pitch > 89.0f)
				pitch = 89.0f;
			if(pitch < -89.0f)
				pitch = -89.0f;
			glm::vec3 dir;
			dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			dir.y = sin(glm::radians(pitch));
			dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
			m_pCam->setDir(glm::normalize(dir));

			oldPos = pos;
		} else {
			oldPos = pos;
		}
	}
	void callback(MouseData md) {
		if (md.mb==IBCodes::MB_BUTTON_NONE) {
			pos = glm::ivec2(md.xpos, md.ypos);
		} else {
			if (md.mb==IBCodes::MB_BUTTON_MIDDLE) {
				if (md.ia==IBCodes::IA_PRESS) {
					m_nonCapDrag = true;
				}
				else if (md.ia==IBCodes::IA_RELEASE)
					m_nonCapDrag = false;
			}
		}
	}
	void captureMouse(bool b) {
		m_captured = b;
	}
	void setSensitivity(glm::vec2 sens) { m_sens = sens; };
private:
	bool m_captured = false;
	bool m_nonCapDrag = false;

	float yaw = -90.0f;
	float pitch = 0.0f;

	glm::vec2 m_sens = glm::vec2(0.1f);
	glm::ivec2 pos = glm::ivec2(0,0); //TODO initialize to screen center?
	bool oldPosSet = false;
	glm::ivec2 oldPos = glm::ivec2(0,0);

	std::shared_ptr<GLFWMouse> m_pMouse;
};

/**
 * @brief Camera Controller Keyboard - position of camera
*/
class CCkb : public ICameraController, public ICBlistener<KeyboardData> {
public:
	enum state : uint32_t {
		FWD = 1,
		BWD = 2,
		LFT = 4,
		RGT = 8,
		UP = 16,
		DWN = 32,
		SPT = 64,  // sprint
		WLK = 128, // walk
		VLFT = 256,
		VRGT = 512,
		VUP = 1024,
		VDWN = 2048,
	};
	CCkb(Camera* cam) : ICameraController(cam) {
		m_pKB = GLFWKeyboard::instance();
		m_pKB->addListener(this);
	}
	~CCkb() { GLFWKeyboard::instance()->removeListener(this); }

	void update(float deltaTime);
	void callback(KeyboardData kd);
private:
	uint32_t m_state = 0;

	glm::vec2 m_sens = glm::vec2(20.f);
	//TODOf uniform place with mouse cam
	float yaw = -90.0f;
	float pitch = 0.0f;
	
	std::shared_ptr<GLFWKeyboard> m_pKB;
};

class FreeFlyCamera : public Camera {
public:
	FreeFlyCamera() : m_cckb(this), m_ccm(this) {
		addCameraController(&m_cckb);
		addCameraController(&m_ccm);
	}
	~FreeFlyCamera() {
		removeCameraController(&m_cckb);
		removeCameraController(&m_ccm);
	}

	//TODO better way to forward info?
	void captureMouse(bool b) {
		m_ccm.captureMouse(b);
	}
	void setSensitivity(glm::vec2 sens) {
		m_ccm.setSensitivity(sens);
	}

	void makePerpendicular();
private:
	CCkb m_cckb;
	CCmouse m_ccm;
};

}//ehj
