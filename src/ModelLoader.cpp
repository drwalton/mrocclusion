#include "mrocclusion/ModelLoader.hpp"
#include "mrocclusion/KahanVal.hpp"
#include "mrocclusion/Exception.hpp"
#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <fstream>

#include <string>
#include <iostream>

using std::vector;
void savePointCloudToPly(const std::string &filename,
	const std::vector<vec3> &positions,
	bool binary);

enum FileType {
	PLY,
	OBJ
};

struct ModelLoader::Impl
{
	vector<vec3> verts;
	vector<GLuint> indices;
	vector<vec3> normals;
	vector<vec2> texCoords;
	vector<vec3> vertColors;

	void loadFile(const std::string &filename);
	void saveFile(const std::string &filename);
	void saveFilePly(const std::string &filename);
	FileType getFileType(const std::string &filename);
	void loadFileAssimp(const std::string &filename);
	void saveFileAssimp(const std::string &filename);
	void loadPointCloudFromPly(const std::string &filename);
	void savePointCloudToPly(const std::string &filename) {
		::savePointCloudToPly(filename, verts, false);
	}
	void centerMesh();
};
void saveCloudToPlyFile(const std::string &filename,
	const std::vector<vec3> &positions,
	const std::vector<vec3> &colors,
	const std::vector<vec3> &normals,
	const std::vector<float> &radii,
	bool binary);
void saveMeshToPlyFile(const std::string &filename,
	const std::vector<vec3> &positions,
	const std::vector<vec3> &colors,
	const std::vector<vec3> &normals,
	const std::vector<float> &radii,
	const std::vector<GLuint> &indices,
	bool binary);

void ModelLoader::Impl::loadFile(const std::string &filename)
{
	FileType type = getFileType(filename);

	switch (type)
	{
//	case PLY:
//		loadPointCloudFromPly(filename);
//		break;
	default:
		loadFileAssimp(filename);
		break;
	}
}

void ModelLoader::Impl::saveFile(const std::string &filename)
{
	FileType type = getFileType(filename);

	std::cout << "Saving model file: " << filename <<
		"\nVertices: " << verts.size() << " Normals: " << normals.size()
		<< " Tex Coords: " << texCoords.size() << std::endl;

	switch (type)
	{
	default:
		saveFilePly(filename);
		//saveFileAssimp(filename);
		break;
	}
}

void ModelLoader::Impl::saveFilePly(const std::string &filename)
{
	std::vector<float> radii;
	if (indices.size() == 0) {
		indices.resize(verts.size());
		for (size_t i = 0; i < verts.size(); ++i) {
			indices[i] = (GLuint)(i);
		}
		saveMeshToPlyFile(filename, verts, vertColors, normals, radii, indices, false);
	}
	else {
		saveMeshToPlyFile(filename, verts, vertColors, normals, radii, indices, false);
	}
}


void ModelLoader::Impl::loadPointCloudFromPly(const std::string &filename)
{
	verts = std::vector<vec3>();
	std::string line = "";
	bool binary = false;
	size_t nPoints = 0;
	char linebuf[100];
	{
    	std::ifstream istream(filename);
    	while(istream.good()) {
    		istream.getline(linebuf, 100);
    		if(strstr(linebuf, "end_header") != nullptr) {
    			break;
    		} else if(strstr(linebuf, "format") != nullptr) {
    			if(strstr(linebuf, "binary") != nullptr) {
    				binary = true;
    			}
    		} else if(strstr(linebuf, "element") != nullptr) {
				char *p = linebuf;
				while(!isdigit(*p++));
				nPoints = atoi(--p);
			}
    	}
	}
	std::ios::openmode flags;
	if (binary) flags = std::ios::binary;
	std::ifstream file(filename, flags);
	if (file.fail()) throw GraphicsException("Could not open file \"" +
		filename + "\" to write PLY file.");
	
	//Advance to end of header
	memset(linebuf, 0, 100);
	while(strstr(linebuf, "end_header") == nullptr) {
		file.getline(linebuf, 100);
	}

	verts.resize(nPoints);
	for(size_t i = 0; i < nPoints; ++i){
		if(file.bad()) {
			throw GraphicsException("BAD PLY FILE");
		}
		vec3 pt;
		file >> pt.x() >> pt.y() >> pt.z();
		verts[i] = pt;
	}
}

