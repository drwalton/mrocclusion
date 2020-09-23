#ifndef MROCCLUSION_VECTORTYPES_HPP_INCLUDED
#define MROCCLUSION_VECTORTYPES_HPP_INCLUDED

#include <Eigen/Core>
#include <vector>
#include <GL/glew.h>

typedef Eigen::Vector2f vec2;
typedef Eigen::Vector3f vec3;
typedef Eigen::Vector4f vec4;
typedef Eigen::Matrix<float, 5, 1> vec5;
typedef Eigen::Matrix<float, 6, 1> vec6;
typedef Eigen::Matrix<float, 7, 1> vec7;
typedef Eigen::Matrix<float, 8, 1> vec8;

typedef Eigen::Vector2d dvec2;
typedef Eigen::Vector3d dvec3;
typedef Eigen::Vector4d dvec4;

typedef Eigen::Matrix2f mat2;
typedef Eigen::Matrix3f mat3;
typedef Eigen::Matrix4f mat4;
typedef Eigen::Matrix<float, 5, 5> mat5;
typedef Eigen::Matrix<float, 6, 6> mat6;

typedef Eigen::Matrix<GLbyte, 2, 1> vec2b;
typedef Eigen::Matrix<GLbyte, 3, 1> vec3b;
typedef Eigen::Matrix<GLbyte, 4, 1> vec4b;

typedef Eigen::Matrix<GLuint, 2, 1> uvec2;
typedef Eigen::Matrix<GLuint, 3, 1> uvec3;
typedef Eigen::Matrix<GLuint, 4, 1> uvec4;

typedef Eigen::Matrix<GLint, 2, 1> ivec2;
typedef Eigen::Matrix<GLint, 3, 1> ivec3;
typedef Eigen::Matrix<GLint, 4, 1> ivec4;

//!\brief Class encapsulating a rigid transform (rotation & translation).
class RigidTransform {
public:
	RigidTransform();

	RigidTransform(const mat3 &r, const vec3 &t);

	RigidTransform(const mat4 &m);

	RigidTransform inverse() const;

	RigidTransform &operator=(const mat4 &m);

	mat3 rotation;
	vec3 translation;
	vec3 operator *(const vec3 &rhs) const;
	operator mat4() const;
	vec4 operator *(const vec4 &rhs) const;

	static RigidTransform loadFromFile(const std::string &filename);
};

typedef mat4 WorldToCamTransform;

struct AACuboid
{
	float minX, maxX, minY, maxY, minZ, maxZ;
};

struct AARect
{
	float minX, maxX, minY, maxY;
};

struct Viewport
{
	Viewport(int x, int y, int w, int h)
		:x(x), y(y), w(w), h(h)
	{}
	Viewport(){}
	void set() const
	{
		glViewport(x, y, w, h);
	}
	int x, y, w, h;
};

struct TriangleMeshIndices
{
	std::vector<vec3> vertices;
	std::vector<GLuint> indices;
	void applyTransformInPlace(const mat4 &transform);
	void append(const TriangleMeshIndices &other);
};

//!\brief Fit a viewport within supplied limits, maintaining a given aspect ratio.
//!\note For example, given a 320x240 viewport and a ratio of 1, will return a
//!      centred 240x240 viewport.
//!\note Useful for showing e.g. an image without stretching, distortion.
//!\param v The larger viewport to fit the subviewport into.
//!\param aspect The desired aspect ratio (given as width/height).
Viewport fitViewportWithAspectRatio(Viewport v, float aspect);

//!\brief Reflect function. Identical behaviour to the one from GLSL.
vec3 reflect(vec3 dir, vec3 norm);

#endif
