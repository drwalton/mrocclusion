#include "mrocclusion/RgbdOcclusionMethod.hpp"
#include "mrocclusion/BilateralRgbdOcclusionMethod.hpp"

#include <cmath>
#include <thread>
#include <opencv2/opencv.hpp>

typedef unsigned char uchar;

inline float gaussian(float x, float sigma)
{
	x = fminf(x, 100.f); //Values above 100.f tend to be close to zero, and unstable.
	float a = x / sigma;
	return expf(-0.5f * a * a);
}

inline uchar absdiff(uchar a, uchar b) {
	return a < b ? b - a : a - b;
}

inline float colorDiff(const cv::Vec3b &c1, const cv::Vec3b &c2) {
	float diff = 0.f;
	diff += absdiff(c1[0], c2[0]);
	diff += absdiff(c1[1], c2[1]);
	diff += absdiff(c1[2], c2[2]);
	return diff;
}

inline float spatialDiff(int r1, int c1, int r2, int c2) {
	float diff = 0.f, tmp;
	tmp = float(r1) - float(r2);
	diff += tmp*tmp;
	tmp = float(c1) - float(c2);
	diff += tmp*tmp;
	return sqrtf(diff);
}

void processOneRow(int r, const cv::Mat &alphaMatte,
	uchar *processptr, const cv::Vec3b *rcolorPtr,
	const cv::Mat &behind, const cv::Mat &infront, const cv::Mat &realColor,
	float colorSigma_, float depthSigma_,
	int winSize, float *rptrOut)
{

	for (int c = 0; c < alphaMatte.cols; ++c) {
		if (processptr[c] == 0) {
			//Pixel not in "PROCESS" category, ignore.
			continue;
		}
		int nFgPixels = 0, nBgPixels = 0;
		float sumWeights = 0.f;
		float sumVals = 0.f;

		//Find bounds of window to iterate over.
		int left = std::max(0, c - winSize);
		int right = std::min(alphaMatte.cols - 1, c + winSize);
		int above = std::max(0, r - winSize);
		int below = std::min(alphaMatte.rows - 1, r + winSize);

		cv::Vec3b centerColor = rcolorPtr[c];

		//Iterate over window.
		for (int r2 = above; r2 <= below; ++r2) {
			const uchar *behindptr = behind.ptr<uchar>(r2);
			const uchar *infrontptr = infront.ptr<uchar>(r2);
			const cv::Vec3b *r2ColorPtr = realColor.ptr<cv::Vec3b>(r2);
			for (int c2 = left; c2 <= right; ++c2) {
				//On edge of window.
				float val;
				if (infrontptr[c2]) {
					++nFgPixels;
					val = 0.f;
				}
				else if (behindptr[c2]) {
					++nBgPixels;
					val = 1.f;
				}
				else {
					continue;
				}
				float cDiff = colorDiff(r2ColorPtr[c2], centerColor);
				float cWeight = gaussian(cDiff, colorSigma_);
				float sDiff = spatialDiff(r, c, r2, c2);
				float sWeight = gaussian(sDiff, depthSigma_);
				float weight = cWeight * sWeight;

				sumWeights += weight;
				sumVals += weight * val;
			}
		}

		if (sumWeights != 0.f) {
			rptrOut[c] = sumVals / sumWeights;
		}
	}
}

BilateralRgbdOcclusionMethod::BilateralRgbdOcclusionMethod(
	size_t colorWidth, size_t colorHeight, 
	size_t depthWidth, size_t depthHeight)
	:RgbdOcclusionMethod(
		colorWidth, colorHeight, 
		depthWidth, depthHeight),
	parallel_(true),
	colorSigma_(14.0),
	depthSigma_(8.0),
	filterSize_(32),
	useOffVirtualObject_(false)

{
}

BilateralRgbdOcclusionMethod::~BilateralRgbdOcclusionMethod() throw()
{
}

void BilateralRgbdOcclusionMethod::calculateOcclusion(
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
	cv::Mat ignore = ~infront & ~behind & (virtualDepth == 0);

	cv::Mat alphaMatte(ignore.size(), CV_32FC1);
	alphaMatte.setTo(0.f, ~process);
	alphaMatte.setTo(1.f, behind & (virtualDepth != 0));

	if (!parallel_) {
		for (size_t r = 0; r < depthHeight; ++r) {
			float *rptrOut = alphaMatte.ptr<float>(r);
			const cv::Vec3b *rcolorPtr = realColor.ptr<cv::Vec3b>(r);
			uchar *processptr = process.ptr<uchar>(r);

			processOneRow(r, alphaMatte, processptr, rcolorPtr, behind, infront,
				realColor, colorSigma_, depthSigma_,
				filterSize_, rptrOut);
		}
	} else {
		std::vector<std::thread> threads;
		for(int r = 0; r < alphaMatte.rows; ++r) {
			bool processThisRow = false;
			for (int c = 0; c < alphaMatte.cols; ++c) {
				if (process.at<uchar>(r, c) != 0) {
					processThisRow = true;
				}
			}

			if (processThisRow) {
				float *rptrOut = alphaMatte.ptr<float>(r);
				const cv::Vec3b *rcolorPtr = realColor.ptr<cv::Vec3b>(r);
				uchar *processptr = process.ptr<uchar>(r);

				threads.emplace_back([r, &alphaMatte, processptr, rcolorPtr,
					&behind, &infront, &realColor, this, rptrOut]() {
					processOneRow(r, alphaMatte, processptr, rcolorPtr, behind, infront,
						realColor, colorSigma_, depthSigma_,
						filterSize_, rptrOut);
				});
			}
		}
		
		for(std::thread &t : threads) {
			t.join();
		}
	}

	alphaMatte *= 255.f;
	cv::Mat matteByte(cv::Size(colorWidth, colorHeight), CV_8UC1, matte);
	alphaMatte.convertTo(matteByte, CV_8UC1);
}

void BilateralRgbdOcclusionMethod::processEvent(SDL_Event & e)
{
	float sigmaStep = 1.0f;
	if (e.type == SDL_KEYDOWN) {
		if (e.key.keysym.sym == SDLK_s) {
			if (e.key.keysym.mod & KMOD_LSHIFT) {
				++filterSize_;
			} else {
				if (filterSize_ > 3) {
					--filterSize_;
				}
			}
		}
		if (e.key.keysym.sym == SDLK_p) {
			parallel_ = !parallel_;
		}
		if (e.key.keysym.sym == SDLK_c) {
			if (e.key.keysym.mod & KMOD_LSHIFT) {
				colorSigma_ += sigmaStep;
			} else {
				if (colorSigma_ > sigmaStep) {
					colorSigma_ -= sigmaStep;
				}
			}
		}
		if (e.key.keysym.sym == SDLK_d) {
			if (e.key.keysym.mod & KMOD_LSHIFT) {
				depthSigma_ += sigmaStep;
			} else {
				if (depthSigma_ > sigmaStep) {
					depthSigma_ -= sigmaStep;
				}
			}
		}
		std::cout << "Bilateral: size: " << filterSize_ << " parallel: " << parallel_
			<< " cSigma: " << colorSigma_ << " dSigma: " << depthSigma_ << std::endl;
	}
}
