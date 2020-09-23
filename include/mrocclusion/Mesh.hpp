#ifndef MROCCLUSION_MESH_HPP_INCLUDED
#define MROCCLUSION_MESH_HPP_INCLUDED

#include "mrocclusion/Renderable.hpp"
#include "mrocclusion/GLBuffer.hpp"
#include "mrocclusion/ModelLoader.hpp"
#include "mrocclusion/VertexArrayObject.hpp"
#include <GL/glew.h>
#include <memory>


//!\brief Class abstracting a renderable 3D triangle mesh.
class Mesh : public Renderable
{
public:
	Mesh(const mat4 &modelToWorld = mat4::Identity());
	~Mesh() throw();

	void vert (const std::vector<vec3> &verts, GLenum usage = GL_STATIC_DRAW);
	void norm (const std::vector<vec3> &norms, GLenum usage = GL_STATIC_DRAW);
	void tex  (const std::vector<vec2> &tex  , GLenum usage = GL_STATIC_DRAW);
	void color(const std::vector<vec3> &color, GLenum usage = GL_STATIC_DRAW);
	void elems(const std::vector<GLuint> &elems, GLenum usage = GL_STATIC_DRAW);

	void fromModelLoader(ModelLoader &l);

	virtual void render();

	void shaderProgram(ShaderProgram *p);
	ShaderProgram* shaderProgram() const;
private:
	Mesh(const Mesh&);
	Mesh& operator=(const Mesh&);
	std::unique_ptr<VertexBuffer> vert_, norm_, tex_, color_;
	std::unique_ptr<ElementBuffer> elem_;
	ShaderProgram *shaderProgram_;
	VertexArrayObject vao_;
	size_t nElems_, nVerts_;
};


#endif
