#include "mrocclusion/Mesh.hpp"
#include "mrocclusion/Constants.hpp"


Mesh::Mesh(const mat4 &modelToWorld)
    :Renderable(modelToWorld),
    shaderProgram_(nullptr),
    nElems_(0), nVerts_(0)
{}

Mesh::~Mesh() throw()
{}

void Mesh::vert(const std::vector<vec3> &v, GLenum usage)
{
	vert_.reset(new VertexBuffer(v, usage));
	vao_.bind();
	vert_->bind();
	glEnableVertexAttribArray(UniformLocation::VERT);
	glVertexAttribPointer(UniformLocation::VERT, 3, GL_FLOAT, GL_FALSE, 0, 0);
	vert_->unbind();
	vao_.unbind();
	nVerts_ = v.size();
	throwOnGlError();
}
void Mesh::norm(const std::vector<vec3> &n, GLenum usage)
{
	norm_.reset(new VertexBuffer(n, usage));
	vao_.bind();
	norm_->bind();
	glEnableVertexAttribArray(UniformLocation::NORM);
	glVertexAttribPointer(UniformLocation::NORM, 3, GL_FLOAT, GL_FALSE, 0, 0);
	norm_->unbind();
	vao_.unbind();
	throwOnGlError();
}
void Mesh::tex(const std::vector<vec2> &n, GLenum usage)
{
	tex_.reset(new VertexBuffer(n, usage));
	vao_.bind();
	tex_->bind();
	glEnableVertexAttribArray(UniformLocation::TEX);
	glVertexAttribPointer(UniformLocation::TEX, 2, GL_FLOAT, GL_FALSE, 0, 0);
	tex_->unbind();
	vao_.unbind();
	throwOnGlError();
}
void Mesh::color(const std::vector<vec3> &n, GLenum usage)
{
	color_.reset(new VertexBuffer(n, usage));
	vao_.bind();
	color_->bind();
	glEnableVertexAttribArray(UniformLocation::COLOR);
	glVertexAttribPointer(UniformLocation::COLOR, 3, GL_FLOAT, GL_FALSE, 0, 0);
	color_->unbind();
	vao_.unbind();
	throwOnGlError();
}
void Mesh::elems(const std::vector<GLuint> &elems, GLenum usage)
{
	elem_.reset(new ElementBuffer(elems, usage));
	nElems_ = elems.size();
	vao_.bind();
	elem_->bind();
	vao_.unbind();
	throwOnGlError();
}

void Mesh::fromModelLoader(ModelLoader &m)
{
	vert(m.vertices());
	elems(m.indices());
	if (m.hasNormals()) {
		norm(m.normals());
	}
	if (m.hasTexCoords()) {
		tex(m.texCoords());
	}
	if (m.hasVertColors()) {
		color(m.vertColors());
	}
}

void Mesh::render()
{
	if(shaderProgram_ == nullptr) {
		throw GraphicsException("Attempted to render mesh without supplying shader program.");
	}
	vao_.bind();
	shaderProgram_->use();
	shaderProgram_->setUniform("modelToWorld", modelToWorld());
	shaderProgram_->setUniform("normToWorld", normToWorld());
	shaderProgram_->setupCameraBlock();
	//glPointSize(10.f);
	//glDrawArrays(GL_POINTS, 0, GLsizei(nVerts_));
	if(nElems_ != 0) {
    	glDrawElements(GL_TRIANGLES, GLsizei(nElems_), GL_UNSIGNED_INT, 0);
	} else {
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(nVerts_));
	}
	shaderProgram_->unuse();
	vao_.unbind();
}

ShaderProgram *Mesh::shaderProgram() const
{
	return shaderProgram_;
}

void Mesh::shaderProgram(ShaderProgram *p)
{
	shaderProgram_ = p;
}

