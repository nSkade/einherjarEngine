#include "DrawBoard.hpp"
#include "suCMN.hpp"

namespace ehj {

DrawBoard::DrawBoard(ivec2 res, const GLVertexBuffer& ssvb) :
	m_fb1({res,GL_RGBA8,GL_LINEAR}),
	m_fb2({res,GL_RGBA8,GL_LINEAR})
{
	glpPencil.createPass(
		EHJ_THIS_FOLDER()+"ssq.vs",
		EHJ_THIS_FOLDER()+"pencil.fs"
	);
	
	glpDO.createPass(
		EHJ_THIS_FOLDER()+"ssq.vs",
		EHJ_THIS_FOLDER()+"dynamicObj.fs"
	);
}

void DrawBoard::imgui() {
	ImGui::Checkbox("show pencil",&pencilVisible);
	ImGui::DragFloat("pencilSize",&pencilSize,0.001,0.001,10.);
	ImGui::ColorEdit4("pencilColor",&pencilColor[0]);
	
	if(ImGui::Button("add bounce light")) {
		Bl b = {vec2(float(rand())/RAND_MAX,float(rand())/RAND_MAX),
				vec2(float(rand())/RAND_MAX,float(rand())/RAND_MAX)};
		bounceLightsC.push_back(b);
	}
	if(ImGui::Button("clear bounce lights")) {
		bounceLightsC.clear();
	}
}

void DrawBoard::updateBufferSize(ivec2 res) {
	GLFrameBuffer::Opt fbOpt {res,GL_RGBA8};
	m_fb1 = GLFrameBuffer(fbOpt);
	m_fb2 = GLFrameBuffer(fbOpt);
	//glClearColor(0.,0.,0.,0.);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void DrawBoard::setPencilUniforms(GLProgram& glp) {
	glUniform1f(glp.getUnfLoc("u_pencilSize"),pencilSize);
	glUniform4fv(glp.getUnfLoc("u_pencilColor"),1,&pencilColor[0]);
	glUniform1i(glp.getUnfLoc("u_pencilVisible"),pencilVisible);
}

void DrawBoard::drawPencil(GLMesh& ssm, ivec2 res, float dt, std::function<void(GLProgram& glp)> setCMNuniforms) { // draw pencil
	int width=res.x;
	int height=res.y;
	glBindFramebuffer(GL_FRAMEBUFFER,m_fb1.getFBO());
	glViewport(0, 0, res.x, res.y);
	glClear(GL_DEPTH_BUFFER_BIT);
	glpPencil.bind();
	setCMNuniforms(glpPencil);
	setPencilUniforms(glpPencil);
	ssm.bind();
	ssm.draw();
	//glDrawElements(GL_TRIANGLES,ssm.getEBOsize(),GL_UNSIGNED_INT,0);
	
	for (int i=0; i < bounceLightsC.size(); ++i) {
		auto& b = bounceLightsC[i];
		b.p += b.d * dt * .2f;
		if (b.p.x > float(width)/height || b.p.x < 0.) {
			b.p.x = clamp(b.p.x,0.f,float(width)/height);
			b.d.x *= -1;
		}
		if (b.p.y > 1. || b.p.y < 0.) {
			b.p.y = clamp(b.p.y,0.f,1.f);
			b.d.y *= -1;
		}
	}
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,m_fb1.getTexCol());
	glBindFramebuffer(GL_FRAMEBUFFER,m_fb2.getFBO());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glpDO.bind();
	setCMNuniforms(glpDO);
	setPencilUniforms(glpDO);

	std::vector<vec2> bounceLights;
	for (int i=0;i<bounceLightsC.size();++i)
		bounceLights.push_back(bounceLightsC[i].p);

	glUniform1i(glpDO.getUnfLoc("u_bounceLightCount"),bounceLights.size());
	glUniform2fv(glpDO.getUnfLoc("u_bounceLights"), bounceLights.size(), (float*) bounceLights.data());

	glDrawElements(GL_TRIANGLES,ssm.getEBOsize(),GL_UNSIGNED_INT,0);
}

}//ehj
