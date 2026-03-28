#include "GLMesh.hpp"

GLMesh::GLMesh(ehj::Mesh mesh, GLenum usage) {

	std::vector<int> indexBuffer = mesh.getIndexBuffer();
	
	m_EBOsize = indexBuffer.size();
	
	glCreateBuffers(1,&m_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size()*sizeof(int), &indexBuffer[0], usage);
}

GLMesh::~GLMesh() {
	glDeleteBuffers(1, &m_EBO);
}

uint32_t GLMesh::getEBO() {
	return m_EBO;
}

uint32_t GLMesh::getEBOsize() {
	return m_EBOsize;
}
