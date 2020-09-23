#include "mrocclusion/NaiveRgbdOcclusionMethod.hpp"

#include <cmath>
#include <thread>
#include <opencv2/opencv.hpp>

typedef unsigned char uchar;

NaiveRgbdOcclusionMethod::NaiveRgbdOcclusionMethod(
	size_t colorWidth, size_t colorHeight,
	size_t depthWidth, size_t depthHeight,
	bool assumeInfront)
	:RgbdOcclusionMethod(
		colorWidth, colorHeight,
		depthWidth, depthHeight),
	assumeInfront_(assumeInfront)
{
}

NaiveRgbdOcclusionMethod::~NaiveRgbdOcclusionMethod() throw()
{
}

void NaiveRgbdOcclusionMethod::calculateOcclusion(
	const unsigned char *rgbdColor, 
	const unsigned short *rgbdDepth, 
	const unsigned short *virtualDepth,
	unsigned char * matte)
{
	const cv::Mat realColor(cv::Size(colorWidth, colorHeight), CV_8UC3, (void*)(rgbdColor));
	const cv::Mat realDepth(cv::Size(depthWidth, depthHeight), CV_16UC1, (void*)(rgbdDepth));
	const cv::Mat virtualDepthMat(cv::Size(depthWidth, depthHeight), CV_16UC1, (void*)(virtualDepth));
	cv::Mat process, behind, infront;
	behind = (realDepth > virtualDepthMat) & (realDepth != 0) & (virtualDepthMat != 0);
	infront = (realDepth < virtualDepthMat) & (realDepth != 0);

	cv::Mat matteByte(cv::Size(colorWidth, colorHeight), CV_8UC1, matte);
	if (assumeInfront_) {
		matteByte = ((virtualDepthMat != 0) & behind);
	} else {
		matteByte = ((virtualDepthMat != 0) & (behind | ~infront));
	}
	if (debugMode) {
		cv::imshow("Final matte", matteByte);
		cv::waitKey(1);
	}
}

void NaiveRgbdOcclusionMethod::processEvent(SDL_Event & e)
{
}

std::string NaiveRgbdOcclusionMethod::getName() const
{
	return std::string("Naive ") + (assumeInfront_ ? "infront" : "behind");
}
