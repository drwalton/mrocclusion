#include "mrocclusion/GpuCvfOcclusionMethod.hpp"
#include "mrocclusion/IterativeRgbdOcclusionMethodHistogram.hpp"
#include "mrocclusion/guidedfilter.h"
#include <opencv2/ximgproc/edge_filter.hpp>
#include "mrocclusion/SelectiveGuidedFilter.hpp"
#include "mrocclusion/Directories.hpp"
#include <chrono>

GpuCvfOcclusionMethod::GpuCvfOcclusionMethod(
	size_t colorWidth, size_t colorHeight,
	size_t depthWidth, size_t depthHeight)
	:RgbdOcclusionMethod(colorWidth, colorHeight, depthWidth, depthHeight),
	nHistogramBins_(1 << 4),
	dilateAmt_(10),
	costFilterR_(6),
	costFilterEps_(.0025f),
	finalFilterR_(1),
	finalFilterEps_(.3f),
	useOffVirtualObject_(false),
	useSelectiveGuidedFilter_(false),
	maxIterations_(1),
	filter(colorWidth, colorHeight),
	matteTex(GL_TEXTURE_2D, GL_RED, colorWidth, colorHeight, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr),
	colorTex(GL_TEXTURE_2D, GL_RGBA, colorWidth, colorHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr),
	zeroOneTex(GL_TEXTURE_2D, GL_RED, colorWidth, colorHeight, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr),
	snapToZeroOne({
		MROCCLUSION_SHADER_DIR + "guidedfilter/SnapToZeroOne.comp" })
{

}

GpuCvfOcclusionMethod::~GpuCvfOcclusionMethod() throw()
{}

void GpuCvfOcclusionMethod::calculateCosts(
	RgbHistogram &infrontHist, RgbHistogram &behindHist, 
	const cv::Mat &infront, const cv::Mat &behind, const cv::Mat &process,
	const cv::Mat &realColor,
	cv::Mat *costs)
{
	for (size_t r = 0; r < costs->rows; ++r) {
		for (size_t c = 0; c < costs->cols; ++c) {
			if (infront.at<unsigned char>(r, c)) {
				costs->at<float>(r, c) = 0.f;
			} else if (behind.at<unsigned char>(r, c)) {
				costs->at<float>(r, c) = 1.f;
			} else if (process.at<unsigned char>(r, c)) {
				cv::Vec3b color = realColor.at<cv::Vec3b>(r, c);
				float pInfront = infrontHist.probability(&color[0]);
				float pBehind = behindHist.probability(&color[0]);
				float cost;
				if (pInfront + pBehind == 0.f) {
					cost = 0.5f;
				} else {
					cost = 1.f - (pInfront / (pInfront + pBehind));
					if (cost < 0.f || cost > 1.f) {
						throw 1;
					}
				}
				costs->at<float>(r, c) = cost;
			} else {
				costs->at<float>(r, c) = 0.5f;
			}
		}
	}
}

void GpuCvfOcclusionMethod::calculateOcclusion(
	const unsigned char * rgbdColor, const unsigned short * rgbdDepth, 
	const unsigned short * virtualDepth, unsigned char * matte)
{
	calculateOcclusion(rgbdColor, rgbdDepth, virtualDepth, &matteTex);
	cv::Mat matteMat(cv::Size(colorWidth, colorHeight), CV_8UC1, matte);
	matteTex.getData(matteMat.data, colorWidth*colorHeight);
}

