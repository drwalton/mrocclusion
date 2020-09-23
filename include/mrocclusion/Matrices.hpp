#ifndef MROCCLUSION_MATRICES_HPP_INCLUDED
#define MROCCLUSION_MATRICES_HPP_INCLUDED

#include "VectorTypes.hpp"
#include <Eigen/Dense>
#include <cmath>
#include <boost/property_tree/ptree.hpp>


#ifndef M_PI_2
#define M_PI_2     1.57079632679489661923  /* pi/2 */
#endif

mat4 rotToMat4(const mat3 &r);
mat4 angleAxisMat4(float angle, const vec3 &axis);

//!\brief Create a perspective matrix, suitable for use with OpenGL.
//! Intended to map from camera space to clip space.
//!\note The -ve z-axis goes into the screen! That is, the camera looks along
//!/     the negative z-axis
//!\param fov Field of view of the perspective projection, in radians.
//!  The FoV is given corner-to-corner, in radians.
//!\param aspect Aspect ratio of the image, i.e. width/height
//!\param zNear Near-plane distance. Points closer to the camera than zNear
//! along the z-axis will fall outside clip space. Should be positive.
//!\param zFar Far-plane distance. Points further from the camera than zFar
//! along the z-axis will fall outside clip space. Should be positive.
mat4 perspective(float fov, float aspect, float zNear, float zFar);

inline float depthToClipSpaceZ(float d, float zNear, float zFar)
{
	return -d*(zFar + zNear) / (-d*(zFar - zNear) - 2.f*zFar*zNear);
}

mat4 rotationAboutAxis(float angle, vec3 axis);

const mat4 blenderToGLMat = rotationAboutAxis(float(M_PI_2), vec3(1.f, 0.f, 0.f));

//!Mapping from world space to camera space.
class CamTransform {
public:
	explicit CamTransform();
	explicit CamTransform(const mat4 &m);
	explicit CamTransform(const boost::property_tree::ptree &t);
	~CamTransform() throw();

	mat3 rotation() const;
	CamTransform &rotation(const mat3 &r);

	vec3 translation() const;
	CamTransform &translation(const vec3 &t);

	CamTransform inverse() const;

	vec3 operator*(const vec3 &rhs) const;
	CamTransform operator*(const CamTransform &rhs) const;
	operator mat4() const;

	//!\brief This returns the epipolar matrix E st if this is the transform of c2 relative to c1,
	//!       corresponding points satisfy x_2^T E x_1 = 0.
	mat3 makeEpipolarMatrix() const;
private:
	mat3 rotation_;
	vec3 translation_;
};

bool convertProjectionMatrixToOpenGLStyle(float cparam[3][4], int width, int height, float gnear, float gfar, float m[16]);

mat4 scaleMat4(float scale);

mat4 translateMat4(vec3 translate);

mat4 lookAt(vec3 eye, vec3 center, vec3 up = vec3(0.f, 1.f, 0.f));

vec3 applyTransform(const mat4 &T, const vec3 &v);


#endif
