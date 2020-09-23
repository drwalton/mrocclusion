#include "mrocclusion/IterativeRgbdOcclusionMethod.hpp"
#include "mrocclusion/IterativeRgbdOcclusionMethod-inl.hpp"

#include <ceres/ceres.h>

IterativeRgbdOcclusionMethod::IterativeRgbdOcclusionMethod(
	size_t colorWidth, size_t colorHeight, 
	size_t depthWidth, size_t depthHeight)
	:RgbdOcclusionMethod(colorWidth, colorHeight, depthWidth, depthHeight),
	gradientWeight_(1.0),
	colorWeight_(0.5),
	maxIterations_(20),
	printProgress_(false)
{
}

IterativeRgbdOcclusionMethod::~IterativeRgbdOcclusionMethod() throw()
{
}

void IterativeRgbdOcclusionMethod::calculateOcclusion(
	const unsigned char * rgbdColor, const unsigned short * rgbdDepth, 
	const unsigned short * virtualDepth, unsigned char * matte)
{
	const cv::Mat realColor(cv::Size(colorWidth, colorHeight), CV_8UC3, (void*)(rgbdColor));
	const cv::Mat realDepth(cv::Size(depthWidth, depthHeight), CV_16UC1, (void*)(rgbdDepth));
	const cv::Mat virtualDepthMat(cv::Size(depthWidth, depthHeight), CV_16UC1, (void*)(virtualDepth));
	cv::Mat process, behind, infront;
	classifyPixels(realColor, realDepth, virtualDepthMat,
		process, behind, infront, useOffVirtualObject_);
	cv::Mat ignore = ~infront & ~behind & (virtualDepth == 0);

	cv::Mat matteD(realColor.size(), CV_64FC1);
	matteD.setTo(0.f);
	matteD.setTo(0.5f, process);
	matteD.setTo(1.f, behind & (virtualDepth != 0));

	ceres::Problem problem;

	for (size_t r = 0; r < depthHeight; ++r) {
		for (size_t c = 0; c < depthWidth; ++c) {
			if (process.at<uchar>(r, c) != 0) {
				PixelStatus status[4];
				float value[4];
				size_t r2[] = { r - 1, r, r + 1, r };
				size_t c2[] = { c, c - 1, c, c + 1 };
				for (size_t i = 0; i < 4; ++i) {
					if (process.at<uchar>(r2[i], c2[i]) != 0) {
						status[i] = UNKNOWN;
					}
					else if (infront.at<uchar>(r2[i], c2[i]) != 0) {
						status[i] = FIXED;
						value[i] = 0.f;
					}
					else if (behind.at<uchar>(r2[i], c2[i]) != 0) {
						status[i] = FIXED;
						value[i] = 1.f;
					}
					else {
						status[i] = MISSING;
					}
				}
				ceres::CostFunction *costFunc =
					new ceres::AutoDiffCostFunction
					<GradientCostFunctor, 1, 1, 1, 1, 1, 1>(
						new GradientCostFunctor(
							gradientWeight_, status, value));
				problem.AddResidualBlock(costFunc, NULL,
					matteD.ptr<double>(r) + c,
					matteD.ptr<double>(r - 1) + c,
					matteD.ptr<double>(r) + c - 1,
					matteD.ptr<double>(r + 1) + c,
					matteD.ptr<double>(r) + c + 1);

				ceres::CostFunction *colorCostFunc =
					new ceres::AutoDiffCostFunction
					<ColorSimilarityCostFunctor, 1, 1, 1, 1, 1, 1>(
						new ColorSimilarityCostFunctor(
							colorWeight_,
							realColor.ptr<uchar>(r) + c * 3,
							realColor.ptr<uchar>(r - 1) + c * 3,
							realColor.ptr<uchar>(r) + (c - 1) * 3,
							realColor.ptr<uchar>(r + 1) + c * 3,
							realColor.ptr<uchar>(r) + (c + 1) * 3,
							status));
				problem.AddResidualBlock(colorCostFunc, NULL,
					matteD.ptr<double>(r) + c,
					matteD.ptr<double>(r - 1) + c,
					matteD.ptr<double>(r) + c - 1,
					matteD.ptr<double>(r + 1) + c,
					matteD.ptr<double>(r) + c + 1);
			}
		}
	}

	ceres::Solver::Options options;
	options.max_num_iterations = maxIterations_;
	options.minimizer_progress_to_stdout = printProgress_;
	ceres::Solver::Summary summary;
	ceres::Solve(options, &problem, &summary);

	bool showSummary = true;
	if (showSummary) {
		std::cout << summary.FullReport() << std::endl;
	}

	matteD *= 255.f;
	cv::Mat matteOut(cv::Size(depthWidth, depthHeight), CV_8UC1, matte);
	matteD.convertTo(matteOut, CV_8UC1);
}
