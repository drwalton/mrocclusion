#ifndef MROCCLUSION_RGBD_INFTAMRGBDSLAM_HPP_INCLUDED
#define MROCCLUSION_RGBD_INFTAMRGBDSLAM_HPP_INCLUDED

#include "mrocclusion/RgbdSlam.hpp"
#include <memory>


//!\brief RGBD SLAM implementation using InfiniTAM v2.
class InftamRgbdSlam : public RgbdSlam
{
public:
	//!\note Meshing engine requires lots of memory. It is recommended to disable this
	//!      feature on GPUs of ~2GB or less.
	explicit InftamRgbdSlam(
		const PinholeCameraModel &colorCamModel,
		const PinholeCameraModel &depthCamModel,
		const mat4 &rgbToDepth,
		bool makeMeshingEngine = true,
		bool useImu = false);
	virtual ~InftamRgbdSlam() throw();

	virtual void processFrames(
		const unsigned char *colorFrame, 
		const unsigned short *depthFrame,
		const mat3 *imuMeasurement = nullptr);

	virtual WorldToCamTransform currTransform();

	virtual bool trackingGood();

	//!\brief Return a pointer to a raycasted image of the 
	//!       current scene model, from the current estimated
	//!       camera pose.
	//!\return A pointer to the image, in RGBA8888 format.
	virtual const unsigned char *reconstructionRaycast();

	//!\brief Return a pointer to a raycasted image of the 
	//!       current scene model, using the specified camera
	//!       pose and model.
	//!\return A pointer to the image, in RGBA8888 format.
	virtual const unsigned char *reconstructionRaycast(
		const WorldToCamTransform &camPose,
		const PinholeCameraModel &camModel);
	
	//!\brief Return expected point cloud positions
	//!       (from reconstruction, and current estimated pose).
	vec4 *pointCloudPositions();
	
	//!\brief Return expected point cloud colors
	//!       (from reconstruction, and current estimated pose).
	vec4 *pointCloudColors();
	
	//!\brief Calculate and return expected depth map, in 16-bit unsigned int
	//!       format (from reconstruction, and current estimated pose).
	//!\note Currently a fairly slow CPU implementation
	//!\note Based on output of pointCloudPositions
	unsigned short *expectedDepthMap();

	//!\brief Turn on/off integration.
	//!       If disabled, tracking will continue but the reconstruction will
	//!       not be updated with new information.
	void doIntegration(bool i);

	void saveSceneToMesh(const std::string &filename);

	void meshScene();
private:
	struct Impl;
	std::unique_ptr<Impl> pimpl_;
};


#endif