void savePointCloudToPly(const std::string &filename,
	const std::vector<vec3> &positions,
	bool binary)
{

	std::ios::openmode flags = std::ios::trunc;
	if (binary) flags = std::ios::binary | std::ios::trunc;
	std::ofstream file(filename, flags);
	if (file.fail()) throw GraphicsException("Could not open file \"" +
		filename + "\" to write PLY file.");

	//Write header.
	file << "ply\n";
	if (binary) {
		file << "format binary_little_endian 1.0\n";
	}
	else {
		file << "format ascii 1.0\n";
	}
	file
		<< "comment author: David R. Walton\n"
		<< "comment object: Saved Reconstruction\n"
		<< "element vertex " << positions.size() << "\n"
		<< "property float x\n"
		<< "property float y\n"
		<< "property float z\n"
		<< "end_header\n";

	if (binary) {
		//Write content.
		for (size_t i = 0; i < positions.size(); ++i) {
			file.write((const char*)(&(positions[i].x())), sizeof(float))
				.write((const char*)(&(positions[i].y())), sizeof(float))
				.write((const char*)(&(positions[i].z())), sizeof(float));
		}
	}
	else {
		file << std::fixed;

		//Write content.
		for (size_t i = 0; i < positions.size(); ++i) {
			file
				<< positions[i].x() << " "
				<< positions[i].y() << " "
				<< positions[i].z() << "\n";
		}

		file << "\n";
	}
}


FileType ModelLoader::Impl::getFileType(const std::string &filename)
{
	std::string extension = filename.substr(filename.find_last_of(".") + 1);

	if (extension == "ply")
	{
		return PLY;
	}
	else if (extension == "obj")
	{
		return OBJ;
	}
	else
	{
		throw GraphicsException(("Attempted to load mesh from file \"" + filename +
			"\" which has unknown file extension \"" + extension + "\"").c_str());
	}
}


void ModelLoader::Impl::loadFileAssimp(const std::string &filename)
{
	Assimp::Importer importer;
	importer.ReadFile(filename.c_str(),
		aiProcess_GenSmoothNormals |
		aiProcess_Triangulate |
		aiProcess_FlipWindingOrder);

	const aiScene *scene = importer.GetScene();

	if (scene == nullptr)
	{
		throw GraphicsException(("File \"" + filename +
			"\" could not be loaded").c_str());
	}

	if (scene->mNumMeshes == 0)
	{
		throw GraphicsException(("File \"" + filename + "\" loaded, but does not "
			"appear to contain any meshes").c_str());
	}

	const aiMesh *mesh = scene->mMeshes[0];

	size_t maxIndex = 0;

	//NOTE: Here we assume all faces are triangles.
	size_t nIndices = mesh->mNumFaces * 3;

	indices.resize(nIndices);
	for (size_t f = 0; f < mesh->mNumFaces; ++f)
	{
		indices[f * 3 + 0] = mesh->mFaces[f].mIndices[0];
		indices[f * 3 + 1] = mesh->mFaces[f].mIndices[2];
		indices[f * 3 + 2] = mesh->mFaces[f].mIndices[1];
	}

	for (size_t i = 0; i < mesh->mNumFaces * 3; ++i)
	{
		if (indices[i] > maxIndex) maxIndex = indices[i];
	}

	if (mesh->HasPositions())
	{
		verts.resize(maxIndex + 1);
		std::copy(mesh->mVertices,
			mesh->mVertices + maxIndex + 1, (aiVector3D*)verts.data());
	}

	if (mesh->HasNormals())
	{

		normals.resize(maxIndex + 1);
		std::copy(mesh->mNormals,
			mesh->mNormals + maxIndex + 1, (aiVector3D*)normals.data());
	}

	if (mesh->HasTextureCoords(0))
	{
		texCoords.resize(maxIndex+1);
		for (size_t i = 0; i < maxIndex + 1; ++i)
		{
			texCoords[i](0) = mesh->mTextureCoords[0][i].x;
			texCoords[i](1) = mesh->mTextureCoords[0][i].y;
		}
	}
	if (mesh->HasVertexColors(0)) {
		vertColors.resize(maxIndex + 1);
		for (size_t i = 0; i < maxIndex + 1; ++i) {
			vertColors[i](0) = mesh->mColors[0][i].r;
			vertColors[i](1) = mesh->mColors[0][i].g;
			vertColors[i](2) = mesh->mColors[0][i].b;

		}
	}
}

