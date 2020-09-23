#include "mrocclusion/FullScreenQuad.hpp"
#include "mrocclusion/Constants.hpp"
#include "mrocclusion/Texture.hpp"

std::unique_ptr<FullScreenQuad> FullScreenQuad::instancePtr_(nullptr);

const std::vector<vec2> VERTS{ vec2(-1.f, 1.f), vec2(-1.f, -1.f), vec2(1.f, 1.f), vec2(1.f, 1.f), vec2(-1.f, -1.f), vec2(1.f, -1.f) };
const std::vector<vec2> TEXS{ vec2(0.f, 1.f), vec2(0.f, 0.f), vec2(1.f, 1.f), vec2(1.f, 1.f), vec2(0.f, 0.f), vec2(1.f, 0.f) };

FullScreenQuad &FullScreenQuad::getInstance()
{
	if (instancePtr_.get() == nullptr) {
		instancePtr_.reset(new FullScreenQuad);
	}
	return *instancePtr_;
}

void FullScreenQuad::showTexture(Texture *tex, ShaderProgram *prog)
{
	prog->setUniform("tex", 0);
	tex->bindToImageUnit(0);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	FullScreenQuad::getInstance().render(*prog);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	tex->unbind();
}

void FullScreenQuad::showTexture(GLuint tex, ShaderProgram *prog)
{
	prog->setUniform("tex", 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	FullScreenQuad::getInstance().render(*prog);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

FullScreenQuad::FullScreenQuad()
:verts_(VERTS), tex_(TEXS)
{
	vao_.bind();
	verts_.bind();
	glEnableVertexAttribArray(UniformLocation::VERT);
	glVertexAttribPointer(UniformLocation::VERT, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	tex_.bind();
	glEnableVertexAttribArray(UniformLocation::TEX);
	glVertexAttribPointer(UniformLocation::TEX, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	tex_.unbind();
	vao_.unbind();
	throwOnGlError();
}

FullScreenQuad::~FullScreenQuad() throw()
{}

void FullScreenQuad::render(ShaderProgram &s)
{
	s.use();
	vao_.bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	s.unuse();
}
