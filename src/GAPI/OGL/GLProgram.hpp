#pragma once

#include <glad/glad.h>

#include "GLUtils.hpp"

#include <string>
#include <map>

class GLProgram
{
public:
	GLProgram();
	~GLProgram();

	void createProgram();
	
	void bind(); //TODO remove?
	GLint getProgramID(); //TODO remove?
	GLuint getShaderID(GLenum shaderType);
	
	void addSourceFromString(std::string shaderSource, GLenum shaderType, const std::string& filePath);
	void addSourceFromFile(std::string shaderPath, GLenum shaderType);
	void addSourceFromFile(std::string shaderPath);

	static std::string loadFileContents(std::string path);

	/**
	 * @brief resolves '#include "othershader"' in shaderSource recursively
	*/
	void resolveInclude(const std::string shaderPath, std::string* shaderSource, uint32_t curDepth);
	/**
	 * @brief loads shaderFile from given Path and resolves '#include' directives
	*/
	void addSourceFromFileRecursive(std::string shaderPath);

	GLenum detectShaderType(std::string fileName);
	void loadProgramFromFilename(std::string folderPath, std::string programName);
	void loadProgramFromFolder(std::string folderPath);

	/**
	 * @brief Returns Uniform Location.
	*/
	GLint getUnfLoc(std::string name);
	void clearUniformLocations();

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
	std::map<std::string,GLint> m_uniformLocations;

	uint32_t m_maxIncludeDepth = 10;
};
