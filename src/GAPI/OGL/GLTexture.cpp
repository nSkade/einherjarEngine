#include "GLTexture.hpp"

void GLTexture::construct() {
	int width, height, nrChannels;

	std::cout << "GLTex: num ch: " << nrChannels << "\n";

	glGenTextures(1, &m_tex);
	glBindTexture(GL_TEXTURE_2D, m_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_opt.texturefilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_opt.texturefilter);

	if (m_opt.path.length()>0) {
		unsigned char* data = stbi_load(m_opt.path.c_str(), &width, &height, &nrChannels, 0);
		m_res.x=width;
		m_res.y=height;
		//TODO GL_RGB and GL_UNSIGNED_BYTE
		glTexImage2D(GL_TEXTURE_2D, 0, m_opt.internalformat, m_res.x, m_res.y,
					0, GL_RGB, GL_UNSIGNED_BYTE, data);
	} else if (m_opt.width > 0 && m_opt.height > 0) {
		//TODO GL_RGB and GL_FLOAT
		glTexImage2D(GL_TEXTURE_2D, 0, m_opt.internalformat, m_opt.width, m_opt.height,
					0, GL_RGB, GL_FLOAT, NULL);
	} else {
		std::cerr << "GLTex cannot create Texture\n";
	}
	glGenerateMipmap(GL_TEXTURE_2D);
}