/*
void ModelLoader::Impl::saveFileAssimp(const std::string &filename)
{
	Assimp::Exporter exporter;

	aiScene scene;

	scene.mMeshes[0] = new aiMesh;
	scene.mNumMeshes = 1;
	aiMesh *mesh = scene.mMeshes[0];

	mesh->mVertices = reinterpret_cast<aiVector3D*>(verts.data());
	mesh->mNumVertices = verts.size();

	if (indices.size() > 0)
	{
		mesh->mNumFaces = (indices.size() / 3);
		mesh->mFaces = new aiFace[indices.size() / 3];
		for (size_t f = 0; f < mesh->mNumFaces; ++f)
		{
			aiFace &face = mesh->mFaces[f];
			face.mNumIndices = 3;
			face.mIndices = new unsigned int[3];
			for (size_t i = 1; i < 3; ++i)
			{
				face.mIndices[i] = indices[3 * f + i];
			}
		}
	}

	if (normals.size() == verts.size())
	{
		mesh->mNormals = reinterpret_cast<aiVector3D*>(verts.data());
	}

	if (vertColors.size() == verts.size())
	{
		//TODO implement vertex colors
		std::cout << "Warning: not saving vertex colors!" << std::endl;
	}

	if (texCoords.size() == verts.size())
	{
		mesh->mNumUVComponents[0] = 2;
		mesh->mTextureCoords[0] = new aiVector3D[verts.size()];

		for (size_t i = 0; i < verts.size(); ++i)
		{
			aiVector3D &t = mesh->mTextureCoords[0][i];
			t.x = texCoords[i](0);
			t.y = texCoords[i](1);
			t.z = 0.0f;
		}
	}

	exporter.Export(&scene,
		filename.substr(filename.find_last_of(".") + 1), filename);
}
*/

ModelLoader::ModelLoader()
:pimpl_(new Impl)
{}

ModelLoader::ModelLoader(const std::string &filename)
: pimpl_(new Impl)
{
	loadFile(filename);
}

ModelLoader::~ModelLoader() throw()
{
}

void ModelLoader::loadFile(const std::string & filename)
{
	pimpl_->loadFile(filename);
}

void ModelLoader::saveFile(const std::string &filename)
{
	pimpl_->saveFile(filename);
}

void ModelLoader::center()
{
	if (pimpl_->verts.size() == 0)
	{
		throw GraphicsException("ModelLoader::center() called on empty mesh!");
	}

	KahanVal<vec3> centroidSum(vec3::Zero());

	for (auto &vert : pimpl_->verts)
	{
		centroidSum += vert;
	}

	vec3 centroid = centroidSum.get() /
		static_cast<float>(pimpl_->verts.size());

	for (auto &vert : pimpl_->verts)
	{
		vert -= centroid;
	}
}

void ModelLoader::scale(float dim)
{
	//float dist = dim * 0.5f;
	//TODO
	throw std::runtime_error("ModelLoader::scale NOT IMPLEMENTED");
}

std::vector<vec3> &ModelLoader::vertices()
{
	return pimpl_->verts;
}

const std::vector<vec3> &ModelLoader::vertices() const
{
	return pimpl_->verts;
}

std::vector<GLuint> &ModelLoader::indices()
{
	return pimpl_->indices;
}

const std::vector<GLuint> &ModelLoader::indices() const
{
	return pimpl_->indices;
}

bool ModelLoader::hasNormals() const
{
	return pimpl_->normals.size() == pimpl_->verts.size();
}

