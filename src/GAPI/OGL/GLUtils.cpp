#include "GLUtils.hpp"

#include <glad/glad.h>

void ehj_gl_err_callback() {
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

	auto debugCallback = [](GLenum source, GLenum type, unsigned int id, GLenum severity, 
							   GLsizei length, const char* message, const void* userParam) {
		if (type == GL_DEBUG_TYPE_ERROR) {
			std::cerr << "GL Error: " << message << std::endl;
			__debugbreak();
		}
	};
	glDebugMessageCallback(debugCallback, nullptr);
}

uint32_t ehj_gl_err() {
	unsigned int err = 0;
	err = glGetError();
	while ((err != 0)) {
		std::string str = "";
		switch (err)
		{
			case GL_INVALID_ENUM:				   str = "INVALID_ENUM"; break;
			case GL_INVALID_VALUE:				   str = "INVALID_VALUE"; break;
			case GL_INVALID_OPERATION:			   str = "INVALID_OPERATION"; break;
			case GL_STACK_OVERFLOW:				   str = "STACK_OVERFLOW"; break;
			case GL_STACK_UNDERFLOW:			   str = "STACK_UNDERFLOW"; break;
			case GL_OUT_OF_MEMORY:				   str = "OUT_OF_MEMORY"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: str = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cerr << "\nehj_gl_err: " << err << " " << str << "\n";
		err = glGetError();
		if (EHJ_EXIT_ON_GL_ERR)
		#ifdef EHJ_DBG
			__debugbreak();
		#else
			exit(-1);
		#endif
		else
			return 1;
	}
	return 0;
};

uint32_t ehj_gl_err_continue() {
	unsigned int err = 0;
	err = glGetError();
	while ((err != 0)) {
		std::string str = "";
		switch (err)
		{
			case GL_INVALID_ENUM:				   str = "INVALID_ENUM"; break;
			case GL_INVALID_VALUE:				   str = "INVALID_VALUE"; break;
			case GL_INVALID_OPERATION:			   str = "INVALID_OPERATION"; break;
			case GL_STACK_OVERFLOW:				   str = "STACK_OVERFLOW"; break;
			case GL_STACK_UNDERFLOW:			   str = "STACK_UNDERFLOW"; break;
			case GL_OUT_OF_MEMORY:				   str = "OUT_OF_MEMORY"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: str = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cerr << "\nehj_gl_err: " << err << " " << str << "\n";
		err = glGetError();
	}
	return 0;
};
