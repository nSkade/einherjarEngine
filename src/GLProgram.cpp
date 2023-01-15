#include "GLProgram.hpp"

#include <fstream>
#include <sstream>

#include <string>
#include <iostream>
#include <filesystem>

#include <iostream> //TODO remove

namespace fs = std::filesystem;

namespace ehj
{
	void GLProgram::createProgram() {
		m_programID = glCreateProgram();
		auto itr = m_shaders.toEnum.begin();
		while (itr != m_shaders.toEnum.end()) {
			glAttachShader(m_programID, itr->first);
			ehj_gl_err();
			itr++;
		}
		glLinkProgram(m_programID);
		ehj_gl_err();
	}

	GLint GLProgram::getProgramID() {
		return m_programID;
	}
	
	void GLProgram::bind() {
		glUseProgram(m_programID);
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
		ehj_gl_err();
		glCompileShader(shaderID);
		ehj_gl_err();
		m_shaders.push_back(shaderID,shaderType);
	}
	
	void GLProgram::addSourceFromFile(std::string shaderPath, GLenum shaderType) {
		std::stringstream buffer;
		std::ifstream t(shaderPath);
		buffer << t.rdbuf();
		std::string shaderString = buffer.str();
		addSourceFromString(shaderString,shaderType);
	}
	
	void GLProgram::loadProgramFromFilename(std::string folderPath, std::string fileName) {
		for (const auto & entry : fs::directory_iterator(folderPath)) {
			std::cout << entry.path() << std::endl;
		
		}
	}
	
	void GLProgram::loadProgramFromFolder(std::string folderPath) {
		for (const auto & entry : fs::directory_iterator(folderPath)) {
			std::cout << entry.path() << std::endl;

		}
	}
}