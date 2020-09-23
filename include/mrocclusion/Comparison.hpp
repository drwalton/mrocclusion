#ifndef MROCCLUSION_COMPARISON_HPP_INCLUDED
#define MROCCLUSION_COMPARISON_HPP_INCLUDED

#include <vector>
#include <memory>
#include <boost/property_tree/ptree.hpp>
#include <opencv2/opencv.hpp>

class ErrorMetric;
class RgbdOcclusionMethod;

//!This file contains methods common to CompareOcclusionMethods,
//! CompareOcclusionMethodsInfinitam and RecordOcclusionMethodFootage.
void readAnnotationFile(const std::string &filename,
	std::vector<size_t> *compareFrames,
	std::vector<size_t> *gtFrames,
	std::vector<size_t> *kinfuFrames);
	
void loadErrorMetrics(std::vector<std::unique_ptr<ErrorMetric> > *errorMetrics);

void loadOcclusionMethods(boost::property_tree::ptree &config,
	std::vector<std::unique_ptr<RgbdOcclusionMethod> > *occlusionMethods,
	size_t colorWidth, size_t colorHeight, size_t depthWidth, size_t depthHeight);

float temporalNoiseMetric(const std::vector<cv::Mat> mattes);

#endif
