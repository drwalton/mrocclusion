#include "mrocclusion/IterativeRgbdOcclusionMethodHistogram.hpp"

#include "mrocclusion/IterativeRgbdOcclusionMethod-inl.hpp"
#include "mrocclusion/RgbHistogram.hpp"

#include <ceres/ceres.h>
#include <glog/logging.h>

double powInt(double x, int p) {
	double tmp = x;
	for (size_t i = 1; i < p; ++i) {
		x *= tmp;
	}
	return x;
}

double polySigmoid(double x) {
	return (8.0 * powInt(x, 5)) - (20.0 * powInt(x, 4)) + (14.0 * powInt(x, 3)) - powInt(x, 2);
}

size_t max(size_t a, size_t b)
{
	return a > b ? a : b;
}

bool fitFgBgHistograms(
	cv::Mat process, cv::Mat behind, cv::Mat infront,
	cv::Mat realColor,
	RgbHistogram *infrontHist, RgbHistogram *behindHist,
	size_t dilateAmount, size_t minSampleCount)
{
	//Find sample area in infront, behind by dilating process.
	cv::Mat bigProcess;
	cv::dilate(process, bigProcess, cv::Mat());
	for (size_t i = 0; i < dilateAmount - 1; ++i) {
		cv::dilate(bigProcess, bigProcess, cv::Mat());
	}
	cv::Mat infrontSampleLocs = bigProcess & infront;
	cv::Mat behindSampleLocs = bigProcess & behind;

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
	if (infrontHist->nSamples() <= max(minSampleCount, infrontHist->nBins()) || 
		behindHist->nSamples() <= max(minSampleCount, behindHist->nBins())) {
		return false;
	}

	infrontHist->calcProbabilities();
	behindHist->calcProbabilities();

	return true;
}

struct ColorHistogramErrorFunctor
{
public:
	double expectedValue, colorWeight;

	explicit ColorHistogramErrorFunctor(double colorWeight, double infrontProb, double behindProb)
		:expectedValue(/* polySigmoid */(behindProb / (infrontProb + behindProb))),
		colorWeight(colorWeight)
	{}

	template <typename T> bool operator()(
		const T *const valHere, T *residual) const
	{
		*residual = T(colorWeight) * ceres::abs(*valHere - T(expectedValue));
		return true;
	}
};

IterativeRgbdOcclusionMethodHistogram::IterativeRgbdOcclusionMethodHistogram(
	size_t colorWidth, size_t colorHeight, 
	size_t depthWidth, size_t depthHeight)
	:RgbdOcclusionMethod(colorWidth, colorHeight, depthWidth, depthHeight),
	maxIterations_(100),
	printProgress_(false),
	gradientWeight_(1.0),
	colorWeight_(0.5),
	binWidth_(32), dilateAmount_(3), maxNSamples_(100)
{
}

IterativeRgbdOcclusionMethodHistogram::~IterativeRgbdOcclusionMethodHistogram() throw()
{
}

void IterativeRgbdOcclusionMethodHistogram::calculateOcclusion(
	const unsigned char * rgbdColor, const unsigned short * rgbdDepth, 
	const unsigned short * virtualDepth, unsigned char * matte)
{
	const cv::Mat realColor(cv::Size(colorWidth, colorHeight), CV_8UC3, (void*)(rgbdColor));
	const cv::Mat realDepth(cv::Size(depthWidth, depthHeight), CV_16UC1, (void*)(rgbdDepth));
	const cv::Mat virtualDepthMat(cv::Size(depthWidth, depthHeight), CV_16UC1, (void*)(virtualDepth));
	cv::Mat process, behind, infront;
	classifyPixels(realColor, realDepth, virtualDepthMat,
		process, behind, infront, useOffVirtualObject_);
	cv::Mat ignore = ~infront & ~behind & (virtualDepthMat == 0);

	RgbHistogram infrontHist(binWidth_), behindHist(binWidth_);
	fitFgBgHistograms(process, behind, infront, realColor,
		&infrontHist, &behindHist,
		dilateAmount_, 40);

	cv::Mat matteD(realColor.size(), CV_64FC1);
	matteD.setTo(0.f);
	matteD.setTo(0.5f, process);
	matteD.setTo(1.f, behind & (virtualDepthMat != 0));
	cv::Mat sample(1,3,CV_32FC1);

	if (!infrontHist.valid() || !behindHist.valid()) {
		//Failed to fit histograms, fallback.
		cv::Mat matteOut(cv::Size(depthWidth, depthHeight), CV_8UC1, matte);
		matteD.setTo(0.0);
		matteD.setTo(1.0, behind);
		matteD.convertTo(matteOut, CV_8UC1);
		return;
	}

	ceres::Problem problem;

	for (size_t r = 0; r < realDepth.rows; ++r) {
		uchar *procRow = process.ptr<uchar>(r);
		for (size_t c = 0; c < realDepth.cols; ++c) {
			if (procRow[c] != 0) {

				//Add gradient cost functor.
				PixelStatus status[4];
				float value[4];
				size_t r2[] = { r - 1, r, r + 1, r };
				size_t c2[] = { c, c - 1, c, c + 1 };
				for (size_t i = 0; i < 4; ++i) {
					if (process.ptr<uchar>(r2[i])[c2[i]] != 0) {
						status[i] = UNKNOWN;
					}
					else if (infront.ptr<uchar>(r2[i])[c2[i]] != 0) {
						status[i] = FIXED;
						value[i] = 0.f;
					}
					else if (behind.ptr<uchar>(r2[i])[c2[i]] != 0) {
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

				//Get probability under most likely Gaussian in mixture.
				for (size_t i = 0; i < 3; ++i)
					sample.ptr<float>(0)[i] = float(realColor.ptr<cv::Vec3b>(r)[c][i]);
				float infrontProb = infrontHist.probability(sample.data);
				float behindProb = behindHist.probability(sample.data);

				//Add color cost functor.
				ceres::CostFunction *colorCostFunc =
					new ceres::AutoDiffCostFunction
					<ColorHistogramErrorFunctor, 1, 1>(
						new ColorHistogramErrorFunctor(
							colorWeight_,
							infrontProb, behindProb));
				problem.AddResidualBlock(colorCostFunc, NULL,
					matteD.ptr<double>(r) + c);
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
