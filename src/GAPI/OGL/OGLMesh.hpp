#include <stdint.h>
#include "../../Mesh.hpp"
#include <glad/glad.h>

class OGLMesh {
public:
	/**
	 * @param usage - either GL_STATIC_DRAW, GL_DYNAMIC_DRAW, GL_STREAM_DRAW
	*/
	OGLMesh(ehj::Mesh mesh, GLenum usage);
	~OGLMesh();

	uint32_t getVAO();
	uint32_t getEBO();
	uint32_t getEBOsize();

	int32_t getAttribPos();
	int32_t getAttribNrm();
	int32_t getAttribCol();

private:
	uint32_t m_VAO; // vertex array object
	uint32_t m_VBO; // vertex buffer object
	uint32_t m_EBO; // element buffer object
	
	uint32_t m_EBOsize;

	GLuint m_attribPos = 0;
	GLuint m_attribNrm = 1;
	GLuint m_attribCol = 2;

	GLuint m_vaoBindingPoint;
};
