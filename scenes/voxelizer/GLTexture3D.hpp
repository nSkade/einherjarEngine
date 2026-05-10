#pragma once

#include "suOGL.hpp"
#include <glad/glad.h>

#include <string>

#include <thread>

using namespace glm;

class GLTexture3D {
public:
	struct Opt {
		std::vector<std::string> paths;
		GLint internalformat = GL_RGB8;
		GLint texturefilter = GL_NEAREST;
		GLint wrapS = GL_CLAMP_TO_BORDER;
		GLint wrapT = GL_CLAMP_TO_BORDER;
		int width=0;
		int height=0;
		std::vector<unsigned char*> data;
		int nrChannels=4;
	} m_opt;

	GLTexture3D(std::vector<std::string> paths) :
		m_opt(Opt{paths})
		{
		construct();
	}

	GLTexture3D(const Opt& opt) :
		m_opt(opt)
		{
		construct();
	}

	GLTexture3D(const GLTexture3D& a) {
		m_res = a.m_res;
		m_opt = a.m_opt;
		construct();
	}

	GLTexture3D& operator=(const GLTexture3D& a) {
		if (this != &a) {
			destruct();		// Clean old resources
			m_res = a.m_res;
			m_opt=a.m_opt;
			construct();	// Re-create with new size
		}
		return *this;
	}

	~GLTexture3D() {
		destruct();
	}

	GLuint getTex() { return m_tex; };
	glm::ivec3 getRes() { return m_res; };

private:
	void construct();
	void destruct() {
		glDeleteTextures(1,&m_tex);
	}
	GLuint m_tex; // color texture
	glm::ivec3 m_res;
};

inline void GLTexture3D::construct() {
	int width, height, nrChannels;

	glGenTextures(1, &m_tex);
	glBindTexture(GL_TEXTURE_3D, m_tex);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, m_opt.wrapS);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, m_opt.wrapT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, m_opt.texturefilter);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, m_opt.texturefilter);

	std::vector<unsigned char*> data;
	if (m_opt.paths.size()>0 || m_opt.data.size() > 0) {
		if (m_opt.data.size() > 0) {
			data = m_opt.data;
			width = m_opt.width; height=m_opt.height; nrChannels=m_opt.nrChannels;
		}
		else {
			data.resize(m_opt.paths.size());
			std::vector<std::thread> loadThreads;
			for (int i=0;i<m_opt.paths.size();++i) {
				loadThreads.emplace_back([this,&data,i,&width,&height,&nrChannels](){
					int cwidth, cheight, cnrChannels;
					data[i] = stbi_load(m_opt.paths[i].c_str(), &cwidth, &cheight, &cnrChannels, 0);
					if (i==0) {
						width=cwidth;
						height=cheight;
						nrChannels=cnrChannels;
					}
				});
			}
			for (auto& t : loadThreads)
				t.join();
		}

		m_res.x=width;
		m_res.y=height;

		GLint channels = GL_RGB;//(nrChannels==1) ? GL_RED : (nrChannels==3) ? GL_RGB : GL_RGBA;//TODO

		//TODOf		
		//GLint internalFormat = (nrChannels==1) ? GL_R8 : (nrChannels==3) ? GL_RGB8 : GL_RGBA8;
		//GLenum format = (nrChannels==1) ? GL_RED : (nrChannels==3) ? GL_RGB : GL_RGBA;
		//GLenum type = GL_UNSIGNED_BYTE;

		//glTexImage2D(GL_TEXTURE_2D, 0, m_opt.internalformat, m_res.x, m_res.y,
		//			0, channels, GL_UNSIGNED_BYTE, data);
		
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // safe for tightly packed data
		glTexImage3D(GL_TEXTURE_3D,
					 0,                    // mip level
					 m_opt.internalformat,       // internal format
					 width, height, (GLsizei)m_opt.paths.size(), // width, height, depth
					 0,                    // border
					channels, GL_UNSIGNED_BYTE,
					 nullptr);             // no data yet
		// Upload each slice as a z-offset
		for(int z=0; z<m_opt.paths.size(); ++z){
//(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
			glTexSubImage3D(GL_TEXTURE_3D,
							0,             // mip
							0, 0, z,       // x,y,z offset
							width, height, 1,       // width,height,depth (1 slice)
							channels, GL_UNSIGNED_BYTE,
							data[z]);
		}
	} else if (m_opt.width > 0 && m_opt.height > 0) {
		//TODOff empty 3d texture
		std::cerr << "GLTex3D empty 3d texture not impl yet\n";
		////TODO GL_RGB and GL_FLOAT
		//glTexImage2D(GL_TEXTURE_2D, 0, m_opt.internalformat, m_opt.width, m_opt.height,
		//			0, GL_RGB, GL_FLOAT, NULL);
	} else {
		std::cerr << "GLTex3D cannot create Texture\n";
	}
	//glGenerateMipmap(GL_TEXTURE_2D); //TODO very expensive on load time
}
