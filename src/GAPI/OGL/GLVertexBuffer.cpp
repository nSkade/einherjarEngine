#include "GLVertexBuffer.hpp"
#include "Mesh.hpp"
#include <stdexcept>

using namespace ehj;

GLVertexBuffer::GLVertexBuffer(const ehj::VertexData& vertexData, GLbitfield flags) {
	if (vertexData.merged.size() == 0)
		throw std::runtime_error("vertexData not assembled");
	//TODOf m_Dim = mesh.m_Dim;
	m_VP = vertexData.m_VP;
	m_Dim= vertexData.m_Dim;

	//TODO opengl 4.6 not available
	glCreateBuffers(1,&m_VBO);
	glNamedBufferStorage(m_VBO, vertexData.merged.size()*sizeof(float), &vertexData.merged[0], flags);
	//glBufferData(GL_ARRAY_BUFFER,vertexBuffer.size()*sizeof(float), &vertexBuffer[0], GL_DYNAMIC_DRAW);
	glCreateVertexArrays(1,&m_VAO);
}

GLVertexBuffer::~GLVertexBuffer() {
	glDeleteBuffers(1, &m_VBO);
	glDeleteVertexArrays(1, &m_VAO);
}

void GLVertexBuffer::bind(uint32_t bindingIndex) {
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
	if (m_VP & VertexData::VP_NRM)
		MPC += 3;
	if (m_VP & VertexData::VP_COL)
		MPC += 3;
	if (m_VP & VertexData::VP_UV)
		MPC += 2;
	if (m_VP & VertexData::VP_TAN)
		MPC += 4;

	glVertexArrayVertexBuffer(m_VAO,m_vaoBindingPoint,m_VBO,0,sizeof(float)*MPC);
	//glVertexArrayVertexBuffer(m_VAO,m_vaoBindingPoint,m_VBO,0,sizeof(float)*Dim*MPC);


	// position
	glEnableVertexArrayAttrib(m_VAO, m_attribPos);
	glVertexArrayAttribFormat(m_VAO, m_attribPos, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(m_VAO, m_attribPos, m_vaoBindingPoint);

	int offset = 3 * sizeof(float);

	if (m_VP & VertexData::VP_NRM) {
		glEnableVertexArrayAttrib(m_VAO, m_attribNrm);
		glVertexArrayAttribFormat(m_VAO, m_attribNrm, 3, GL_FLOAT, GL_FALSE, offset);
		glVertexArrayAttribBinding(m_VAO, m_attribNrm, m_vaoBindingPoint);
		offset += 3 * sizeof(float);
	} else {
		glDisableVertexArrayAttrib(m_VAO, m_attribNrm);
		//m_attribNrm=-1; //TODO do this?
	}

	if (m_VP & VertexData::VP_COL) {
		glEnableVertexArrayAttrib(m_VAO, m_attribCol);
		glVertexArrayAttribFormat(m_VAO, m_attribCol, 3, GL_FLOAT, GL_FALSE, offset);
		glVertexArrayAttribBinding(m_VAO, m_attribCol, m_vaoBindingPoint);
		offset += 3 * sizeof(float);
	} else {
		glDisableVertexArrayAttrib(m_VAO, m_attribCol);
		//m_attribCol=-1; //TODO do this?, see glBindAttribLocation(glp.getID(),glMesh.getAttribCol(),"vCol");
		//TODO just use layout(location... instead
	}

	if (m_VP & VertexData::VP_UV) {
		glEnableVertexArrayAttrib(m_VAO, m_attribUV);
		glVertexArrayAttribFormat(m_VAO, m_attribUV, 2, GL_FLOAT, GL_FALSE, offset);
		glVertexArrayAttribBinding(m_VAO, m_attribUV, m_vaoBindingPoint);
		offset += 2 * sizeof(float);
	} else {
		glDisableVertexArrayAttrib(m_VAO, m_attribUV);
		//m_attribUV=-1; //TODO do this?,
	}

	if (m_VP & VertexData::VP_TAN) {
		glEnableVertexArrayAttrib(m_VAO, m_attribTan);
		glVertexArrayAttribFormat(m_VAO, m_attribTan, 4, GL_FLOAT, GL_FALSE, offset);
		glVertexArrayAttribBinding(m_VAO, m_attribTan, m_vaoBindingPoint);
	} else {
		glDisableVertexArrayAttrib(m_VAO, m_attribTan);
		//m_attribUV=-1; //TODO do this?,
	}

	//
	glBindVertexArray(m_VAO);
}

uint32_t GLVertexBuffer::getVAO() {
	return m_VAO;
}

int32_t GLVertexBuffer::getAttribPos() const {
	return m_attribPos;
}
int32_t GLVertexBuffer::getAttribNrm() const {
	return m_attribNrm;
}
int32_t GLVertexBuffer::getAttribCol() const {
	return m_attribCol;
}
int32_t GLVertexBuffer::getAttribUV() const {
	return m_attribUV;
}
int32_t GLVertexBuffer::getAttribTan() const {
	return m_attribTan;
}
