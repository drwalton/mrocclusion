#include "mrocclusion/RenderToTexture.hpp"
#include "mrocclusion/Texture.hpp"
#include "mrocclusion/Exception.hpp"


struct RenderToTexture::Impl {
	Impl(GLenum internalFormat,
		size_t width, size_t height,
		GLint border, GLenum format,
		GLenum type, bool useStencilBuffer)
		:texture(GL_TEXTURE_2D,
			internalFormat, width, height,
			border, format, type, nullptr, GL_LINEAR, GL_LINEAR),
		zBuffer(GL_TEXTURE_2D,
			GL_DEPTH_COMPONENT, width, height,
			border, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT)
	{ 
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture.tex(), 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, zBuffer.tex(), 0);

		if (useStencilBuffer) {
			stencilBuffer.reset(new Texture(GL_TEXTURE_2D, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE));
			glFramebufferTexture(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, stencilBuffer->tex(), 0);
		}

		GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawBuffers);

		int err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (err != GL_FRAMEBUFFER_COMPLETE) {
			glBindTexture(GL_TEXTURE_2D, texture.tex());
			GLint widthV;
			glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WIDTH, &widthV);
			throw GraphicsException(
				"Error creating framebuffer in RenderToTexture16! " +
				std::to_string(err));
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	Texture texture;
	Texture zBuffer;
	std::unique_ptr<Texture> stencilBuffer;
	GLuint framebuffer, renderbuffer;
};

RenderToTexture::RenderToTexture(
	GLenum internalFormat, 
	size_t width, size_t height, 
	GLint border, GLenum format, 
	GLenum type, bool useStencil)
	:pimpl_(new Impl(internalFormat, 
		width, height, border, format, type, useStencil))
{
}

RenderToTexture::~RenderToTexture() throw()
{
}

Texture & RenderToTexture::texture()
{
	return pimpl_->texture;
}

Texture & RenderToTexture::zBufferTexture()
{
	return pimpl_->zBuffer;
}

void RenderToTexture::setAsRenderTarget()
{
	glBindFramebuffer(GL_FRAMEBUFFER, pimpl_->framebuffer);
	glViewport(0, 0, pimpl_->texture.width(), pimpl_->texture.height());
}

void RenderToTexture::unsetAsRenderTarget()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

