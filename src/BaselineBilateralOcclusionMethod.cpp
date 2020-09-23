#include "mrocclusion/BaselineBilateralOcclusionMethod.hpp"
#include <opencv2/ximgproc/edge_filter.hpp>

BaselineBilateralOcclusionMethod::BaselineBilateralOcclusionMethod(size_t colorWidth, size_t colorHeight, size_t depthWidth, size_t depthHeight)
	:RgbdOcclusionMethod(colorWidth, colorHeight, depthWidth, depthHeight),
	colorSigma(30.0),
	depthSigma(12.0),
	radius(20.0)
{}

BaselineBilateralOcclusionMethod::~BaselineBilateralOcclusionMethod() throw()
{
}

void BaselineBilateralOcclusionMethod::calculateOcclusion(const unsigned char * rgbdColor, const unsigned short * rgbdDepth, const unsigned short * virtualDepth, unsigned char * matte)
{
	cv::Mat initialMatte(cv::Size(colorWidth, colorHeight), CV_8UC1);
	cv::Mat rgbdColorMat(cv::Size(colorWidth, colorHeight), CV_8UC3, (void*)(rgbdColor));
	cv::Mat rgbdDepthMat(cv::Size(depthWidth, depthHeight), CV_16UC1, (void*)(rgbdDepth));
	cv::Mat virtualDepthMat(cv::Size(depthWidth, depthHeight), CV_16UC1, (void*)(virtualDepth));

	initialMatte.setTo(0);
	initialMatte.setTo(255, (virtualDepthMat != 0) & (virtualDepthMat < rgbdDepthMat));
	initialMatte.setTo(128, (virtualDepthMat != 0) & (rgbdDepthMat == 0));

	cv::Mat matteMat(cv::Size(colorWidth, colorHeight), CV_8UC1, matte);
	cv::ximgproc::jointBilateralFilter(rgbdColorMat, initialMatte, matteMat, radius, colorSigma, depthSigma);
	matteMat.setTo(255, (virtualDepthMat != 0) & (virtualDepthMat < rgbdDepthMat));
	matteMat.setTo(0, (virtualDepthMat == 0));
	matteMat.setTo(0, (virtualDepthMat != 0) & (virtualDepthMat > rgbdDepthMat) & (rgbdDepthMat != 0));
}

std::string BaselineBilateralOcclusionMethod::getName() const
{
	return "Baseline Bilateral";
}

void BaselineBilateralOcclusionMethod::processEvent(SDL_Event & e)
{
}
