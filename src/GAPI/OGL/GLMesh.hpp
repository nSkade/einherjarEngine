#pragma once

#include <Mesh.hpp>

#include <glad/glad.h>

class GLMesh {
public:
	/**
	 * @param usage either GL_STATIC_DRAW, GL_DYNAMIC_DRAW, GL_STREAM_DRAW
	*/
	GLMesh(ehj::Mesh mesh) : GLMesh(mesh,GL_DYNAMIC_DRAW) {};
	GLMesh(ehj::Mesh mesh, GLenum usage);
	~GLMesh();

	void bind() {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	};

	void draw() {
		glDrawElements(GL_TRIANGLES,m_EBOsize,GL_UNSIGNED_INT,0);
	}

	uint32_t getEBO();
	uint32_t getEBOsize();
private:
	uint32_t m_EBO; // element buffer object
	uint32_t m_EBOsize;
};
