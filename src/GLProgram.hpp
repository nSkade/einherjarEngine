#pragma once
#include "../lib/glad/glad.h"
#include <string>
#include <map>

#include "GLUtils.hpp"

namespace ehj
{
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
		} m_shaders;
	};
}