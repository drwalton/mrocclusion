#ifndef MROCCLUSION_FULLSCREENQUAD_HPP_INCLUDED
#define MROCCLUSION_FULLSCREENQUAD_HPP_INCLUDED

#include "GLBuffer.hpp"
#include "ShaderProgram.hpp"
#include "VertexArrayObject.hpp"
#include <memory>

class Texture;

//!\brief Class encapsulating a quad, which can be rendered with an appropriate
//!       shader and bound texture to apply the texture to the framebuffer.
//!       Useful for e.g. displaying images.
class FullScreenQuad final
{
public:
	static FullScreenQuad &getInstance();
	static void showTexture(Texture *tex, ShaderProgram *prog);
	static void showTexture(GLuint tex, ShaderProgram *prog);
	void render(ShaderProgram &s);
	~FullScreenQuad() throw();

private:
	static std::unique_ptr<FullScreenQuad> instancePtr_;

	explicit FullScreenQuad();

	FullScreenQuad(const FullScreenQuad&);
	FullScreenQuad& operator=(const FullScreenQuad&);

	VertexArrayObject vao_;
	VertexBuffer verts_, tex_;
};

#endif
