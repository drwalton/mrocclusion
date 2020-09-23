#include "mrocclusion/Comparison.hpp"

#include <fstream>
#include "mrocclusion/ErrorMetrics.hpp"
#include "mrocclusion/RgbdOcclusionMethod.hpp"
#include "boost/foreach.hpp"
#include <iostream>

void readAnnotationFile(const std::string &filename,
	std::vector<size_t> *compareFrames,
	std::vector<size_t> *gtFrames,
	std::vector<size_t> *kinfuFrames)
{
	std::ifstream txtFile(filename);
	char currChar;
	size_t currIdx;
	
	while(txtFile >> currChar) {
		txtFile >> currIdx;
		if(currChar != 'K') {
			throw std::runtime_error("Error in file - should have 'K'"
			" to start kinfu");
		}
		kinfuFrames->push_back(currIdx);
		size_t endIdx;
		txtFile >> currChar;
		if(currChar != 'C') {
			throw std::runtime_error("Error in file - should have 'C'"
			" to start capture");
		}
		txtFile >> currIdx;
		txtFile >> currChar;
		if(currChar != 'E') {
			throw std::runtime_error("Error in file - should have 'E'"
			" to end capture");
		}
		txtFile >> endIdx;
		for(size_t i = currIdx+1; i < endIdx; ++i) {
			compareFrames->push_back(i);
		}
		txtFile >> currChar;
		if(currChar != 'G') {
			throw std::runtime_error("Error in file - should have 'G'"
			" to indicate ground truth.");
		}
		txtFile >> currIdx;
		gtFrames->push_back(currIdx);
	}
}

void loadErrorMetrics(std::vector<std::unique_ptr<ErrorMetric> > *errorMetrics)
{
	//errorMetrics->emplace_back(new SadErrorMetric());
	errorMetrics->emplace_back(new MseErrorMetric());
	//errorMetrics->emplace_back(new GradientErrorMetric());
}

float temporalNoiseMetric(const std::vector<cv::Mat> mattes)
{
	cv::Mat std(mattes[0].rows, mattes[0].cols, CV_32FC1);
	std::vector<float> vals(mattes.size());
	for (size_t r = 0; r < std.rows; ++r) {
		for (size_t c = 0; c < std.cols; ++c) {
			for (size_t i = 0; i < mattes.size(); ++i) {
				vals[i] = static_cast<float>(mattes[i].at<unsigned char>(r, c));
			}
			float mean = 0.f;
			for (float f : vals) {
				mean += f;
			}
			mean /= vals.size();
			float s = 0.f;
			for (float f : vals) {
				s += std::abs(f - mean);
			}
			s /= vals.size();
			std.at<float>(r, c) = s;
		}
	}
	return cv::mean(std)[0];
}


void loadOcclusionMethods(boost::property_tree::ptree &config,
	std::vector<std::unique_ptr<RgbdOcclusionMethod> > *occlusionMethods,
	size_t colorWidth, size_t colorHeight, size_t depthWidth, size_t depthHeight)
{
	std::cout << "Loading occlusion methods..." << std::endl;
	BOOST_FOREACH(
		const boost::property_tree::ptree::value_type &method_v,
		config.get_child("occlusionMethods")) {
			occlusionMethods->push_back(
				RgbdOcclusionMethod::factory(method_v.second,
					colorWidth, colorHeight, depthWidth, depthHeight));
	}
	for(size_t i = 0; i < occlusionMethods->size(); ++i) {
		std::cout << "\t> " << (*occlusionMethods)[i]->getName() << std::endl;
	}
	std::cout << std::endl;
}
