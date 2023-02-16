#include "GLUtils.hpp"

uint32_t ehj_gl_err() {
	unsigned int err = 0;
	err = glGetError();
	while ((err != 0)) {
		std::string str = "";
		switch (err)
		{
			case GL_INVALID_ENUM:                  str = "INVALID_ENUM"; break;
			case GL_INVALID_VALUE:                 str = "INVALID_VALUE"; break;
			case GL_INVALID_OPERATION:             str = "INVALID_OPERATION"; break;
			case GL_STACK_OVERFLOW:                str = "STACK_OVERFLOW"; break;
			case GL_STACK_UNDERFLOW:               str = "STACK_UNDERFLOW"; break;
			case GL_OUT_OF_MEMORY:                 str = "OUT_OF_MEMORY"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: str = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << "\nehj_gl_err: " << err << " " << str << "\n";
		err = glGetError();
		if (EXIT_ON_GL_ERR)
			__debugbreak();
			//exit(1);
		else
			return 1;
	}
	return 0;
};
