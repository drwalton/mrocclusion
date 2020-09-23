#include "mrocclusion/Matrices.hpp"
#include <Eigen/Dense>
#include <vector>


mat4 rotToMat4(const mat3 &r) {
	mat4 m = mat4::Identity();
	m.block<3,3>(0,0) = r;
	return m;
}
mat4 angleAxisMat4(float angle, const vec3 &axis) {
	return rotToMat4(Eigen::AngleAxisf(angle, axis).toRotationMatrix());
}

mat4 perspective(float fov, float aspect, float zNear, float zFar)
{
	float tanHalfFovy = tan(fov * 0.5f);

	mat4 result = mat4::Zero();
	result(0, 0) = 1.0f / (aspect * tanHalfFovy);
	//result(0, 0) = zNear / (aspect * tanHalfFovy);
	result(1, 1) = 1.0f / tanHalfFovy;
	//result(1, 1) = zNear / tanHalfFovy;
	result(2, 2) = (zFar + zNear) / (zNear - zFar);
	result(3, 2) = -1.0f;
	result(2, 3) = (2.0f * zFar * zNear) / (zNear - zFar);

	return result;
}


mat4 rotationAboutAxis(float angle, vec3 axis)
{
	auto rotation = Eigen::AngleAxisf(angle, axis).toRotationMatrix();
	mat4 rot4 = mat4::Identity();
	rot4 <<
		rotation(0, 0), rotation(0, 1), rotation(0, 2), 0.0f,
		rotation(1, 0), rotation(1, 1), rotation(1, 2), 0.0f,
		rotation(2, 0), rotation(2, 1), rotation(2, 2), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f;
	return rot4;
}

CamTransform::CamTransform()
:rotation_(mat3::Identity()), translation_(vec3::Zero())
{}

CamTransform::CamTransform(const mat4 &m)
: rotation_(m.block<3, 3>(0, 0)), translation_(m.block<3, 1>(0,3))
{}

using boost::property_tree::ptree;

template <typename T>
std::vector<T> as_vector(ptree const& pt, ptree::key_type const& key)
{
	std::vector<T> r;
	for (auto& item : pt.get_child(key))
		r.push_back(item.second.get_value<T>());
	return r;
}

CamTransform::CamTransform(const boost::property_tree::ptree &t)
{
	std::vector<float> r = as_vector<float>(t, "rotation");
	std::vector<float> v = as_vector<float>(t, "translation");
	memcpy(rotation_.data(), r.data(), 9 * sizeof(float));
	memcpy(translation_.data(), v.data(), 3 * sizeof(float));
}

CamTransform::~CamTransform() throw()
{}

vec3 CamTransform::translation() const
{
	return translation_;
}
CamTransform &CamTransform::translation(const vec3 &t)
{
	translation_ = t;
	return *this;
}

mat3 CamTransform::rotation() const
{
	return rotation_;
}
CamTransform &CamTransform::rotation(const mat3 &t)
{
	rotation_ = t;
	return *this;
}

CamTransform CamTransform::inverse() const
{
	return CamTransform(operator mat4().inverse());
}

vec3 CamTransform::operator*(const vec3 &v) const
{
	return rotation_*v + translation_;
}

CamTransform CamTransform::operator*(const CamTransform &rhs) const
{
	mat4 l = this->operator mat4(), r = rhs.operator mat4();
	return CamTransform(l*r);
}

CamTransform::operator mat4() const
{
	mat4 ans = mat4::Identity();
	ans.block<3, 3>(0, 0) = rotation_;
	ans.block<3, 1>(0, 3) = translation_;
	return ans;
}

mat3 CamTransform::makeEpipolarMatrix() const
{
	mat3 tx;
	tx <<
		0.f, -translation_[2], translation_[1],
		translation_[2], 0.f, -translation_[0],
		-translation_[1], translation_[0], 0.f;
	return tx * rotation_;
}

bool convertProjectionMatrixToOpenGLStyle(float cparam[3][4], 
	int width, int height, float gnear, float gfar, float m[16]) {
    float trans[3][4];
    float p[3][3], q[4][4];
    int i, j;

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            p[i][j] = cparam[i][j] / cparam[2][2];
        }
    }
    q[0][0] = (2.0f * p[0][0] / width);
    q[0][1] = (2.0f * p[0][1] / width);
    q[0][2] = ((2.0f * p[0][2] / width) - 1.0f);
    q[0][3] = 0.0f;

    q[1][0] = 0.0f;
    q[1][1] = (2.0f * p[1][1] / height);
    q[1][2] = ((2.0f * p[1][2] / height) - 1.0f);
    q[1][3] = 0.0f;

    q[2][0] = 0.0f;
    q[2][1] = 0.0f;
    q[2][2] = (gfar + gnear) / (gfar - gnear);
    q[2][3] = -2.0f * gfar * gnear / (gfar - gnear);

    q[3][0] = 0.0f;
    q[3][1] = 0.0f;
    q[3][2] = 1.0f;
    q[3][3] = 0.0f;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 3; j++) {
            m[i + j * 4] = q[i][0] * trans[0][j] + q[i][1] * trans[1][j] + q[i][2] * trans[2][j];
        }
        m[i + 3 * 4] = q[i][0] * trans[0][3] + q[i][1] * trans[1][3] + q[i][2] * trans[2][3] + q[i][3];
    }

    return true;
}

mat4 scaleMat4(float scale) {
	mat4 a = mat4::Identity();
	a(0,0) *= scale;
	a(1,1) *= scale;
	a(2,2) *= scale;
	return a;
}

mat4 translateMat4(vec3 translate) {
	mat4 a = mat4::Identity();
	a.block<3, 1>(0, 3) = translate;
	return a;
}

mat4 lookAt(vec3 eye, vec3 center, vec3 up)
{
	vec3 f = (center - eye).normalized();
	vec3 u = up.normalized();
	vec3 s = f.cross(u).normalized();
	u = s.cross(f);

	mat4 res;
	res << s.x(), s.y(), s.z(), -s.dot(eye),
		u.x(), u.y(), u.z(), -u.dot(eye),
		-f.x(), -f.y(), -f.z(), f.dot(eye),
		0, 0, 0, 1;

	return res;
}

vec3 applyTransform(const mat4 & T, const vec3 & v)
{
	vec4 v4(v.x(), v.y(), v.z(), 1.f);
	v4 = T * v4;
	return vec3(v4.x() / v4.w(), v4.y() / v4.w(), v4.z() / v4.w());
}
	
