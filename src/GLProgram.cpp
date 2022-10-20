#include "GLProgram.hpp"
#include <fstream>
#include <sstream>

namespace ehj
{
	//GLProgram::GLProgram() {
	//
	//}
	void GLProgram::addSourceFromFile(std::string shaderPath, GLenum shaderType) {
		std::stringstream buffer;
		std::ifstream t(shaderPath);
		buffer << t.rdbuf();
		std::string shaderString = buffer.str();
		addSourceFromString(shaderString,shaderType);
	}
	void GLProgram::addSourceFromString(std::string shaderSource, GLenum shaderType) {
		auto itr = m_shaders.toID.find(shaderType);
		GLuint shaderID;
		if (itr == m_shaders.toID.end()) { // shader does not exist
			shaderID = glCreateShader(shaderType);
		} else
			shaderID = itr->second;
		const char* shaderSourceC = shaderSource.c_str();
		glShaderSource(shaderID, 1, &shaderSourceC, NULL);
		glCompileShader(shaderID);
		m_shaders.push_back(shaderID,shaderType);
	}
	void GLProgram::createProgram() {
		m_programID = glCreateProgram();
		
		auto itr = m_shaders.toEnum.begin();
		while (itr != m_shaders.toEnum.end()) {
			glAttachShader(m_programID, itr->first);
			itr++;
		}
		glLinkProgram(m_programID);
	}
	GLint GLProgram::getProgramID() {
		return m_programID;
	}
	void GLProgram::bind() {
		glUseProgram(m_programID);
	}
}
