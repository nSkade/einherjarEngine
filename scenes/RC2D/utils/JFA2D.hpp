#pragma once

#include "Input/GLFW/OGL/GLFWWindowGL.hpp"
#include "suCMN.hpp"
#include <suOGL.hpp>

namespace ehj {

using namespace glm;

/**
 * @brief Jump Flood Algorithm 2d
 */
struct JFA2D {
private:
	int m_jfPassCountOrig;
public:
	GLFrameBuffer m_fb1;
	GLFrameBuffer m_fb2;
	GLProgram m_glp;
	int m_jfPassCount = 11;
	ivec2 m_res;

	int getMaxJfPassCount() { return m_jfPassCountOrig; }

	/**
	 * @param ssvb vertex buffer of screen space mesh
	 */
	JFA2D(ivec2 res, const GLVertexBuffer& ssvb) :
		m_fb1(res),
		m_fb2(res)
	{
		m_glp.createPass(
			EHJ_THIS_FOLDER()+"ssq.vs",
			EHJ_THIS_FOLDER()+"jumpflood.fs"
		);
		
		updateBufferSize(res);
		m_jfPassCount=m_jfPassCountOrig;
	}

	void updateBufferSize(ivec2 res) {
		m_res=res;
		GLFrameBuffer::Opt opt = {m_res,GL_RG16_SNORM,GL_LINEAR};
		m_fb1 = GLFrameBuffer(opt);
		m_fb2 = GLFrameBuffer(opt);
		m_jfPassCountOrig = ceil(glm::log2((float) fmax(res.x,res.y)))+1;
	}

	/**
	 * @param glMesh ssq
	 * @param fb flatland scene
	 */
	void passJFA(GLMesh& glMesh, GLFrameBuffer& fb, mat4 pvm) {
		// flood fill pass, requires for loop passes in order to cover whole screen
		//if (frame%2==0)
		{ // JFA
			//TODO lower res super good but has flickering see JFA channel
			//glViewport(0, 0, width, height);
			glViewport(0, 0, m_res.x, m_res.y);
			glBindFramebuffer(GL_FRAMEBUFFER,m_fb2.getFBO());
			glBindTexture(GL_TEXTURE_2D, m_fb2.getTexCol());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glBindFramebuffer(GL_FRAMEBUFFER,m_fb1.getFBO());
			
			glBindTexture(GL_TEXTURE_2D, m_fb1.getTexCol());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			m_glp.bind();

			glUniformMatrix4fv(m_glp.getUnfLoc("u_pvm"), 1, GL_FALSE, &pvm[0][0]);
			glUniform2iv(m_glp.getUnfLoc("u_resolution"), 1, &m_res[0]);

			// already bound glBindFramebuffer(GL_FRAMEBUFFER,fbJumpFlood.getFBO());
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,fb.getTexCol());
			glUniform1i(glGetUniformLocation(m_glp.getID(), "u_tex"), 0);           // texture unit 0
			// render uv
			glUniform1f(m_glp.getUnfLoc("u_jfOffset"),0.);
			
			glDrawElements(GL_TRIANGLES,glMesh.getEBOsize(),GL_UNSIGNED_INT,0);
			for (int i=0;i< m_jfPassCount;++i) {
				auto* fbJFfrom = &m_fb1;
				auto* fbJFto = &m_fb2;
				if (i%2==1)
					std::swap(fbJFfrom,fbJFto);
				
				glBindFramebuffer(GL_FRAMEBUFFER,fbJFto->getFBO());
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //TODO this clear not needed?
				
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D,fbJFfrom->getTexCol());
				glUniform1i(glGetUniformLocation(m_glp.getID(), "u_texJumpFlood"), 1);  // texture unit 1
				glUniform1f(m_glp.getUnfLoc("u_jfOffset"),pow(2,m_jfPassCount-i-1));

				glDrawElements(GL_TRIANGLES,glMesh.getEBOsize(),GL_UNSIGNED_INT,0);
			}

			if (m_jfPassCount %2==1) {
				// blit into fbJFto
				glBlitNamedFramebuffer(m_fb2.getFBO(),m_fb1.getFBO(),
					0,0,m_res.x,m_res.y,0,0,m_res.x,m_res.y,
					GL_COLOR_BUFFER_BIT,GL_NEAREST);
			}
		}
	}

};

}//ehj
