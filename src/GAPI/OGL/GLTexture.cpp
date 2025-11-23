#include "GLTexture.hpp"

//TODO
#ifndef STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

void GLTexture::construct() {
	int width, height, nrChannels;
	unsigned char* data = stbi_load(m_path.c_str(), &width, &height, &nrChannels, 0);

	printf("GLTex: num ch: %d\n",nrChannels);

	m_res.x=width;
	m_res.y=height;

	glGenTextures(1, &m_tex);
	glBindTexture(GL_TEXTURE_2D, m_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_texturefilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_texturefilter);

	glTexImage2D(GL_TEXTURE_2D, 0, m_internalformat, m_res.x, m_res.y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
}
