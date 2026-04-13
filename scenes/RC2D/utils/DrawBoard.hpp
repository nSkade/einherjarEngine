#pragma once

#include <suOGL.hpp>

namespace ehj {

struct DrawBoard {
	GLFrameBuffer m_fb1;
	GLFrameBuffer m_fb2;
	GLProgram glpPencil;
	GLProgram glpDO; // dynamic objects
	float pencilSize = .02;
	vec4 pencilColor = vec4(1.,0.,0.,1.);
	bool pencilVisible = true;
	
	struct Bl {
		vec2 p,d;
	};
	std::vector<Bl> bounceLightsC;

	DrawBoard(ivec2 res, const GLVertexBuffer& ssvb);

	void imgui();
	void updateBufferSize(ivec2 res);
	void setPencilUniforms(GLProgram& glp);

	/**
	 * draw temporary elements / dynamic objects
	 * @param ssm screen space glmesh
	 * @param dt deltaTime
	 */
	void drawPencil(GLMesh& ssm, ivec2 res, float dt, std::function<void(GLProgram& glp)> setCMNuniforms);

};

}//ehj
