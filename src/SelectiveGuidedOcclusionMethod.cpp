#include "mrocclusion/SelectiveGuidedOcclusionMethod.hpp"
#include "mrocclusion/SelectiveGuidedFilter.hpp"

SelectiveGuidedOcclusionMethod::SelectiveGuidedOcclusionMethod(
	size_t colorWidth, size_t colorHeight,
	size_t depthWidth, size_t depthHeight)
	:RgbdOcclusionMethod(colorWidth, colorHeight, depthWidth, depthHeight),
	r_(15),
	eps_(0.1f),
	useOffVirtualObject_(false)
{

}

SelectiveGuidedOcclusionMethod::~SelectiveGuidedOcclusionMethod() throw()
{}

void SelectiveGuidedOcclusionMethod::calculateOcclusion(
	const unsigned char * rgbdColor, const unsigned short * rgbdDepth, 
	const unsigned short * virtualDepth, unsigned char * matte)
{
	const cv::Mat realColor(cv::Size(colorWidth, colorHeight), CV_8UC3, (void*)(rgbdColor));
	const cv::Mat realDepth(cv::Size(depthWidth, depthHeight), CV_16UC1, (void*)(rgbdDepth));
	const cv::Mat virtualDepthMat(cv::Size(depthWidth, depthHeight), CV_16UC1, (void*)(virtualDepth));
	cv::Mat process, behind, infront;
	classifyPixels(realColor, realDepth, virtualDepthMat,
		process, behind, infront, useOffVirtualObject_);
	cv::Mat initialMatte(cv::Size(colorWidth, colorHeight), CV_8UC1);
	initialMatte.setTo(0);
	initialMatte.setTo(255, behind & (virtualDepthMat != 0));
	initialMatte.setTo(0, infront & (~process));
	initialMatte.setTo(128, process);
	cv::Mat matteMat(cv::Size(colorWidth, colorHeight), CV_8UC1, matte);
	initialMatte.copyTo(matteMat);
	cv::Mat useMask = process | infront | behind;

	if (debugMode) {
		cv::imshow("processMask", process);
		cv::imshow("useMask", useMask);
	}

	selectiveGuidedFilter(initialMatte, realColor, process, useMask, &matteMat, r_, eps_);
}

void SelectiveGuidedOcclusionMethod::processEvent(SDL_Event & event)
{
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_r) {
			if (event.key.keysym.mod & KMOD_LSHIFT) {
				r(r() + 1);
			} else {
				if (r() >= 2) {
					r(r() - 1);
				}
			}
		}
		if (event.key.keysym.sym == SDLK_e) {
			if (event.key.keysym.mod & KMOD_LSHIFT) {
				eps(eps() + 0.01f);
			} else {
				if (eps() > 0.01f) {
					eps(eps() - 0.01f);
				}
			}
		}
	}
	if (debugMode) {
		std::cout << "Selective Guided R: " << r() << ", eps: " << eps() << std::endl;
	}
}

size_t SelectiveGuidedOcclusionMethod::r() const
{
	return r_;
}

float SelectiveGuidedOcclusionMethod::eps() const
{
	return eps_;
}

void SelectiveGuidedOcclusionMethod::r(size_t r)
{
	r_ = r;
}

void SelectiveGuidedOcclusionMethod::eps(float e)
{
	eps_ = e;
}