void GpuCvfOcclusionMethod::calculateOcclusion(const unsigned char * rgbdColor, const unsigned short * rgbdDepth, const unsigned short * virtualDepth, Texture * matte)
{
	filter.visualise = this->visualise;
	const cv::Mat realColor(cv::Size(colorWidth, colorHeight), CV_8UC3, (void*)(rgbdColor));
	const cv::Mat realDepth(cv::Size(depthWidth, depthHeight), CV_16UC1, (void*)(rgbdDepth));
	const cv::Mat virtualDepthMat(cv::Size(depthWidth, depthHeight), CV_16UC1, (void*)(virtualDepth));
	cv::Mat process, behind, infront;
	classifyPixels(realColor, realDepth, virtualDepthMat,
		process, behind, infront, useOffVirtualObject_);

	if (visualise) {
		cv::Mat classes = makePixelClassesMat(process, behind, infront);
		cv::imshow("Pixel classes", classes);
		cv::waitKey(1);
	}

	RgbHistogram infrontHist(256/(nHistogramBins_>>2)), behindHist(256/(nHistogramBins_>>2));
	//Todo allow histogram fitting, initial binary labeling to be repeated.
	cv::Mat costs(cv::Size(colorWidth, colorHeight), CV_32FC1);
	cv::Mat classMat(cv::Size(colorWidth, colorHeight), CV_8UC1);
	classMat.setTo(128);
	classMat.setTo(255, behind);
	classMat.setTo(0, infront);
	classMat.setTo(0, virtualDepthMat == 0);
	zeroOneTex.update(classMat.data);

	cv::Mat infrontSampleLocs, behindSampleLocs;
	getHistogramSampleLocs(infront, behind, process, &infrontSampleLocs, &behindSampleLocs, dilateAmt_);
	if (debugMode && visualise) {
		cv::imshow("Initial infront locs", infrontSampleLocs);
		cv::imshow("Initial behind locs", behindSampleLocs);
	}

	if (fitFgBgHistograms(realColor, infrontSampleLocs, behindSampleLocs, &infrontHist, &behindHist, 40)) {

		calculateCosts(infrontHist, behindHist, infront, behind, process, realColor, &costs);

		if (visualise) {
			cv::imshow("Initial costs (GPU)", costs);
		}
		colorTex.update(rgbdColor);
		filter.costsToMatte(&colorTex, (float*)(costs.data), &zeroOneTex, matte, costFilterR_, costFilterEps_, finalFilterR_, finalFilterEps_);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		snapToZeroOne.use();
		glBindImageTexture(0, matte->tex(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R8UI);
		glBindImageTexture(1, zeroOneTex.tex(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8UI);
		glDispatchCompute(colorWidth, colorHeight, 1);
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

		if (visualise) {
			cv::Mat matteM(cv::Size(colorWidth, colorHeight), CV_8UC1);
			matte->getData(matteM.data, colorWidth*colorHeight);
			cv::imshow("Final matte", matteM);
			cv::waitKey(1);
		}

	} else {
		cv::Mat matteMat = virtualDepthMat != 0;
		matte->update(matteMat.data);
	}
}

std::string GpuCvfOcclusionMethod::getName() const
{
	return "CVFGPU_" + std::to_string(nHistogramBins_) + "_" + std::to_string(finalFilterR_) + "_" + std::to_string(finalFilterEps_);
}

void GpuCvfOcclusionMethod::processEvent(SDL_Event & e)
{
	const float epsStep = 0.1f;
	if (e.type == SDL_KEYDOWN) {
		if (e.key.keysym.sym == SDLK_e) {
			if (e.key.keysym.mod & KMOD_LSHIFT) {
				finalFilterEps_ += epsStep;
			} else {
				if (finalFilterEps_ > epsStep) {
					finalFilterEps_ -= epsStep;
				}
			}
		}
		if (e.key.keysym.sym == SDLK_r) {
			if (e.key.keysym.mod & KMOD_LSHIFT) {
				++finalFilterR_;
			} else {
				if(finalFilterR_ > 1)
				--finalFilterR_;
			}
		}
		if (e.key.keysym.sym == SDLK_f) {
			if (e.key.keysym.mod & KMOD_LSHIFT) {
				++costFilterR_;
			} else {
				if(costFilterR_ > 1)
				--costFilterR_;
			}
		}
		if (e.key.keysym.sym == SDLK_d) {
			if (e.key.keysym.mod & KMOD_LSHIFT) {
				costFilterEps_ += epsStep;
			} else {
				if (costFilterEps_ > epsStep) {
					costFilterEps_ -= epsStep;
				}
			}
		}
		std::cout << "Cost Volume GPU: cEps: " << costFilterEps_
			<< " cR: " << costFilterR_
			<< " fEps: " << finalFilterEps_
			<< " fR: " << finalFilterR_ << std::endl;
	}
}
