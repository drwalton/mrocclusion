#include "mrocclusion/GuidedRgbdOcclusionMethod.hpp"
#include "mrocclusion/guidedfilter.h"

void guidedFilter(cv::Mat &p, cv::Mat &I, cv::Mat &q,
	size_t radius, float e)
{
	int width = radius*2 + 1;
	cv::Mat If, pf;
	if(I.channels() != 1) {
		cv::Mat I1;
		cv::cvtColor(I, I1, cv::COLOR_RGB2GRAY);
		I1.convertTo(If, CV_32FC1);
		cv::Mat p1;
		cv::cvtColor(p, p1, cv::COLOR_RGB2GRAY);
		p1.convertTo(pf, CV_32FC1);
	} else {
    	I.convertTo(If, CV_32FC1);
    	p.convertTo(pf, CV_32FC1);
	}
	If /= 255.f;
	pf /= 255.f;
	cv::Mat meanI = I, meanp = p;
	cv::boxFilter(If, meanI, -1, cv::Size(width, width));
	cv::boxFilter(pf, meanp, -1, cv::Size(width, width));
	cv::Mat II = If.mul(If);
	cv::Mat corrI;
	cv::boxFilter(II, corrI, -1, cv::Size(width, width));
	cv::Mat Ip = If.mul(pf);
	cv::Mat corrIp;
	cv::boxFilter(Ip, corrIp, -1, cv::Size(width, width));
	cv::Mat varI = corrI - (meanI.mul(meanI));
	cv::Mat covIp = corrIp - (meanI.mul(meanp));
	cv::imshow("covIp", covIp / 255.f);

	cv::Mat a = covIp / (varI + e);
	cv::Mat b = meanp - a.mul(meanI);

	cv::Mat meana, meanb;
	cv::boxFilter(a, meana, -1, cv::Size(width, width));
	cv::boxFilter(b, meanb, -1, cv::Size(width, width));

	cv::imshow("meana", meana);
	cv::imshow("meanb", meanb);
	cv::Mat qf = (meana.mul(If) + meanb);
	qf *= 255.f;
	qf.convertTo(q, CV_8UC1);
}

GuidedRgbdOcclusionMethod::GuidedRgbdOcclusionMethod(
	size_t colorWidth, size_t colorHeight,
	size_t depthWidth, size_t depthHeight)
	:RgbdOcclusionMethod(colorWidth, colorHeight, depthWidth, depthHeight),
	r_(20), e_(0.001f)
{}

GuidedRgbdOcclusionMethod::~GuidedRgbdOcclusionMethod() throw()
{}

int GuidedRgbdOcclusionMethod::r() const
{
	return r_;
}

float GuidedRgbdOcclusionMethod::e() const
{
	return e_;
}

void GuidedRgbdOcclusionMethod::r(int r)
{
	r_ = r;
}

void GuidedRgbdOcclusionMethod::e(float e)
{
	e_ = e;
}


void GuidedRgbdOcclusionMethod::calculateOcclusion(
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
	//initialMatte.setTo(128, (realDepth == 0) & (virtualDepthMat != 0));
	if (debugMode) {
		cv::imshow("initial Matte", initialMatte);
		cv::imshow("virtual depth", virtualDepthMat);
		cv::waitKey(1);
	}

	cv::Mat output(cv::Size(colorWidth, colorHeight), CV_8UC1, matte);
	cv::Mat result = guidedFilter(realColor, initialMatte, r_, e_);
	result.setTo(0, virtualDepthMat == 0);
	result.setTo(0, (realDepth < virtualDepthMat) & (realDepth != 0));
	result.setTo(255, (realDepth > virtualDepthMat) & (virtualDepthMat != 0));
	result.copyTo(output);
}

void GuidedRgbdOcclusionMethod::processEvent(SDL_Event & event)
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
				e(e() + 1.01f);
			} else {
				if (e() > 0.f) {
					e(e() - 1.01f);
				}
			}
		}
	}
	std::cout << "Guided R: " << r() << ", eps: " << e() << std::endl;
}
