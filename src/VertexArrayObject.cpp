#include "mrocclusion/VertexArrayObject.hpp"

VertexArrayObject::VertexArrayObject()
{
	glGenVertexArrays(1, &vao_);
}
VertexArrayObject::~VertexArrayObject() throw()
{
	glDeleteVertexArrays(1, &vao_);
}
void VertexArrayObject::bind()
{
	glBindVertexArray(vao_);
}

void VertexArrayObject::unbind()
{
	glBindVertexArray(0);
}

