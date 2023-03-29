#include "OGLMesh.hpp"

#include "GLUtils.hpp"

OGLMesh::OGLMesh(ehj::Mesh mesh, GLenum usage) {

	m_MP = mesh.getMP();
	m_Dim = mesh.getDim();

	std::vector<float> vertexBuffer = mesh.getVertexBuffer();
	std::vector<int> indexBuffer = mesh.getIndexBuffer();

	m_EBOsize = indexBuffer.size();
	
	//TODO opengl 4.6 not available
	glCreateBuffers(1,&m_VBO);
	glNamedBufferStorage(m_VBO, vertexBuffer.size()*sizeof(float), &vertexBuffer[0], GL_DYNAMIC_STORAGE_BIT);
	//glBufferData(GL_ARRAY_BUFFER,vertexBuffer.size()*sizeof(float), &vertexBuffer[0], GL_DYNAMIC_DRAW);
	glCreateVertexArrays(1,&m_VAO);
	
	glCreateBuffers(1,&m_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size()*sizeof(int), &indexBuffer[0], usage);
}

OGLMesh::~OGLMesh() {
	glDeleteBuffers(1, &m_EBO);
	glDeleteBuffers(1, &m_VBO);
	glDeleteVertexArrays(1, &m_VAO);
}

void OGLMesh::bind(uint32_t bindingIndex) {
	m_vaoBindingPoint = bindingIndex;
	
	uint32_t Dim = m_Dim;
	uint32_t MPC = 1; // Vertex
	if (m_MP & ehj::Mesh::MP_NORMAL)
		++MPC;
	if (m_MP & ehj::Mesh::MP_COLOR)
		++MPC;

	ehj_gl_err();
	glVertexArrayVertexBuffer(m_VAO,m_vaoBindingPoint,m_VBO,0,sizeof(float)*Dim*MPC);

	ehj_gl_err();
	glEnableVertexArrayAttrib(m_VAO,m_attribPos);
	glVertexArrayAttribFormat(m_VAO,m_attribPos,3,GL_FLOAT,GL_FALSE,0);
	glVertexArrayAttribBinding(m_VAO,m_attribPos,m_vaoBindingPoint);
	
	if (m_MP & ehj::Mesh::MP_NORMAL) {
		glEnableVertexArrayAttrib(m_VAO,m_attribNrm);
		glVertexArrayAttribFormat(m_VAO,m_attribNrm,3,GL_FLOAT,GL_FALSE,3*sizeof(float));
		glVertexArrayAttribBinding(m_VAO,m_attribNrm,m_vaoBindingPoint);
	} else
		m_attribNrm = -1;

	if (m_MP & ehj::Mesh::MP_COLOR) {
		glEnableVertexArrayAttrib(m_VAO,m_attribCol);
		//TODO offset calculation might be unstable
		glVertexArrayAttribFormat(m_VAO,m_attribCol,3,GL_FLOAT,GL_FALSE, (MPC-1)*3*sizeof(float));
		glVertexArrayAttribBinding(m_VAO,m_attribCol,m_vaoBindingPoint);
	}
	else
		m_attribCol = -1;
}

uint32_t OGLMesh::getVAO() {
	return m_VAO;
}

uint32_t OGLMesh::getEBO() {
	return m_EBO;
}

uint32_t OGLMesh::getEBOsize() {
	return m_EBOsize;
}

int32_t OGLMesh::getAttribPos() {
	return m_attribPos;
}
int32_t OGLMesh::getAttribNrm() {
	return m_attribNrm;
}
int32_t OGLMesh::getAttribCol() {
	return m_attribCol;
}