std::vector<vec3> &ModelLoader::normals()
{
	return pimpl_->normals;
}

const std::vector<vec3> &ModelLoader::normals() const
{
	return pimpl_->normals;
}

bool ModelLoader::hasTexCoords() const
{
	return pimpl_->texCoords.size() == pimpl_->verts.size();
}

std::vector<vec2> &ModelLoader::texCoords()
{
	return pimpl_->texCoords;
}

const std::vector<vec2> &ModelLoader::texCoords() const
{
	return pimpl_->texCoords;
}

bool ModelLoader::hasVertColors() const
{
	return pimpl_->vertColors.size() == pimpl_->verts.size();
}

std::vector<vec3> &ModelLoader::vertColors()
{
	return pimpl_->vertColors;
}

const std::vector<vec3> &ModelLoader::vertColors() const
{
	return pimpl_->vertColors;
}

void ModelLoader::loadPlyPointCloud(const std::string &filename)
{
	pimpl_->loadPointCloudFromPly(filename);
}
void ModelLoader::savePlyPointCloud(const std::string &filename)
{
	pimpl_->savePointCloudToPly(filename);
}

void saveCloudToPlyFile(const std::string &filename,
	const std::vector<vec3> &positions,
	const std::vector<vec3> &colors,
	const std::vector<vec3> &normals,
	const std::vector<float> &radii,
	bool binary)
{

	std::ios::openmode flags = std::ios::trunc;
	if (binary) flags = std::ios::binary | std::ios::trunc;
	std::ofstream file(filename, flags);
	if (file.fail()) throw std::runtime_error("Could not open file \"" +
		filename + "\" to write PLY file.");

	//Write header.
	file << "ply\n";
	if (binary) {
		file << "format binary_little_endian 1.0\n";
	}
	else {
		file << "format ascii 1.0\n";
	}
	file
		<< "comment author: David R. Walton\n"
		<< "comment object: Saved Reconstruction\n"
		<< "element vertex " << positions.size() << "\n"
		<< "property float x\n"
		<< "property float y\n"
		<< "property float z\n";

	if (colors.size() == positions.size()) {
		file
			<< "property uchar red\n"
			<< "property uchar green\n"
			<< "property uchar blue\n";
	}
	if (normals.size() == positions.size()) {
		file
			<< "property float nx\n"
			<< "property float ny\n"
			<< "property float nz\n";
	}
	if (radii.size() == positions.size()) {
		file
			<< "property float radius\n";
	}
	file
		<< "end_header\n";

	if (binary) {
		//Write content.
		for (size_t i = 0; i < positions.size(); ++i) {
			file.write((const char*)(&(positions[i].x())), sizeof(float))
				.write((const char*)(&(positions[i].y())), sizeof(float))
				.write((const char*)(&(positions[i].z())), sizeof(float));
			if (colors.size() == positions.size()) {
				vec3b c((unsigned char)(colors[i].x()),
				(unsigned char)(colors[i].y()),
				(unsigned char)(colors[i].z()));
				file.write((const char*)(&(c[0])), sizeof(char))
					.write((const char*)(&(c[1])), sizeof(char))
					.write((const char*)(&(c[2])), sizeof(char));
			}
			if (normals.size() == positions.size()) {
				file.write((const char*)(&(normals[i].x())), sizeof(float))
					.write((const char*)(&(normals[i].y())), sizeof(float))
					.write((const char*)(&(normals[i].z())), sizeof(float));
			}
			if (radii.size() == positions.size()) {
				file.write((const char*)(&(radii[i])), sizeof(float));
			}
		}
	}
	else {
		file << std::fixed;

		//Write content.
		for (size_t i = 0; i < positions.size(); ++i) {
			file
				<< positions[i].x() << " "
				<< positions[i].y() << " "
				<< positions[i].z() << " ";
			if (colors.size() == positions.size()) {
				file
					<< (unsigned int)(colors[i][0]) << " "
					<< (unsigned int)(colors[i][1]) << " "
					<< (unsigned int)(colors[i][2]) << " ";
			}
			if (normals.size() == positions.size()) {
				file
					<< normals[i].x() << " "
					<< normals[i].y() << " "
					<< normals[i].z() << " ";
			}
			if (radii.size() == positions.size()) {
				file
					<< radii[i];
			}
			file << "\n";

		}

		file << "\n";
	}
}

