#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

using namespace glm;

//TODO implement, cur just copy pasted
class GLTexture {
public:
	struct Opt {
		GLint internalformat = GL_RGBA32F;
		GLint texturefilter = GL_LINEAR;
		std::string path = "";
		int width=0;
		int height=0;
	} m_opt;

	GLTexture(std::string path) :
		m_opt(Opt{GL_RGBA32F,GL_LINEAR,path})
		{
		construct();
	}

	GLTexture(Opt opt) :
		m_opt(opt)
		{
		construct();
	}

	GLTexture(const GLTexture& a) {
		m_res = a.m_res;
		m_opt = a.m_opt;
		construct();
	}

	GLTexture& operator=(const GLTexture& a) {
		if (this != &a) {
			destruct();     // Clean old resources
			m_res = a.m_res;
			m_opt=a.m_opt;
			construct();    // Re-create with new size
		}
		return *this;
	}

	~GLTexture() {
		destruct();
	}

	GLuint getTex() { return m_tex; };
	glm::ivec2 getRes() { return m_res; };

private:
	void construct();
	void destruct() {
		glDeleteTextures(1,&m_tex);
	}
	GLuint m_tex; // color texture
	glm::ivec2 m_res;
};
