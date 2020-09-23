#include "mrocclusion/ArCamera.hpp"
#include <mrocclusion/Constants.hpp>
#include <mrocclusion/Matrices.hpp>
#include <iostream>


const float zFar = 50.f;
const float zNear = 0.01f;


mat4 glPerspectiveFromPinholeModel(const PinholeCameraModel &model) 
{
	float fx = 2.0f * model.fx() / model.width();
	float fy = 2.0f * model.fy() / model.height();
	float cx = (2.f * model.cx() / model.width()) - 1.f;
	float cy = (2.f * model.cy() / model.height()) - 1.f;
	mat4 p = mat4::Zero();

	p(0, 0) = fx;
	p(1, 1) = fy;

	p(0, 3) = cx;
	p(1, 3) = cy;

	p(2, 2) = (zFar + zNear) / (zNear - zFar);
	p(3, 2) = -1.0f;
	p(2, 3) = (2.0f * zFar * zNear) / (zNear - zFar);

	std::cout << "Created AR GL Perspective matrix:\n" << p << std::endl;

	mat4 p2 = perspective(M_PI_2 * 0.5f, 640.f / 480.f, zFar, zNear);
	std::cout << "For comparison, a GL Perspective matrix:\n" << p2 << std::endl;

	return p;
}

ARCamera::ARCamera(const PinholeCameraModel & model)
	:buffer_(&block_, sizeof(CameraBlock)),
	perspective_(glPerspectiveFromPinholeModel(model))
{
	updatePose(WorldToCamTransform());
	bindCameraBlock();
}

ARCamera::~ARCamera() throw()
{
}

void ARCamera::updatePose(const WorldToCamTransform & t)
{
	block_.worldToClip = perspective_ * t;//static_cast<mat4>(t);
	vec4 dir;
	dir[3] = 1.f;
	dir.block<3, 1>(0, 0) = t.block<3, 3>(0, 0).inverse() * vec3(0.f, 0.f, -1.f);
	block_.cameraDir = dir;
	block_.cameraPos = t.inverse() * vec4(0.f, 0.f, 0.f, 1.f);
	buffer_.update(&block_);
}

void ARCamera::bindCameraBlock()
{
	buffer_.bindRange(UniformBlock::CAMERA);
}


