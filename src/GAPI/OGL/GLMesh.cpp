#include "GLMesh.hpp"

#include "GLUtils.hpp"

GLMesh::GLMesh(ehj::Mesh mesh, GLenum usage) {

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

GLMesh::~GLMesh() {
	glDeleteBuffers(1, &m_EBO);
	glDeleteBuffers(1, &m_VBO);
	glDeleteVertexArrays(1, &m_VAO);
}

void GLMesh::bind(uint32_t bindingIndex) {
	m_vaoBindingPoint = bindingIndex;
	
	//TODO uint32_t Dim = m_Dim;
	//uint32_t MPC = 1; // Vertex
	//if (m_MP & ehj::Mesh::MP_NORMAL)
	//	++MPC;
	//if (m_MP & ehj::Mesh::MP_COLOR)
	//	++MPC;
	//if (m_MP & ehj::Mesh::MP_UV)
	//	++MPC;
	
	uint32_t MPC = 3;
	if (m_MP & ehj::Mesh::MP_NORMAL)
		MPC += 3;
	if (m_MP & ehj::Mesh::MP_COLOR)
		MPC += 3;
	if (m_MP & ehj::Mesh::MP_UV)
		MPC += 2;

	//TODO remove,,, ehj_gl_err();
	glVertexArrayVertexBuffer(m_VAO,m_vaoBindingPoint,m_VBO,0,sizeof(float)*MPC);
	//glVertexArrayVertexBuffer(m_VAO,m_vaoBindingPoint,m_VBO,0,sizeof(float)*Dim*MPC);

	//TODO remove,,, ehj_gl_err();

	// position
	glEnableVertexArrayAttrib(m_VAO, m_attribPos);
	glVertexArrayAttribFormat(m_VAO, m_attribPos, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(m_VAO, m_attribPos, m_vaoBindingPoint);

	int offset = 3 * sizeof(float);

	if (m_MP & ehj::Mesh::MP_NORMAL) {
		glEnableVertexArrayAttrib(m_VAO, m_attribNrm);
		glVertexArrayAttribFormat(m_VAO, m_attribNrm, 3, GL_FLOAT, GL_FALSE, offset);
		glVertexArrayAttribBinding(m_VAO, m_attribNrm, m_vaoBindingPoint);
		offset += 3 * sizeof(float);
	} else {
		glDisableVertexArrayAttrib(m_VAO, m_attribNrm);
		//m_attribNrm=-1; //TODO do this?
	}

	if (m_MP & ehj::Mesh::MP_COLOR) {
		glEnableVertexArrayAttrib(m_VAO, m_attribCol);
		glVertexArrayAttribFormat(m_VAO, m_attribCol, 3, GL_FLOAT, GL_FALSE, offset);
		glVertexArrayAttribBinding(m_VAO, m_attribCol, m_vaoBindingPoint);
		offset += 3 * sizeof(float);
	} else {
		glDisableVertexArrayAttrib(m_VAO, m_attribCol);
		//m_attribCol=-1; //TODO do this?, see glBindAttribLocation(glp.getID(),glMesh.getAttribCol(),"vCol");
	}

	if (m_MP & ehj::Mesh::MP_UV) {
		glEnableVertexArrayAttrib(m_VAO, m_attribUV);
		glVertexArrayAttribFormat(m_VAO, m_attribUV, 2, GL_FLOAT, GL_FALSE, offset);
		glVertexArrayAttribBinding(m_VAO, m_attribUV, m_vaoBindingPoint);
	} else {
		glDisableVertexArrayAttrib(m_VAO, m_attribUV);
		//m_attribUV=-1; //TODO do this?,
	}
}

uint32_t GLMesh::getVAO() {
	return m_VAO;
}

uint32_t GLMesh::getEBO() {
	return m_EBO;
}

uint32_t GLMesh::getEBOsize() {
	return m_EBOsize;
}

int32_t GLMesh::getAttribPos() const {
	return m_attribPos;
}
int32_t GLMesh::getAttribNrm() const {
	return m_attribNrm;
}
int32_t GLMesh::getAttribCol() const {
	return m_attribCol;
}
int32_t GLMesh::getAttribUV() const {
	return m_attribUV;
}
