#pragma once

#include <Mesh.hpp>

#include <glad/glad.h>
#include <stdint.h>

class GLMesh {
public:
	/**
	 * @param usage either GL_STATIC_DRAW, GL_DYNAMIC_DRAW, GL_STREAM_DRAW
	*/
	GLMesh(ehj::Mesh mesh) : GLMesh(mesh,GL_DYNAMIC_DRAW) {};
	GLMesh(ehj::Mesh mesh, GLenum usage);
	~GLMesh();

	void bind(uint32_t bindingIndex);

	uint32_t getVAO();
	uint32_t getEBO();
	uint32_t getEBOsize();

	int32_t getAttribPos() const;
	int32_t getAttribNrm() const;
	int32_t getAttribCol() const;
	int32_t getAttribUV()  const;

private:
	uint32_t m_MP = 0; // mesh properties
	uint32_t m_Dim = 3;

	uint32_t m_VAO; // vertex array object
	uint32_t m_VBO; // vertex buffer object
	uint32_t m_EBO; // element buffer object
	
	uint32_t m_EBOsize;

	//TODO abstract this
	GLuint m_attribPos = 0;
	GLuint m_attribNrm = 1;
	GLuint m_attribCol = 2;
	GLuint m_attribUV = 3;

	GLuint m_vaoBindingPoint = 0; //TODO manage
};

