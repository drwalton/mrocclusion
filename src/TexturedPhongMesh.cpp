#include "mrocclusion/TexturedPhongMesh.hpp"
#include "mrocclusion/Directories.hpp"
#include "mrocclusion/ModelLoader.hpp"
#include "mrocclusion/Texture.hpp"
#include "mrocclusion/Exception.hpp"
#include <opencv2/opencv.hpp>


size_t TexturedPhongMesh::nInstances = 0;
std::unique_ptr<ShaderProgram> TexturedPhongMesh::shader, TexturedPhongMesh::shadowShader;

TexturedPhongMesh::TexturedPhongMesh(
	const std::string & textureFile, const std::string & meshFile)
	:ambientLight_(0.3f),
	specularity_(3.0f),
	diffuseReflectance_(.7f),
	specularColor_(.1f, .1f, .1f),
	lightWorldPos_(0.f, 1.f, 0.f),
	drawShadow_(false),
	shadowColor_(0.6f, 0.6f, 0.6f, 0.4f)
{
	if (nInstances == 0) {
		shader.reset(new ShaderProgram({
			MROCCLUSION_SHADER_DIR + "TexturedPhongMesh.vert",
			MROCCLUSION_SHADER_DIR + "TexturedPhongMesh.frag"
		}));
		shader->setUniform("tex", 0);
	}
	++nInstances;
	shadowPlane_.N = vec3(0.f, 1.f, 0.f);
	shadowPlane_.d = 0.f;

	ModelLoader loader(meshFile);
	mesh_.fromModelLoader(loader);

	cv::Mat albedoIm = cv::imread(textureFile);
	cv::Mat albedoImFlip;
	cv::flip(albedoIm, albedoImFlip, 0);

	if (albedoIm.rows == 0 || albedoIm.cols == 0) {
		throw GraphicsException("Unable to load texture from image file " + textureFile);
	}
	albedoTexture_.reset(new Texture(GL_TEXTURE_2D, GL_RGB8,
		albedoIm.cols, albedoIm.rows, 0,
		GL_BGR, GL_UNSIGNED_BYTE, albedoImFlip.data));
	mesh_.shaderProgram(shader.get());
}

TexturedPhongMesh::~TexturedPhongMesh() throw()
{
	--nInstances;
	if (nInstances == 0) {
		shader.reset(nullptr);
	}
}

void TexturedPhongMesh::ambientLight(float l)
{
	ambientLight_ = l;
}

void TexturedPhongMesh::specularity(float s)
{
	specularity_ = s;
}

void TexturedPhongMesh::diffuseReflectance(float r)
{
	diffuseReflectance_ = r;
}

void TexturedPhongMesh::specularColor(const vec3 & color)
{
	specularColor_ = color;
}

void TexturedPhongMesh::lightWorldPos(const vec3 & p)
{
	lightWorldPos_ = p;
}

const vec3 &TexturedPhongMesh::lightWorldPos() const
{
	return lightWorldPos_;
}

void TexturedPhongMesh::render()
{
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	shader->setUniform("ambientLight", ambientLight_);
	shader->setUniform("specularity", specularity_);
	shader->setUniform("diffuseReflectance", diffuseReflectance_);
	shader->setUniform("specularColor", specularColor_);
	shader->setUniform("lightWorldPos", lightWorldPos_);
	mesh_.modelToWorld(modelToWorld());
	albedoTexture_->bindToImageUnit(0);
	mesh_.render();
	if(drawShadow_) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_ZERO, GL_SRC_ALPHA);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_EQUAL, 0, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
		glStencilMask(0xFF);
		glClear(GL_STENCIL_BUFFER_BIT);
		glDisable(GL_CULL_FACE);
		mesh_.shaderProgram(shadowShader.get());
		shadowShader->setUniform("lightWorldPos", lightWorldPos_);
		shadowShader->setUniform("shadowColor", shadowColor_);
		shadowShader->setUniform("planeN", shadowPlane_.N);
		shadowShader->setUniform("planeD", shadowPlane_.d);
		mesh_.render();
		mesh_.shaderProgram(shader.get());
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
	}
}

void TexturedPhongMesh::render(ShaderProgram *program)
{
	mesh_.shaderProgram(program);
	albedoTexture_->bindToImageUnit(0);
	program->setUniform("ambientLight", ambientLight_);
	program->setUniform("specularity", specularity_);
	program->setUniform("diffuseReflectance", diffuseReflectance_);
	program->setUniform("specularColor", specularColor_);
	program->setUniform("lightWorldPos", lightWorldPos_);
	mesh_.modelToWorld(modelToWorld());
	mesh_.render();
	mesh_.shaderProgram(shader.get());
}

void TexturedPhongMesh::drawShadow(bool s)
{
	if(s) {
		if(!shadowShader) {
			shadowShader.reset(new ShaderProgram({
				MROCCLUSION_SHADER_DIR + "TexturedPhongMeshShadow.vert",
				MROCCLUSION_SHADER_DIR + "TexturedPhongMeshShadow.frag"
			}));
		}
	}
	drawShadow_ = s;
}

bool TexturedPhongMesh::drawShadow() const
{
	return drawShadow_;
}

void TexturedPhongMesh::shadowPlane(Plane plane)
{
	shadowPlane_ = plane;
}

const Plane &TexturedPhongMesh::shadowPlane() const
{
	return shadowPlane_;
}

void TexturedPhongMesh::shadowColor(const vec4 &c)
{
	shadowColor_ = c;
}

vec4 TexturedPhongMesh::shadowColor() const
{
	return shadowColor_;
}


