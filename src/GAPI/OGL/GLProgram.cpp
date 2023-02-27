#include "GLProgram.hpp"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>

#include <iostream> //TODO remove?

namespace fs = std::filesystem;

GLProgram::GLProgram() {
	m_programID = glCreateProgram();
}

GLProgram::~GLProgram() {
	glDeleteProgram(m_programID);
}

void GLProgram::createProgram() {
	auto itr = m_shaders.toEnum.begin();
	while (itr != m_shaders.toEnum.end()) {
		glAttachShader(m_programID, itr->first);
		ehj_gl_err();
		itr++;
	}
	glLinkProgram(m_programID);
	for (auto id : m_shaders.toEnum) {
		glDeleteShader(id.first);
	}
	m_shaders.clear();
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
	if (itr != m_shaders.toID.end()) { // shader does already exist
		GLuint oldID = itr->second;
		glDeleteShader(oldID);
		m_shaders.remove(oldID);
	}
	ehj_gl_err();
	GLuint shaderID = glCreateShader(shaderType);
	if (shaderID == 0) {
		std::cerr << "GLProgram::addSourceFromString : glCreateShader error occured." << std::endl;
		return;
	}
	
	ehj_gl_err();
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

//TODO not implemented
void GLProgram::loadProgramFromFilename(std::string folderPath, std::string fileName) {
	for (const auto & entry : fs::directory_iterator(folderPath)) {
		std::cout << entry.path() << std::endl;
	
	}
}

GLenum GLProgram::detectShaderType(std::string fileName) {
	GLenum shaderType = GL_INVALID_ENUM;
	
	std::string name = fileName;

	if (name.find(".vert") != name.npos |
	    name.find("_v.") != name.npos)
		shaderType = GL_VERTEX_SHADER;
	
	if (name.find(".frag") != name.npos |
	    name.find("_f.") != name.npos)
		shaderType = GL_FRAGMENT_SHADER;

	if (name.find("tcs") != name.npos)
		shaderType = GL_TESS_CONTROL_SHADER;

	if (name.find("tes") != name.npos)
		shaderType = GL_TESS_EVALUATION_SHADER;
	return shaderType;
}

void GLProgram::loadProgramFromFolder(std::string folderPath) {
	for (const auto & entry : fs::directory_iterator(folderPath)) {
		if (entry.is_directory())
			continue;
		std::cout << entry.path() << std::endl;
		
		GLenum shaderType = detectShaderType(entry.path().filename().string());

		switch (shaderType)
		{
		case GL_FRAGMENT_SHADER:
			std::cout << "GL_FRAGMENT_SHADER\n";
			break;
		case GL_VERTEX_SHADER:
			std::cout << "GL_VERTEX_SHADER\n";
			break;
		case GL_TESS_CONTROL_SHADER:
			std::cout << "GL_TESS_CONTROL_SHADER\n";
			break;
		case GL_TESS_EVALUATION_SHADER:
			std::cout << "GL_TESS_EVALUATION_SHADER\n";
			break;
		case GL_GEOMETRY_SHADER:
			std::cout << "GL_GEOMETRY_SHADER\n";
			break;
		default:
			break;
		}

		if (shaderType != GL_INVALID_ENUM)
			addSourceFromFile(entry.path().string(),shaderType);
	}
}

void GLProgram::addSourceFromFile(std::string shaderPath) {
	std::stringstream buffer;
	std::ifstream t(shaderPath);
	buffer << t.rdbuf();
	std::string shaderString = buffer.str();

	std::filesystem::path sp(shaderPath);
	GLenum shaderType = detectShaderType(sp.filename().string());

	addSourceFromString(shaderString,shaderType);
}

GLint GLProgram::getUnfLoc(std::string name) {
	GLint loc;
	if (m_uniformLocations.find(name) != m_uniformLocations.end()) {
		loc = m_uniformLocations[name];
	} else { // uniform location does not exist
		loc = glGetUniformLocation(m_programID, name.c_str());
		m_uniformLocations[name] = loc;
	}
	return loc;
}

void GLProgram::clearUniformLocations() {
	m_uniformLocations.clear();
}
