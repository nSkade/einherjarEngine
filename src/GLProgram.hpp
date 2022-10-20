
#pragma once
#include "../glad/glad.h"
#include <string>
#include <unordered_map>

namespace ehj
{
	class GLProgram
	{
	public:
		//GLProgram();
		//~GLProgram();

		void addSourceFromFile(std::string shaderPath, GLenum shaderType);
		void addSourceFromString(std::string shaderSource, GLenum shaderType);

		void createProgram();
		
		void bind();
		GLint getProgramID();
		GLuint getShaderID(GLenum shaderType);

	private:
		GLint m_programID = -1;
		struct biMap {
			std::unordered_map<GLuint,GLenum> toEnum;
			std::unordered_map<GLenum,GLuint> toID;
			void push_back(GLuint shaderID, GLenum shaderType) {
				toEnum.insert({shaderID,shaderType});
				toID.insert({shaderType,shaderID});
			}
		} m_shaders;
	};
}
