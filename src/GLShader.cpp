#include "GLShader.hpp"

namespace ehj
{
	void GLShader::GLShader()
	{

	}
	void GLShader::loadFromFile(std::string vertexShaderPath, std::string fragmentShaderPath)
	{

	}
	GLint GLShader::getProgramID()
	{
		return m_programID;
	}
	void GLShader::bind()
	{
		glUseProgram(m_programID);
	}
}