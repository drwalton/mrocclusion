#include "mrocclusion/AdaptiveManifoldOcclusionMethod.hpp"
#include <opencv2/ximgproc/edge_filter.hpp>

AdaptiveManifoldOcclusionMethod::AdaptiveManifoldOcclusionMethod(
	size_t colorWidth, size_t colorHeight,
	size_t depthWidth, size_t depthHeight)
	:RgbdOcclusionMethod(colorWidth, colorHeight, depthWidth, depthHeight),
	r_(0.1f), s_(4.f)
{}

AdaptiveManifoldOcclusionMethod::~AdaptiveManifoldOcclusionMethod() throw()
{}

float AdaptiveManifoldOcclusionMethod::r() const
{
	return r_;
}

float AdaptiveManifoldOcclusionMethod::s() const
{
	return s_;
}

void AdaptiveManifoldOcclusionMethod::r(float r)
{
	r_ = r;
}

void AdaptiveManifoldOcclusionMethod::s(float e)
{
	s_ = e;
}


void AdaptiveManifoldOcclusionMethod::calculateOcclusion(
	const unsigned char *rgbdColor,
	const unsigned short *rgbdDepth,
	const unsigned short *virtualDepth,
	unsigned char * matte)
{
	const cv::Mat realColor(cv::Size(colorWidth, colorHeight), CV_8UC3, (void*)(rgbdColor));
	const cv::Mat realDepth(cv::Size(depthWidth, depthHeight), CV_16UC1, (void*)(rgbdDepth));
	const cv::Mat virtualDepthMat(cv::Size(depthWidth, depthHeight), CV_16UC1, (void*)(virtualDepth));
	cv::Mat process, behind, infront;

	cv::Mat initialMatte(cv::Size(colorWidth, colorHeight), CV_8UC1);
	initialMatte.setTo(0);

	initialMatte.setTo(255, (realDepth > virtualDepthMat) & (virtualDepthMat != 0));
	initialMatte.setTo(128, (realDepth == 0) & (virtualDepthMat != 0));
	if (debugMode) {
		cv::imshow("initial Matte", initialMatte);
		cv::imshow("virtual depth", virtualDepthMat);
	}

	cv::Mat output(cv::Size(colorWidth, colorHeight), CV_8UC1, matte);
	cv::Ptr<cv::ximgproc::AdaptiveManifoldFilter> f = cv::ximgproc::createAMFilter(s_, r_);
	cv::Mat result;
	f->filter(initialMatte, result, realColor);
	result.setTo(0, virtualDepthMat == 0);
	result.setTo(0, (realDepth < virtualDepthMat) & (realDepth != 0));
	result.setTo(255, (realDepth > virtualDepthMat) & (virtualDepthMat != 0));
	result.copyTo(output);
}

void AdaptiveManifoldOcclusionMethod::processEvent(SDL_Event & event)
{
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_r) {
			if (event.key.keysym.mod & KMOD_LSHIFT) {
				if (r() <= 1.f - 0.01f) {
					r(r() + 0.01);
				}
			} else {
				if (r() >= 0.f + 0.01f) {
					r(r() - 0.01f);
				}
			}
		}
		if (event.key.keysym.sym == SDLK_s) {
			if (event.key.keysym.mod & KMOD_LSHIFT) {
				s(s() + 1.f);
			} else {
				if (s() > 0.f) {
					s(s() - 1.f);
				}
			}
		}
	}
	std::cout << "Guided R: " << r() << ", s: " << s() << std::endl;
}
