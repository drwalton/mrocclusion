#ifndef MROCCLUSION_MODEL_LOADER_HPP_INCLUDED
#define MROCCLUSION_MODEL_LOADER_HPP_INCLUDED

#include <GL/glew.h>
#include <vector>

#include <memory>
#include "VectorTypes.hpp"


//!\brief Class designed to abstract loading 3D models from a variety of file
//!      types.
class ModelLoader final
{
public:
	explicit ModelLoader();
	explicit ModelLoader(const std::string &filename);
	~ModelLoader() throw();

	//! \brief Load a file.
	//! \note Deletes any content already present in the ModelLoader object.
	void loadFile(const std::string &filename);

	//! \brief Save data in ModelLoader object to a file.
	void saveFile(const std::string &filename);

	//!\brief Move the centroid of the supplied mesh to the model-space origin.
	void center();

	//!\brief Apply uniform scaling to a mesh so that all its points lie within
	//! an axis-aligned cube centred at the origin.
	//!\param dim The length of a side of the cube.
	void scale(float dim = 2.0f);

	std::vector<vec3> &vertices();
	const std::vector<vec3> &vertices() const;
	std::vector<GLuint> &indices();
	const std::vector<GLuint> &indices() const;

	bool hasNormals() const;
	std::vector<vec3> &normals();
	const std::vector<vec3> &normals() const;

	bool hasTexCoords() const;
	std::vector<vec2> &texCoords();
	const std::vector<vec2> &texCoords() const;

	bool hasVertColors() const;
	std::vector<vec3> &vertColors();
	const std::vector<vec3> &vertColors() const;

	void loadPlyPointCloud(const std::string &filename);
	void savePlyPointCloud(const std::string &filename);

private:

	struct Impl;
	std::shared_ptr<Impl> pimpl_;
};


#endif
