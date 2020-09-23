#include "mrocclusion/ErrorMetrics.hpp"
#include "mrocclusion/KahanVal.hpp"
#include <opencv2/opencv.hpp>

ErrorMetric::ErrorMetric()
{}

ErrorMetric::~ErrorMetric() throw()
{}

SadErrorMetric::SadErrorMetric()
{}

SadErrorMetric::~SadErrorMetric() throw()
{}

double SadErrorMetric::error(
	unsigned char *matte, unsigned char *groundTruth,
	size_t width, size_t height) const
{
	KahanVal<double> sum;
	for(size_t i = 0; i < width*height; ++i) {
		double matteVal = double(matte[i]) / 255.0;
		double gtVal = double(groundTruth[i]) / 255.0;
		double diff = std::abs(matteVal - gtVal);
		sum += diff;
	}
	return sum.get();
}

std::string SadErrorMetric::name() const
{
	return "SAD";
}

MseErrorMetric::MseErrorMetric()
{}

MseErrorMetric::~MseErrorMetric() throw()
{}

double MseErrorMetric::error(
	unsigned char *matte, unsigned char *groundTruth,
	size_t width, size_t height) const
{
	SadErrorMetric sad;
	return sad.error(
		matte, groundTruth, width, height) / double(width*height);
}

std::string MseErrorMetric::name() const
{
	return "MSE";
}

GradientErrorMetric::GradientErrorMetric()
{}

GradientErrorMetric::~GradientErrorMetric() throw()
{}

void normalizedGradient(const cv::Mat im_, cv::Mat &g_x, cv::Mat &g_y)
{
	static const int kernelSize = 3;
	cv::Mat gradient_x, gradient_y;
	cv::Mat im;
	im_.convertTo(im, CV_32FC1);
	cv::Sobel(im, gradient_x, CV_32F, 1, 0, kernelSize);
	cv::Sobel(im, gradient_y, CV_32F, 0, 1, kernelSize);
	cv::Mat len;
	cv::magnitude(gradient_x, gradient_y, len);
	cv::divide(gradient_x, len, g_x);
	cv::divide(gradient_y, len, g_y);
}

double GradientErrorMetric::error(
	unsigned char *matte, unsigned char *groundTruth,
	size_t width, size_t height) const
{
	cv::Mat matteM(height, width, CV_8UC1, matte);
	cv::Mat gtM(height, width, CV_8UC1, groundTruth);
		cv::Mat groundTruthG_x, groundTruthG_y;
		cv::Mat matteG_x, matteG_y;
		normalizedGradient(gtM, groundTruthG_x, groundTruthG_y);
		normalizedGradient(matteM, matteG_x, matteG_y);
		cv::Mat diffG_x, diffG_y;
		cv::absdiff(groundTruthG_x, matteG_x, diffG_x);
		cv::absdiff(groundTruthG_y, matteG_y, diffG_y);
		cv::Mat len;
		cv::magnitude(diffG_x, diffG_y, len);
		return cv::sum(len)[0];
}

std::string GradientErrorMetric::name() const
{
	return "Gradient";
}