void saveMeshToPlyFile(const std::string &filename,
	const std::vector<vec3> &positions,
	const std::vector<vec3> &colors,
	const std::vector<vec3> &normals,
	const std::vector<float> &radii,
	const std::vector<GLuint> &indices,
	bool binary)
{

	std::ios::openmode flags = std::ios::trunc;
	if (binary) flags = std::ios::binary | std::ios::trunc;
	std::ofstream file(filename, flags);
	if (file.fail()) throw std::runtime_error("Could not open file \"" +
		filename + "\" to write PLY file.");

	//Write header.
	file << "ply\n";
	if (binary) {
		file << "format binary_little_endian 1.0\n";
	}
	else {
		file << "format ascii 1.0\n";
	}
	file
		<< "comment author: David R. Walton\n"
		<< "comment object: Saved Reconstruction\n"
		<< "element vertex " << positions.size() << "\n"
		<< "property float x\n"
		<< "property float y\n"
		<< "property float z\n";

	if (colors.size() == positions.size()) {
		file
			<< "property uchar red\n"
			<< "property uchar green\n"
			<< "property uchar blue\n";
	}
	if (normals.size() == positions.size()) {
		file
			<< "property float nx\n"
			<< "property float ny\n"
			<< "property float nz\n";
	}
	if (radii.size() == positions.size()) {
		file
			<< "property float radius\n";
	}
	file << "element face " << indices.size() / 3;
	file << "\nproperty list uchar uint vertex_indices\n";
	file << "end_header\n";

	if (binary) {
		//Write content.
		for (size_t i = 0; i < positions.size(); ++i) {
			file.write((const char*)(&(positions[i].x())), sizeof(float))
				.write((const char*)(&(positions[i].y())), sizeof(float))
				.write((const char*)(&(positions[i].z())), sizeof(float));
			if (colors.size() == positions.size()) {
				vec3b c((unsigned char)(colors[i].x()),
					(unsigned char)(colors[i].y()),
					(unsigned char)(colors[i].z()));
				file.write((const char*)(&(c[0])), sizeof(char))
					.write((const char*)(&(c[1])), sizeof(char))
					.write((const char*)(&(c[2])), sizeof(char));
			}
			if (normals.size() == positions.size()) {
				file.write((const char*)(&(normals[i].x())), sizeof(float))
					.write((const char*)(&(normals[i].y())), sizeof(float))
					.write((const char*)(&(normals[i].z())), sizeof(float));
			}
			if (radii.size() == positions.size()) {
				file.write((const char*)(&(radii[i])), sizeof(float));
			}
		}
		for (size_t i = 0; i < indices.size(); i++) {
			static const unsigned char three = 3;
			file.write((const char*)(&(three)), sizeof(unsigned char));
			file.write((const char*)(&(indices[i])), sizeof(GLuint));
		}
	}
	else {
		file << std::fixed;

		//Write content.
		for (size_t i = 0; i < positions.size(); ++i) {
			file
				<< positions[i].x() << " "
				<< positions[i].y() << " "
				<< positions[i].z() << " ";
			if (colors.size() == positions.size()) {
				file
					<< (unsigned int)(colors[i][0]) << " "
					<< (unsigned int)(colors[i][1]) << " "
					<< (unsigned int)(colors[i][2]) << " ";
			}
			if (normals.size() == positions.size()) {
				file
					<< normals[i].x() << " "
					<< normals[i].y() << " "
					<< normals[i].z() << " ";
			}
			if (radii.size() == positions.size()) {
				file
					<< radii[i];
			}
			file << "\n";

		}

		for (size_t i = 0; i < indices.size(); i += 3) {
			file << "3 "
				<< indices[i] << " "
				<< indices[i + 1] << " "
				<< indices[i + 2] << "\n";
		}

		file << "\n";
	}
}

