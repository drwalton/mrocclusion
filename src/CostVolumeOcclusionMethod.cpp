#include "mrocclusion/CostVolumeOcclusionMethod.hpp"
#include "mrocclusion/IterativeRgbdOcclusionMethodHistogram.hpp"
#include "mrocclusion/guidedfilter.h"
#include <opencv2/ximgproc/edge_filter.hpp>
#include "mrocclusion/SelectiveGuidedFilter.hpp"
#include <chrono>

CostVolumeOcclusionMethod::CostVolumeOcclusionMethod(
	size_t colorWidth, size_t colorHeight,
	size_t depthWidth, size_t depthHeight)
	:RgbdOcclusionMethod(colorWidth, colorHeight, depthWidth, depthHeight),
	nHistogramBins_(1<<4),
	dilateAmt_(5),
	costFilterR_(9),
	costFilterEps_(80.f),
	finalFilterR_(1),
	finalFilterEps_(300.f),
	useOffVirtualObject_(false),
	useSelectiveGuidedFilter_(false),
	maxIterations_(1),
	costFilterType_(CostFilterType::BILATERAL)

{
}

CostVolumeOcclusionMethod::~CostVolumeOcclusionMethod() throw()
{}

void CostVolumeOcclusionMethod::calculateCosts(
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

void CostVolumeOcclusionMethod::filterCosts(
	const cv::Mat & costs, 
	const cv::Mat & infront, const cv::Mat & behind, const cv::Mat & process, 
	const cv::Mat &realColor,
	cv::Mat * filteredCosts)
{
	if(costFilterType_ == CostFilterType::SELECTIVE_GUIDED) {
		cv::Mat useMask = process | infront | behind;
		selectiveGuidedFilter(costs, realColor, process, useMask, filteredCosts,
			costFilterR_, costFilterEps_);
	} else if (costFilterType_ == CostFilterType::GUIDED) {
		auto f = cv::ximgproc::createGuidedFilter(realColor, costFilterR_, costFilterEps_);
		f->filter(costs, *filteredCosts);
	} else /* CostFilterType::BILATERAL */ {
		cv::Mat realColorFloat;
		realColor.convertTo(realColorFloat, CV_32F);
		cv::ximgproc::jointBilateralFilter(realColorFloat, costs, *filteredCosts, costFilterR_ * 2, 15.f, 12.f);
	}
}

void CostVolumeOcclusionMethod::thresholdCosts(const cv::Mat & filteredCosts, const cv::Mat & infront, const cv::Mat & behind, const cv::Mat & process, cv::Mat * binaryMask)
{
	for (size_t r = 0; r < filteredCosts.rows; ++r) {
		for (size_t c = 0; c < filteredCosts.cols; ++c) {
			if (infront.at<unsigned char>(r, c)) {
				binaryMask->at<unsigned char>(r, c) = 0;
			} else if (behind.at<unsigned char>(r, c)) {
				binaryMask->at<unsigned char>(r, c) = 255;
			} else if (process.at<unsigned char>(r, c)) {
				float cost = filteredCosts.at<float>(r, c);
				if (cost < 0.f || cost > 1.f) {
					//std::cout << "Warning: Filtered cost out of range" << std::endl;
				}
				binaryMask->at<unsigned char>(r, c) = ((filteredCosts.at<float>(r,c) > 0.5f) ? 255 : 0);
			} else {
				binaryMask->at<unsigned char>(r, c) = 0;
			}
		}
	}
}

void getHistogramSampleLocs(
	const cv::Mat &infront, const cv::Mat &behind, const cv::Mat &process,
	cv::Mat *infrontSampleLocs, cv::Mat *behindSampleLocs, size_t dilateAmount)
{
	cv::Mat bigProcess;
	cv::dilate(process, bigProcess, cv::Mat());
	for (size_t i = 0; i < dilateAmount - 1; ++i) {
		cv::dilate(bigProcess, bigProcess, cv::Mat());
	}
	*infrontSampleLocs = bigProcess & infront;
	*behindSampleLocs = bigProcess & behind;
}

bool fitFgBgHistograms(
	const cv::Mat &realColor,
	const cv::Mat &infrontSampleLocs, const cv::Mat &behindSampleLocs,
	RgbHistogram *infrontHist, RgbHistogram *behindHist,
	size_t minSampleCount)
{
	for (size_t r = 0; r < realColor.rows; ++r) {
		for (size_t c = 0; c < realColor.cols; ++c) {
			if (infrontSampleLocs.at<uchar>(r, c)) {
				infrontHist->addRgbVal(realColor.ptr<unsigned char>(r) + c*3);
			}
			if (behindSampleLocs.at<uchar>(r, c)) {
				behindHist->addRgbVal(realColor.ptr<unsigned char>(r) + c*3);
			}
		}
	}


	//Failsafe if no known fg/bg pixels.
	if (infrontHist->nSamples() <= std::max(minSampleCount, infrontHist->nBins()) || 
		behindHist->nSamples() <= std::max(minSampleCount, behindHist->nBins())) {
		return false;
	}

	infrontHist->calcProbabilities();
	behindHist->calcProbabilities();

	return true;
}

void CostVolumeOcclusionMethod::calculateOcclusion(
	const unsigned char * rgbdColor, const unsigned short * rgbdDepth, 
	const unsigned short * virtualDepth, unsigned char * matte)
{
	const cv::Mat realColor(cv::Size(colorWidth, colorHeight), CV_8UC3, (void*)(rgbdColor));
	const cv::Mat realDepth(cv::Size(depthWidth, depthHeight), CV_16UC1, (void*)(rgbdDepth));
	const cv::Mat virtualDepthMat(cv::Size(depthWidth, depthHeight), CV_16UC1, (void*)(virtualDepth));
	cv::Mat process, behind, infront;
	classifyPixels(realColor, realDepth, virtualDepthMat,
		process, behind, infront, useOffVirtualObject_);
	RgbHistogram infrontHist(256/(nHistogramBins_>>2)), behindHist(256/(nHistogramBins_>>2));
	//Todo allow histogram fitting, initial binary labeling to be repeated.
	cv::Mat costs(cv::Size(colorWidth, colorHeight), CV_32FC1);
	cv::Mat binaryMask(cv::Size(colorWidth, colorHeight), CV_8UC1);

	auto t = std::chrono::system_clock::now();
	cv::Mat infrontSampleLocs, behindSampleLocs;
	getHistogramSampleLocs(infront, behind, process, &infrontSampleLocs, &behindSampleLocs, dilateAmt_);
	if (debugMode) {
		cv::imshow("Initial infront locs", infrontSampleLocs);
		cv::imshow("Initial behind locs", behindSampleLocs);
	}

	if (fitFgBgHistograms(realColor, infrontSampleLocs, behindSampleLocs, &infrontHist, &behindHist, 40)) {
		auto d = std::chrono::system_clock::now() - t;
		if (debugMode) {
			std::cout << "Fit hist time " << std::chrono::duration_cast<std::chrono::milliseconds>(d).count() << "ms" << std::endl;
		}

		t = std::chrono::system_clock::now();
		calculateCosts(infrontHist, behindHist, infront, behind, process, realColor, &costs);
		if (debugMode) {
			d = std::chrono::system_clock::now() - t;
			std::cout << "calc cost time " << std::chrono::duration_cast<std::chrono::milliseconds>(d).count() << "ms" << std::endl;
			cv::imshow("Initial cost matrix", costs);
			double min, max;
			cv::minMaxLoc(costs, &min, &max);
			std::cout << "Initial costs min, max " << min << ", " << max << " sum " << cv::sum(costs)[0] << std::endl;
		}

		t = std::chrono::system_clock::now();
		cv::Mat filteredCosts;
		filterCosts(costs, infront, behind, process, realColor, &filteredCosts);
		if (debugMode) {
			d = std::chrono::system_clock::now() - t;
			std::cout << "Filter time " << std::chrono::duration_cast<std::chrono::milliseconds>(d).count() << "ms" << std::endl;
			cv::imshow("Filtered cost matrix", filteredCosts);
		}

		thresholdCosts(filteredCosts, infront, behind, process, &binaryMask);
		if (debugMode) {
			cv::imshow("Thresholded costs (binary mask)", binaryMask);
		}

		for (size_t i = 1; i < maxIterations_; ++i) {
			//Revise sample locations: Add in new infront, behind pixels.
			cv::Mat newBehindSampleLocs, newInfrontSampleLocs;
			if (debugMode) {
				cv::imshow("infront locs before", infrontSampleLocs);
			}
			newBehindSampleLocs = behindSampleLocs | (binaryMask & process);
			newInfrontSampleLocs = infrontSampleLocs | (~binaryMask & process);
			if (debugMode) {
				cv::imshow("New infront locs", infrontSampleLocs);
				cv::imshow("New behind locs", behindSampleLocs);
			}
			fitFgBgHistograms(realColor, newInfrontSampleLocs, newBehindSampleLocs,
				&infrontHist, &behindHist, 40);
			calculateCosts(infrontHist, behindHist, infront, behind, process, realColor, &costs);
			if (debugMode) {
				cv::imshow("Iterated cost matrix", costs);
			}
			filterCosts(costs, infront, behind, process, realColor, &filteredCosts);
			if (debugMode) {
				cv::imshow("Iterated filtered cost matrix", filteredCosts);
			}
			thresholdCosts(filteredCosts, infront, behind, process, &binaryMask);
			if (debugMode) {
				cv::imshow("Thresholded iterated", binaryMask);
				cv::waitKey(1);
			}
		}

		//Perform final filtering step to feather edges.
		t = std::chrono::system_clock::now();
		auto f = cv::ximgproc::createGuidedFilter(realColor, finalFilterR_, finalFilterEps_);
		cv::Mat output; 
		f->filter(binaryMask, output);
		if (debugMode) {
			d = std::chrono::system_clock::now() - t;
			std::cout << "Final filter time " << std::chrono::duration_cast<std::chrono::milliseconds>(d).count() << "ms" << std::endl;
		}
		for (size_t r = 0; r < costs.rows; ++r) {
			for (size_t c = 0; c < costs.cols; ++c) {
				if (infront.at<unsigned char>(r, c)) {
					output.at<unsigned char>(r, c) = 0;
				} else if (behind.at<unsigned char>(r, c)) {
					output.at<unsigned char>(r, c) = 255;
				} else if (virtualDepthMat.at<unsigned short>(r,c) == 0) {
					output.at<unsigned char>(r, c) = 0;
				}
			}
		}

		cv::Mat matteMat(cv::Size(output.cols, output.rows), CV_8UC1, matte);
		output.copyTo(matteMat);
		
		if (debugMode) {
			cv::imshow("Final matte", matteMat);
			cv::waitKey(1);
		}
	} else {
		cv::Mat matteMat(cv::Size(colorWidth, colorHeight), CV_8UC1, matte);
		matteMat = virtualDepthMat != 0.f;
	}
}

std::string CostVolumeOcclusionMethod::getName() const
{
	if (costFilterType_ == CostFilterType::BILATERAL) {
		return "Cost Filter (Bilateral)";
	} else if (costFilterType_ == CostFilterType::GUIDED) {
		return "Cost (G) C" + std::to_string(costFilterR_) + " " + std::to_string(costFilterEps_) + " F " + std::to_string(finalFilterR_) + " " + std::to_string(finalFilterEps_);
	} else if (costFilterType_ == CostFilterType::SELECTIVE_GUIDED) {
		return "Cost Filter (Selective Guided)";
	}
	throw std::runtime_error("Cost filter type not added to getName()");
}

void CostVolumeOcclusionMethod::processEvent(SDL_Event & e)
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
		std::cout << "Cost Volume: cEps: " << costFilterEps_
			<< " cR: " << costFilterR_
			<< " fEps: " << finalFilterEps_
			<< " fR: " << finalFilterR_ << std::endl;
	}
}

CostVolumeOcclusionMethod::CostFilterType CostVolumeOcclusionMethod::costFilterType() const
{
	return costFilterType_;
}

void CostVolumeOcclusionMethod::costFilterType(CostFilterType t)
{
	costFilterType_ = t;
}
