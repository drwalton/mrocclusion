#include "mrocclusion/DepthCam.hpp"
#include "mrocclusion/OpenNiDepthCam.hpp"

DepthCam::DepthCam()
{}

DepthCam::~DepthCam() throw()
{}

std::unique_ptr<DepthCam> openDefaultDepthCam()
{
	return std::unique_ptr<DepthCam>(new OpenNiDepthCam());
}

void DepthCam::saveCalibrationImages(const std::string &filename)
{
	throw DepthCamException(
		"saveCalibrationImages() not implemented for this DepthCam implementation.");
}

