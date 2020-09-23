#ifndef MROCCLUSION_VERTEXARRAYOBJECT_HPP_INCLUDED
#define MROCCLUSION_VERTEXARRAYOBJECT_HPP_INCLUDED

#include <GL/glew.h>

class VertexArrayObject final
{
public:
	VertexArrayObject();
	~VertexArrayObject() throw();

	void bind();
	void unbind();
private:
	GLuint vao_;
};

#endif
