
#include <glad/glad.h>
#include <glm/glm.hpp>

//TODO implement, cur just copy pasted
class GLFrameBuffer {
public:
	GLFrameBuffer(glm::ivec2 res) {
		glGenFramebuffers(1,&m_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER,m_fbo);

		glGenTextures(1, &m_tCol);
		glBindTexture(GL_TEXTURE_2D, m_tCol);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, res.x, res.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tCol, 0);
		
		glGenTextures(1, &m_tDep);
		glBindTexture(GL_TEXTURE_2D, m_tDep);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, res.x, res.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_tDep, 0);
	}

	~GLFrameBuffer() {
		glDeleteTextures(1,&m_tCol);
		glDeleteTextures(1,&m_tDep);
		glDeleteFramebuffers(1,&m_fbo);
	}

	void update(glm::ivec2 res) {
		//TODO do these reset?
		glBindTexture(GL_TEXTURE_2D, m_tCol);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, res.x, res.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, m_tDep);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, res.x, res.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

		glBlitNamedFramebuffer(m_fbo,0,0,0,res.x,res.y,0,0,res.x,res.y,GL_COLOR_BUFFER_BIT,GL_NEAREST);
	}

private:
	GLuint m_fbo;
	GLuint m_tCol; // color texture
	GLuint m_tDep; // depth texture

};
