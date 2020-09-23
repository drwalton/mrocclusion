#include "mrocclusion/IterativeRgbdOcclusionMethodMOG.hpp"
#include "mrocclusion/IterativeRgbdOcclusionMethod-inl.hpp"
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

void fitFgBgMoGs(
	cv::Mat process, cv::Mat behind, cv::Mat infront,
	cv::Mat realColor,
	cv::Ptr<cv::ml::EM> &infrontMoG, cv::Ptr<cv::ml::EM> &behindMoG,
	size_t nGaussians, size_t dilateAmount, size_t maxNoSamples)
{
	//Find sample area in infront, behind by dilating process.
	cv::Mat bigProcess;
	cv::dilate(process, bigProcess, cv::Mat());
	for (size_t i = 0; i < dilateAmount - 1; ++i) {
		cv::dilate(bigProcess, bigProcess, cv::Mat());
	}
	cv::Mat infrontSampleLocs = bigProcess & infront;
	cv::Mat behindSampleLocs = bigProcess & behind;

	//Count no. of fg, bg samples.
	size_t nInfrontSamples = 0, nBehindSamples = 0;
	for (size_t r = 0; r < realColor.rows; ++r) {
		for (size_t c = 0; c < realColor.cols; ++c) {
			if (infrontSampleLocs.at<uchar>(r, c)) {
				++nInfrontSamples;
			}
			if (behindSampleLocs.at<uchar>(r, c)) {
				++nBehindSamples;
			}
		}
	}


	//Failsafe if no known fg/bg pixels.
	if (nInfrontSamples <= nGaussians || nBehindSamples <= nGaussians) {
		infrontMoG = (cv::ml::EM*)nullptr;
		behindMoG = (cv::ml::EM*)nullptr;
		return;
	}

	//List rows, columns of fg, bg samples.
	std::vector<size_t> infrontSampleR(nInfrontSamples),
		infrontSampleC(nInfrontSamples),
		behindSampleR(nBehindSamples),
		behindSampleC(nBehindSamples);

	size_t infrontSampleCtr = 0, behindSampleCtr = 0;
	for (size_t r = 0; r < realColor.rows; ++r) {
		for (size_t c = 0; c < realColor.cols; ++c) {
			if (infrontSampleLocs.at<uchar>(r, c)) {
				infrontSampleR[infrontSampleCtr] = r;
				infrontSampleC[infrontSampleCtr] = c;
				++infrontSampleCtr;
			}
			if (behindSampleLocs.at<uchar>(r, c)) {
				behindSampleR[behindSampleCtr] = r;
				behindSampleC[behindSampleCtr] = c;
				++behindSampleCtr;
			}
		}
	}
	infrontSampleCtr = behindSampleCtr = 0;

	nInfrontSamples = std::min(nInfrontSamples, maxNoSamples);
	nBehindSamples = std::min(nBehindSamples, maxNoSamples);

	std::vector<size_t> infrontSampleI(std::min(infrontSampleR.size(), maxNoSamples)),
		behindSampleI(std::min(behindSampleC.size(), maxNoSamples));
	std::iota(infrontSampleI.begin(), infrontSampleI.end(), 0);
	std::iota(behindSampleI.begin(), behindSampleI.end(), 0);

	if (infrontSampleR.size() <= maxNoSamples) {
		std::random_shuffle(infrontSampleI.begin(), infrontSampleI.end());
	}
	if (behindSampleR.size() <= maxNoSamples) {
		std::random_shuffle(behindSampleI.begin(), behindSampleI.end());
	}

	//Create lists of fg, bg sample rows and columns.
	cv::Mat infrontSamples(nInfrontSamples, 3, CV_32FC1),
		behindSamples(nBehindSamples, 3, CV_32FC1);

	for (size_t i = 0; i < nInfrontSamples; ++i) {
		cv::Vec3b sample = realColor.at<cv::Vec3b>(
			infrontSampleR[infrontSampleI[i]], infrontSampleC[infrontSampleI[i]]);
		for (size_t j = 0; j < 3; ++j)
			infrontSamples.at<float>(i, j) = float(sample[j]);
	}
	for (size_t i = 0; i < nBehindSamples; ++i) {
		cv::Vec3b sample = realColor.at<cv::Vec3b>(
			behindSampleR[behindSampleI[i]], behindSampleC[behindSampleI[i]]);
		for (size_t j = 0; j < 3; ++j)
			behindSamples.at<float>(i, j) = float(sample[j]);
	}

	//Create and train EMs.
	infrontMoG = cv::ml::EM::create();
	infrontMoG->setClustersNumber(nGaussians);
	behindMoG = cv::ml::EM::create();
	behindMoG->setClustersNumber(nGaussians);

	infrontMoG->trainEM(infrontSamples);
	behindMoG->trainEM(behindSamples);
}

struct ColorMoGErrorFunctor
{
public:
	double expectedValue, colorWeight;

	explicit ColorMoGErrorFunctor(double colorWeight, double infrontProb, double behindProb)
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

IterativeRgbdOcclusionMethodMOG::IterativeRgbdOcclusionMethodMOG(
	size_t colorWidth, size_t colorHeight, 
	size_t depthWidth, size_t depthHeight)
	:RgbdOcclusionMethod(colorWidth, colorHeight, depthWidth, depthHeight),
	maxIterations_(20),
	printProgress_(false),
	gradientWeight_(1.0),
	colorWeight_(0.5),
	nGaussians_(5), dilateAmount_(3), maxNSamples_(100)
{
}

IterativeRgbdOcclusionMethodMOG::~IterativeRgbdOcclusionMethodMOG() throw()
{
}

void IterativeRgbdOcclusionMethodMOG::calculateOcclusion(
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

	cv::Ptr<cv::ml::EM> infrontMoG, behindMoG;
	infrontMoG = behindMoG = (cv::ml::EM*)nullptr;
	fitFgBgMoGs(process, behind, infront, realColor,
		infrontMoG, behindMoG,
		nGaussians_, dilateAmount_, maxNSamples_);

	cv::Mat matteD(realColor.size(), CV_64FC1);
	matteD.setTo(0.f);
	matteD.setTo(0.5f, process);
	matteD.setTo(1.f, behind & (virtualDepth != 0));
	cv::Mat sample(1,3,CV_32FC1);

	if (!infrontMoG.get() || !behindMoG.get()) {
		//Failed to fit MoG models, fallback.
		cv::Mat matteOut(cv::Size(depthWidth, depthHeight), CV_8UC1, matte);
		matteD.setTo(0);
		matteD.setTo(behind, 255);
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
				cv::Mat infrontProbs, behindProbs;
				infrontMoG->predict2(sample, infrontProbs);
				behindMoG->predict2(sample, behindProbs);

				double min, infrontP, behindP;
				cv::minMaxLoc(behindProbs, &min, &behindP);
				cv::minMaxLoc(infrontProbs, &min, &infrontP);

				//Add color cost functor.
				ceres::CostFunction *colorCostFunc =
					new ceres::AutoDiffCostFunction
					<ColorMoGErrorFunctor, 1, 1>(
						new ColorMoGErrorFunctor(
							colorWeight_,
							infrontP, behindP));
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
