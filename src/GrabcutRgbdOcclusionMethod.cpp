#include "mrocclusion/GrabcutRgbdOcclusionMethod.hpp"

GrabcutRgbdOcclusionMethod::GrabcutRgbdOcclusionMethod(
	size_t colorWidth, size_t colorHeight,
	size_t depthWidth, size_t depthHeight)
	:RgbdOcclusionMethod(colorWidth, colorHeight, depthWidth, depthHeight),
	useOffVirtualObject_(false),
	iters_(1)
{}

GrabcutRgbdOcclusionMethod::~GrabcutRgbdOcclusionMethod() throw()
{}


void GrabcutRgbdOcclusionMethod::calculateOcclusion(
	const unsigned char *rgbdColor, 
	const unsigned short *rgbdDepth,
	const unsigned short *virtualDepth,
	unsigned char * matte)
{
	const cv::Mat realColor(cv::Size(colorWidth, colorHeight), CV_8UC3, (void*)(rgbdColor));
	const cv::Mat realDepth(cv::Size(depthWidth, depthHeight), CV_16UC1, (void*)(rgbdDepth));
	const cv::Mat virtualDepthMat(cv::Size(depthWidth, depthHeight), CV_16UC1, (void*)(virtualDepth));
	cv::Mat process, behind, infront;
	classifyPixels(realColor, realDepth, virtualDepthMat,
		process, behind, infront, useOffVirtualObject_);

	double min, maxBehind, maxInfront;
	cv::minMaxLoc(behind, &min, &maxBehind);
	cv::minMaxLoc(infront, &min, &maxInfront);
	cv::Mat cvMask(cv::Size(colorWidth, colorHeight), CV_8UC1);
	cvMask.setTo(cv::GC_BGD);
	if (maxBehind == 0 || maxInfront == 0) {
	} else {
		//cvMask.setTo(cv::GC_BGD, behind);
		cvMask.setTo(cv::GC_FGD, infront);
		cvMask.setTo(cv::GC_PR_BGD, process);
		
		cv::Mat bgdModel, fgdModel;
		if (debugMode) {
			cv::imshow("input PR BGD", cvMask == cv::GC_PR_BGD);
			cv::imshow("input BGD", cvMask == cv::GC_BGD);
		}
		cv::grabCut(realColor, cvMask, cv::Rect(), bgdModel, fgdModel, iters_, cv::GC_INIT_WITH_MASK);
		if (debugMode) {
			cv::imshow("output BGD", cvMask == cv::GC_BGD | cvMask == cv::GC_PR_BGD);
			cv::waitKey(1);
		}
	}
	
	
	cv::Mat output(cv::Size(colorWidth, colorHeight), CV_8UC1, matte);
	output.setTo(0);
	output.setTo(255, (virtualDepthMat != 0) & (behind | process) & (cvMask == cv::GC_BGD | cvMask == cv::GC_PR_BGD));
}

void GrabcutRgbdOcclusionMethod::processEvent(SDL_Event &event)
{
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_o) {
			usePixelsOffVirtualObject(!usePixelsOffVirtualObject());
		}
	}
}
