#ifndef MROCCLUSION_ARCAMERA_HPP_INCLUDED
#define MROCCLUSION_ARCAMERA_HPP_INCLUDED

#include "mrocclusion/PinholeCameraModel.hpp"
#include <mrocclusion/GLBuffer.hpp>


class ARCamera final {
public:
	explicit ARCamera(const PinholeCameraModel &model);
	~ARCamera() throw();
	
	void updatePose(const WorldToCamTransform &t);

	void bindCameraBlock();

private:
	UniformBuffer buffer_;
	CameraBlock block_;
	mat4 perspective_;
};


#endif //MROCCLUSION_ARCAMERA_HPP_INCLUDED
