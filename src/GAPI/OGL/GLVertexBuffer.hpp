#include "Mesh.hpp"

#include <glad/glad.h>

class GLVertexBuffer {
public:
	GLVertexBuffer(const ehj::VertexData& vertexData) : GLVertexBuffer(vertexData, GL_DYNAMIC_STORAGE_BIT) {};
	GLVertexBuffer(const ehj::VertexData& vertexData, GLbitfield flags);
	~GLVertexBuffer();

	void bind(uint32_t bindingIndex);
	
	uint32_t getVAO();
	int32_t getAttribPos() const;
	int32_t getAttribNrm() const;
	int32_t getAttribCol() const;
	int32_t getAttribUV()  const;
	int32_t getAttribTan() const;
private:
	uint32_t m_VAO; // vertex array object
	uint32_t m_VBO; // vertex buffer object
	
	uint32_t m_VP = 0; // mesh properties
	uint32_t m_Dim = 3;
	
	//TODO abstract this
	GLuint m_attribPos = 0;
	GLuint m_attribNrm = 1;
	GLuint m_attribCol = 2;
	GLuint m_attribUV  = 3;
	GLuint m_attribTan = 4;

	GLuint m_vaoBindingPoint = 0; //TODO manage
};
