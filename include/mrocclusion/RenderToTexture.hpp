#ifndef MROCCLUSION_RENDERTOTEXTURE_HPP_INCLUDED
#define MROCCLUSION_RENDERTOTEXTURE_HPP_INCLUDED

#include <GL/glew.h>
#include <memory>


class Texture;

//!\brief Convenience class for rendering to a 2D texture.
//!       Creates texture, depth texture and framebuffer.
class RenderToTexture final {
public:
	//!\brief Constructor. Similar to constructor for Texture.
	//!\note Cannot set target. This will always be set to GL_TEXTURE_2D.
	RenderToTexture(GLenum internalFormat,
		size_t width, size_t height,
		GLint border, GLenum format, GLenum type,
		bool useStencil = false);

	~RenderToTexture() throw();

	//!\brief Access render texture.
	Texture& texture();

	//!\brief Access texture used as depth (z) buffer. In 16-bit format.
	Texture& zBufferTexture();

	//!\brief Set as render target. Subsequent draw calls draw to the texture.
	void setAsRenderTarget();

	//!\brief Set default framebuffer as render target.
	void unsetAsRenderTarget();
private:
	struct Impl;
	std::unique_ptr<Impl> pimpl_;
};

#endif
