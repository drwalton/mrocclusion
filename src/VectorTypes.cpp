#include "mrocclusion/VectorTypes.hpp"
#include <Eigen/Dense>
#include <fstream>

RigidTransform::RigidTransform()
	:rotation(mat3::Identity()), translation(vec3::Zero())
{}

RigidTransform::RigidTransform(const mat3 &r, const vec3 &t)
	: rotation(r), translation(t)
{}

RigidTransform::RigidTransform(const mat4 &m)
	: rotation(m.block<3, 3>(0, 0)), translation(m.block<3, 1>(0, 3))
{}

RigidTransform  RigidTransform::inverse() const
{
	return RigidTransform(this->operator mat4().inverse());
}

RigidTransform & RigidTransform::operator=(const mat4 & m)
{
	rotation = m.block<3, 3>(0, 0);
	translation = m.block<3, 1>(0, 3);
	return *this;
}

vec3 RigidTransform::operator *(const vec3 &rhs) const
{
	return rotation * rhs + translation;
}

RigidTransform::operator mat4() const
{
	mat4 m = mat4::Zero();
	m.block<3, 3>(0, 0) = rotation;
	m.block<3, 1>(0, 3) = translation;
	m(3, 3) = 1.f;
	return m;
}

vec4 RigidTransform::operator *(const vec4 &rhs) const
{
	return this->operator mat4() * rhs;
}


RigidTransform RigidTransform::loadFromFile(const std::string &filename)
{
	std::ifstream file(filename);
	RigidTransform t;
	float r1, r2, r3, r4, r5, r6, r7, r8, r9;
	file >> r1 >> r2 >> r3 >> r4 >> r5 >> r6 >> r7 >> r8 >> r9;
	t.rotation <<
		r1, r2, r3,
		r4, r5, r6,
		r7, r8, r9;
	file >> r1 >> r2 >> r3;
	t.translation << r1, r2, r3;
	return t;
}

//!\brief Fit a viewport within supplied limits, maintaining a given aspect ratio.
//!\note For example, given a 320x240 viewport and a ratio of 1, will return a
//!      centred 240x240 viewport.
//!\note Useful for showing e.g. an image without stretching, distortion.
//!\param v The larger viewport to fit the subviewport into.
//!\param aspect The desired aspect ratio (given as width/height).
Viewport fitViewportWithAspectRatio(Viewport v, float aspect)
{
	float bigAspect = float(v.w) / float(v.h);

	if(bigAspect == aspect) {
		return v;
	}

	Viewport newV;
	if(bigAspect > aspect) {
		//big viewport is too wide.
		newV.h = v.h;
		newV.w = int(aspect * v.h);
		newV.x = v.x + ((v.w - newV.w)/2);
		newV.y = v.y;
	} else {
		//big viewport is too high.
		newV.w = v.w;
		newV.h = int(v.w / aspect);
		newV.y = v.y - ((v.h - newV.h)/2);
		newV.x = v.x;
	}
	return newV;
}

vec3 reflect(vec3 dir, vec3 norm)
{
	return dir - (2.f * norm.dot(dir) * norm);
}

void TriangleMeshIndices::applyTransformInPlace(const mat4 &transform)
{
	for(vec3 &v : vertices) {
		vec4 v4(v.x(), v.y(), v.z(), 1.f);
		vec4 tv4 = transform * v4;
		v = vec3(tv4.x(), tv4.y(), tv4.z()) / tv4.w();
	}
}

void TriangleMeshIndices::append(const TriangleMeshIndices &other)
{
	GLuint indexOffset = GLuint(vertices.size());
	vertices.reserve(vertices.size() + other.vertices.size());
	for(const vec3 &v : other.vertices) {
		vertices.push_back(v);
	}
	indices.reserve(indices.size() + other.indices.size());
	for(const GLuint &i : other.indices) {
		indices.push_back(i + indexOffset);
	}
}

