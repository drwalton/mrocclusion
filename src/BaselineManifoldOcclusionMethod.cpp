#include "mrocclusion/BaselineManifoldOcclusionMethod.hpp"
#include <opencv2/ximgproc/edge_filter.hpp>

BaselineManifoldOcclusionMethod::BaselineManifoldOcclusionMethod(size_t colorWidth, size_t colorHeight, size_t depthWidth, size_t depthHeight)
	:RgbdOcclusionMethod(colorWidth, colorHeight, depthWidth, depthHeight),
	sigma_s(4.0f),
	sigma_r(.1f)
{
}

BaselineManifoldOcclusionMethod::~BaselineManifoldOcclusionMethod() throw()
{
}

void BaselineManifoldOcclusionMethod::calculateOcclusion(const unsigned char * rgbdColor, const unsigned short * rgbdDepth, const unsigned short * virtualDepth, unsigned char * matte)
{
	cv::Mat initialMatte(cv::Size(colorWidth, colorHeight), CV_8UC1);
	cv::Mat rgbdColorMat(cv::Size(colorWidth, colorHeight), CV_8UC3, (void*)(rgbdColor));
	cv::Mat rgbdDepthMat(cv::Size(depthWidth, depthHeight), CV_16UC1, (void*)(rgbdDepth));
	cv::Mat virtualDepthMat(cv::Size(depthWidth, depthHeight), CV_16UC1, (void*)(virtualDepth));

	initialMatte.setTo(0);
	initialMatte.setTo(255, (virtualDepthMat != 0) & (virtualDepthMat < rgbdDepthMat));
	initialMatte.setTo(128, (virtualDepthMat != 0) & (rgbdDepthMat == 0));

	cv::Mat matteMat(cv::Size(colorWidth, colorHeight), CV_8UC1, matte);
	auto filter = cv::ximgproc::createAMFilter(sigma_s, sigma_r);
	filter->filter(initialMatte, matteMat, rgbdColorMat);
	matteMat.setTo(255, (virtualDepthMat != 0) & (virtualDepthMat < rgbdDepthMat));
	matteMat.setTo(0, (virtualDepthMat == 0));
	matteMat.setTo(0, (virtualDepthMat != 0) & (virtualDepthMat > rgbdDepthMat) & (rgbdDepthMat != 0));
}

std::string BaselineManifoldOcclusionMethod::getName() const
{
	return "Baseline Manifold";
}

void BaselineManifoldOcclusionMethod::processEvent(SDL_Event & e)
{
}
