#include "mrocclusion/BaselineGuidedOcclusionMethod.hpp"
#include <opencv2/ximgproc/edge_filter.hpp>

BaselineGuidedOcclusionMethod::BaselineGuidedOcclusionMethod(
		size_t colorWidth, size_t colorHeight,
		size_t depthWidth, size_t depthHeight)
	:RgbdOcclusionMethod(colorWidth, colorHeight, depthWidth, depthHeight),
	eps(0.01f),
	radius(20.f)
{}

BaselineGuidedOcclusionMethod::~BaselineGuidedOcclusionMethod() throw()
{
}

void BaselineGuidedOcclusionMethod::calculateOcclusion(
	const unsigned char * rgbdColor, 
	const unsigned short * rgbdDepth, 
	const unsigned short * virtualDepth, 
	unsigned char * matte)
{
	cv::Mat initialMatte(cv::Size(colorWidth, colorHeight), CV_8UC1);
	cv::Mat rgbdColorMat(cv::Size(colorWidth, colorHeight), CV_8UC3, (void*)(rgbdColor));
	cv::Mat rgbdDepthMat(cv::Size(depthWidth, depthHeight), CV_16UC1, (void*)(rgbdDepth));
	cv::Mat virtualDepthMat(cv::Size(depthWidth, depthHeight), CV_16UC1, (void*)(virtualDepth));
	cv::Mat behind, infront, process;
	classifyPixels(rgbdColorMat, rgbdDepthMat, virtualDepthMat, process, behind, infront, false);

	initialMatte.setTo(0);
	initialMatte.setTo(255, behind);
	initialMatte.setTo(128, process);

	cv::Mat matteMat(cv::Size(colorWidth, colorHeight), CV_8UC1, matte);
	cv::ximgproc::guidedFilter(rgbdColorMat, initialMatte, matteMat, radius, eps);
	matteMat.setTo(255, (virtualDepthMat != 0) & (virtualDepthMat < rgbdDepthMat));
	matteMat.setTo(0, (virtualDepthMat == 0));
	matteMat.setTo(0, (virtualDepthMat != 0) & (virtualDepthMat > rgbdDepthMat) & (rgbdDepthMat != 0));
}

std::string BaselineGuidedOcclusionMethod::getName() const
{
	return "Baseline Guided";
}

void BaselineGuidedOcclusionMethod::processEvent(SDL_Event & e)
{
}
