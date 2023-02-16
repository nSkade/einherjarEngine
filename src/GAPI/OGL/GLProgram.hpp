#pragma once
#include <string>
#include <map>

#include <glad/glad.h>

#include "GLUtils.hpp"

class GLProgram
{
public:
	GLProgram();
	~GLProgram();

	void createProgram();
	
	void bind(); //TODO remove?
	GLint getProgramID(); //TODO remove?
	GLuint getShaderID(GLenum shaderType);
	
	void addSourceFromString(std::string shaderSource, GLenum shaderType);
	void addSourceFromFile(std::string shaderPath, GLenum shaderType);

	void loadProgramFromFilename(std::string folderPath, std::string programName);
	void loadProgramFromFolder(std::string folderPath);

private:
	GLint m_programID = -1;
	struct biMap {
		std::map<GLuint,GLenum> toEnum;
		std::map<GLenum,GLuint> toID;
		void push_back(GLuint shaderID, GLenum shaderType) {
			toEnum.insert({shaderID,shaderType});
			toID.insert({shaderType,shaderID});
		}
		void clear() {
			toEnum.clear();
			toID.clear();
		}
		void remove(GLuint id) {
			auto e = toEnum.find(id);
			auto i = toID.find(e->second);
			toEnum.erase(e);
			toID.erase(i);
		}
	} m_shaders;
};
