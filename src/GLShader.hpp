
#pragma once
#include "../glad/glad.h"
#include <string>

namespace ehj
{
	class GLShader
	{
	public:
		GLShader();
		~GLShader();
		void loadFromFile(std::string vertexShaderPath, std::string fragmentShaderPath);
		void bind();
		void getProgramID();

	private:
		GLint m_programID;
		GLuint m_vertexShaderID;
		GLuint m_fragmentShaderID;
	}
}