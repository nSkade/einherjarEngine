#include "GLUtils.hpp"

uint32_t ehj_gl_err() {
	unsigned int err = 0;
	err = glGetError();
	while ((err != 0)) {
		std::cout << "\nehj_gl_err: " << err << "\n";
		err = glGetError();
		if (EXIT_ON_GL_ERR)
			exit(1);
		else
			return 1;
	}
	return 0;
};
