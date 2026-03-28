#include "GLTexture.hpp"
#include <Utility/Timer.hpp>

#include <Utility/ConfigFile.hpp>

void GLTexture::construct() {
constexpr bool profile=false;

	int width, height, nrChannels;

	//std::cout << "GLTex: num ch: " << nrChannels << "\n";
ehj::Timer t1;

	glGenTextures(1, &m_tex);
	glBindTexture(GL_TEXTURE_2D, m_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_opt.wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_opt.wrapT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_opt.texturefilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_opt.texturefilter);
	if (profile) {
		static float time3 = 0.;
		time3+=t1.endTimer();
		std::cout << "gl gen tex time: " << time3 << "ms\n"; t1.startTimer();
	}

	static float time = 0.;
	static float time2 = 0.;
	if (m_opt.path.length()>0 || m_opt.data) {
		unsigned char* data = nullptr;
		if (m_opt.data) {
			data = m_opt.data;
			width = m_opt.width; height=m_opt.height; nrChannels=m_opt.nrChannels;
		}
		else
			data = stbi_load(m_opt.path.c_str(), &width, &height, &nrChannels, 0); //TODO free mem
		if (profile) {
			//std::cout << "stbi load in in: " << t1.endTimer() << "ms\n"; t1.startTimer();
			time+=t1.endTimer();
			std::cout << "total stbi load time: " << time << "ms\n"; t1.startTimer();
		}

		m_res.x=width;
		m_res.y=height;

		GLint channels = GL_RGBA;
		//TODO GL_RGB and GL_UNSIGNED_BYTE
		if (nrChannels==3)
			channels = GL_RGB;

		glTexImage2D(GL_TEXTURE_2D, 0, m_opt.internalformat, m_res.x, m_res.y,
					0, channels, GL_UNSIGNED_BYTE, data);
		if (profile) {
			time2+=t1.endTimer();
			std::cout << "total gpu upload time: " << time2 << "ms\n"; t1.startTimer();
		}
	} else if (m_opt.width > 0 && m_opt.height > 0) {
		//TODO GL_RGB and GL_FLOAT
		glTexImage2D(GL_TEXTURE_2D, 0, m_opt.internalformat, m_opt.width, m_opt.height,
					0, GL_RGB, GL_FLOAT, NULL);
	} else {
		std::cerr << "GLTex cannot create Texture\n";
	}
	//glGenerateMipmap(GL_TEXTURE_2D); //TODO very expensive on load time
}
