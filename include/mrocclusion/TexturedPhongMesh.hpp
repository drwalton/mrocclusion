#ifndef MROCCLUSION_TEXTUREDPHONGMESH_HPP_INCLUDED
#define MROCCLUSION_TEXTUREDPHONGMESH_HPP_INCLUDED

#include "mrocclusion/Renderable.hpp"
#include "mrocclusion/Mesh.hpp"
#include <memory>


class Texture;

struct Plane
{
	vec3 N;
	float d;
};

//!\brief Mesh with basic Phong illumination and albedo texture.
//!       Can be constructed easily using an image and a mesh file.
//!\note The current implementation does not account for distance
//!      from the light source in world space. Light position is used
//!      to determing incoming light direction only.
class TexturedPhongMesh : public Renderable {
public:
	TexturedPhongMesh(
		const std::string &textureFile,
		const std::string &meshFile);
	~TexturedPhongMesh() throw();

	//!\brief Ambient diffuse light level
	void ambientLight(float l);

	//!\brief Specular reflectance intensity
	void specularity(float s);

	//!\brief Diffuse reflectance intensity
	void diffuseReflectance(float r);

	//!\brief Color of specular highlights
	void specularColor(const vec3 &color);

	//!\brief Position of light (in world space).
	void lightWorldPos(const vec3 &p);
	const vec3 &lightWorldPos() const;

	virtual void render();

	void render(ShaderProgram *program);

	void drawShadow(bool);
	bool drawShadow() const;

	void shadowPlane(Plane plane);
	const Plane &shadowPlane() const;

	void shadowColor(const vec4&);
	vec4 shadowColor() const;

private:
	TexturedPhongMesh(const TexturedPhongMesh&);
	TexturedPhongMesh& operator=(const TexturedPhongMesh&);

	static std::unique_ptr<ShaderProgram> shader, shadowShader;
	static size_t nInstances;
	std::unique_ptr<Texture> albedoTexture_;
	Mesh mesh_;
	Plane shadowPlane_;
	float specularity_;
	float ambientLight_;
	float diffuseReflectance_;
	bool drawShadow_;
	vec4 shadowColor_;
	vec3 specularColor_, lightWorldPos_;
};


#endif

