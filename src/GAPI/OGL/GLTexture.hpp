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
		std::string path;
	};

	GLTexture(std::string path) :
		m_path(path)
		{
		construct();
	}

	GLTexture(Opt opt) :
		m_internalformat(opt.internalformat),
		m_texturefilter(opt.texturefilter),
		m_path(opt.path)
		{
		construct();
	}

	GLTexture(const GLTexture& a) {
		m_res = a.m_res;
		m_internalformat = a.m_internalformat;
		m_texturefilter = a.m_texturefilter;
		construct();
	}

	GLTexture& operator=(const GLTexture& a) {
		if (this != &a) {
			destruct();     // Clean old resources
			m_res = a.m_res;
			m_internalformat = a.m_internalformat;
			m_texturefilter = a.m_texturefilter;
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
	std::string m_path;
	GLuint m_tex; // color texture
	glm::ivec2 m_res;
	GLint m_internalformat = GL_RGBA32F;
	GLint m_texturefilter = GL_LINEAR; //TODO
};
