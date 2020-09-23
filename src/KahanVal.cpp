#include "mrocclusion/KahanVal.hpp"


#ifndef _WIN32
template<> KahanVal<mat3>::KahanVal(mat3 initVal)
	:val(initVal), c(Eigen::Matrix3f::Zero()),
	y(Eigen::Matrix3f::Zero()), t(Eigen::Matrix3f::Zero())
{}

template<> KahanVal<vec3>::KahanVal(vec3 initVal)
	:val(initVal), c(Eigen::Vector3f::Zero()),
	y(Eigen::Vector3f::Zero()), t(Eigen::Vector3f::Zero())
{}
#endif

