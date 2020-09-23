#ifndef MROCCLUSION_RGBDSLAM_HPP_INCLUDED
#define MROCCLUSION_RGBDSLAM_HPP_INCLUDED

#include "mrocclusion/PinholeCameraModel.hpp"

//!\brief Class encapsulating an RGBD SLAM method.
//!\note Accepts RGB and depth frames, and returns current camera transform.
class RgbdSlam 
{
public:
	explicit RgbdSlam(
		const PinholeCameraModel &colorCamModel,
		const PinholeCameraModel &depthCamModel,
		const mat4 &rgbToDepth);
	virtual ~RgbdSlam() throw();

	//!\brief Process the supplied depth and color frames, updating the current
	//!       transform estimate and world model.
	virtual void processFrames(
		const unsigned char *colorFrame,
		const unsigned short *depthFrame,
		const mat3 *imuMeasurement = nullptr) = 0;

	//!\brief Return the current camera transform estimate.
	//!\note This transform estimate is for the time at which the 
	//!      most recently processed frames were captured.
	//!\sa RgbdSlam::processFrames()
	virtual WorldToCamTransform currTransform() = 0;

	virtual bool trackingGood() = 0;

	virtual const unsigned char *reconstructionRaycast() = 0;

	virtual const unsigned char *reconstructionRaycast(
		const WorldToCamTransform &camPose,
		const PinholeCameraModel &camModel) = 0;
private:

};

#endif
