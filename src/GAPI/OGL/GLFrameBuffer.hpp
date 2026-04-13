#include <glad/glad.h>

using namespace glm;

//TODO implement, cur just copy pasted
class GLFrameBuffer {
public:
	struct Opt {
		ivec2 res = ivec2(1,1);
		GLint internalformat = GL_RGBA32F;
		GLint texturefilter = GL_LINEAR;
	};

	GLFrameBuffer(ivec2 res) :
		m_res(res) {
		construct();
	}

	GLFrameBuffer(Opt opt) :
		m_res(opt.res),
		m_internalformat(opt.internalformat),
		m_texturefilter(opt.texturefilter)
		{
		construct();
	}

	GLFrameBuffer(const GLFrameBuffer& a) {
		m_res = a.m_res;
		m_internalformat = a.m_internalformat;
		m_texturefilter = a.m_texturefilter;
		construct();
	}

	GLFrameBuffer& operator=(const GLFrameBuffer& a) {
		if (this != &a) {
			destruct();     // Clean old resources
			m_res = a.m_res;
			m_internalformat = a.m_internalformat;
			m_texturefilter = a.m_texturefilter;
			construct();    // Re-create with new size
		}
		return *this;
	}

	~GLFrameBuffer() {
		destruct();
	}

	GLuint getFBO() { return m_fbo; };
	GLuint getTexCol() { return m_tCol; };
	GLuint getTexDep() { return m_tDep; };
	glm::ivec2 getRes() { return m_res; };

	//TODO deprecated
	//void update(glm::ivec2 res) {
	//	//TODO do these reset?
	//	glBindTexture(GL_TEXTURE_2D, m_tCol);
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_res.x, m_res.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	//	glBindTexture(GL_TEXTURE_2D, m_tDep);
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_res.x, m_res.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

	//	glBlitNamedFramebuffer(m_fbo,0,0,0,res.x,res.y,0,0,res.x,res.y,GL_COLOR_BUFFER_BIT,GL_NEAREST);
	//}

private:
	void construct() {
		glGenFramebuffers(1,&m_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER,m_fbo);

		glGenTextures(1, &m_tCol);
		glBindTexture(GL_TEXTURE_2D, m_tCol);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_texturefilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_texturefilter);

		//TODO(skade) abstract
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, res.x, res.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexImage2D(GL_TEXTURE_2D, 0, m_internalformat, m_res.x, m_res.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tCol, 0);
		
		glGenTextures(1, &m_tDep);
		glBindTexture(GL_TEXTURE_2D, m_tDep);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_texturefilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_texturefilter);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_res.x, m_res.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_tDep, 0);
		glBindFramebuffer(GL_FRAMEBUFFER,0);
	}
	void destruct() {
		glDeleteTextures(1,&m_tCol);
		glDeleteTextures(1,&m_tDep);
		glDeleteFramebuffers(1,&m_fbo);
	}
	GLuint m_fbo;
	GLuint m_tCol; // color texture
	GLuint m_tDep; // depth texture
	glm::ivec2 m_res;
	GLint m_internalformat = GL_RGBA32F;
	GLint m_texturefilter = GL_LINEAR; //TODO
};
