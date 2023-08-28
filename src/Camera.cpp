#include "Camera.hpp"

#include <glm/ext/matrix_transform.hpp>

namespace ehj {

void Camera::addCameraController(ICameraController* cc) {
	for (uint32_t i=0;i<m_controllers.size();++i) { //TODO check faster with map?
		if (m_controllers[i]==cc)
			return;
	}
	m_controllers.emplace_back(cc);
}

void Camera::removeCameraController(ICameraController* cc) {
	for (uint32_t i=0;i<m_controllers.size();++i) {
		if (m_controllers[i]==cc) {
			m_controllers.erase(m_controllers.begin()+i);
			return;
		}
	}
}

void Camera::update(float deltaTime) {
	makePerpendicular();
	for (uint32_t i=0;i<m_controllers.size();++i)
		m_controllers[i]->update(deltaTime);
	if (!m_viewUpdated) {
		//TODO calculate m_view
		m_view = glm::lookAtRH(m_pos,m_pos+m_dir,m_up);

		m_viewUpdated = true;
	}
}

void Camera::makePerpendicular() {
	if (!m_isPerp) {
		m_dir = glm::normalize(m_dir);
		m_right = glm::cross(glm::normalize(m_up),m_dir);
		if (m_noTilt) {
			m_right = glm::normalize(glm::vec3(m_right.x, 0.0f, m_right.z));
		}
		m_up = glm::cross(m_dir,m_right);
		m_isPerp = true;
	}
}

void CCkb::update(float deltaTime) {
	glm::vec3 d = m_pCam->getDir();
	glm::vec3 u = m_pCam->getUp();
	glm::vec3 r = m_pCam->getRight();
	if (m_state & SPT) deltaTime *= 2.0f;
	if (m_state & WLK) deltaTime *= 0.5f;
	if (m_state & FWD) m_pCam->setPos(m_pCam->getPos() + deltaTime*(d));
	if (m_state & BWD) m_pCam->setPos(m_pCam->getPos() - deltaTime*(d));
	if (m_state & LFT) m_pCam->setPos(m_pCam->getPos() + deltaTime*r);
	if (m_state & RGT) m_pCam->setPos(m_pCam->getPos() - deltaTime*r);
	if (m_state & UP) m_pCam->setPos(m_pCam->getPos() + deltaTime*u);
	if (m_state & DWN) m_pCam->setPos(m_pCam->getPos() - deltaTime*u);

	if (m_state & (VLFT+VRGT+VUP+VDWN)) {
		if (m_state & VLFT)
			yaw -= deltaTime*m_sens[0];
		else if (m_state & VRGT)
			yaw += deltaTime*m_sens[0];
		
		if (m_state & VUP)
			pitch += deltaTime*m_sens[1];
		else if (m_state & VDWN)
			pitch -= deltaTime*m_sens[1];
		
		if(pitch > 89.0f)
			pitch = 89.0f;
		if(pitch < -89.0f)
			pitch = -89.0f;
		glm::vec3 dir;
		dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		dir.y = sin(glm::radians(pitch));
		dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		m_pCam->setDir(glm::normalize(dir));
	}
}

void CCkb::callback(KeyboardData kd) {
	uint32_t s = 0;
	switch (kd.kk) {
	case IBCodes::KK_KEY_W: s = FWD; break;
	case IBCodes::KK_KEY_A: s = LFT; break;
	case IBCodes::KK_KEY_S: s = BWD; break;
	case IBCodes::KK_KEY_D: s = RGT; break;
	case IBCodes::KK_KEY_C: s = DWN; break;
	case IBCodes::KK_KEY_SPACE: s = UP; break;
	case IBCodes::KK_KEY_LEFT_SHIFT: s = SPT; break;
	case IBCodes::KK_KEY_LEFT_ALT: s = WLK; break;
	case IBCodes::KK_KEY_KP_1: s = VLFT; break;
	case IBCodes::KK_KEY_KP_3: s = VRGT; break;
	case IBCodes::KK_KEY_KP_2: s = VUP; break;
	case IBCodes::KK_KEY_KP_0: s = VDWN; break;
	default: break;
	}
	if (kd.ia==IBCodes::IA_PRESS)
		m_state |= s;
	else if (kd.ia==IBCodes::IA_RELEASE)
		m_state &= ~s;
}

}//ehj
