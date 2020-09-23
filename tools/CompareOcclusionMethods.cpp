#include <SDL.h>
#include "mrocclusion/OpenNiDepthCam.hpp"
#include "mrocclusion/Comparison.hpp"
#include "mrocclusion/RgbdOcclusionMethod.hpp"
#include "mrocclusion/ErrorMetrics.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/foreach.hpp"
#include <fstream>
#include "mrocclusion/Files.hpp"
#include "mrocclusion/GLWindow.hpp"
#include "mrocclusion/Directories.hpp"

cv::Mat composeAugmentedImage(const cv::Mat &virtualColor, const cv::Mat &realColor, const cv::Mat &matte)
{
	cv::Mat output(virtualColor.size(), CV_8UC3);
	for (size_t r = 0; r < output.rows; ++r) {
		for (size_t c = 0; c < output.cols; ++c) {
			cv::Vec3b virt = virtualColor.at<cv::Vec3b>(r, c);
			cv::Vec3b real = realColor.at<cv::Vec3b>(r, c);
			unsigned char m = matte.at<unsigned char>(r, c);
			float mVal = (float)(m) / 255.f;
			cv::Vec3b out = mVal * virt + (1.f - mVal) * real;
			output.at<cv::Vec3b>(r, c) = out;
		}
	}
	cv::Mat outputBgr;
	cv::cvtColor(output, outputBgr, cv::COLOR_BGR2RGB);
	return outputBgr;
}

int main(int argc, char *argv[])
{
	if(argc <= 1) {
		std::cout << "Usage: $ CompareOcclusionMethods [config]" << std::endl;
		return 1;
	}

	GLWindow win("hidden", 0, 0);
	
	boost::property_tree::ptree config;
	boost::property_tree::json_parser::read_json(argv[1], config);
	
	std::string baseFilename = config.get<std::string>("baseFilename");
	std::string vidFile = baseFilename + ".oni";
	std::vector<size_t> compareFrames;
	std::vector<size_t> gtFrames;
	std::vector<size_t> kinfuFrames;
	readAnnotationFile(baseFilename + ".txt", &compareFrames, &gtFrames, &kinfuFrames);
	
	std::ofstream logFile(
		MROCCLUSION_RESULTS_DIR + "OcclusionComparison_" +
		uniqueTimestamp() + ".csv");
	
	logFile << "Video file," << vidFile << "\n";
	
	OpenNiDepthCam video(vidFile);
	std::vector<std::unique_ptr<RgbdOcclusionMethod> > occlusionMethods;
	loadOcclusionMethods(config, &occlusionMethods,
		video.colorWidth(), video.colorHeight(),
		video.depthWidth(), video.depthHeight());
	
	std::vector<std::unique_ptr<ErrorMetric> > errorMetrics;
	loadErrorMetrics(&errorMetrics);
	
	//Add titles to table (2x rows).
	//First row: frame no, and names of each method.
	logFile << "Frame No., ";
	for(auto &method : occlusionMethods) {
		logFile << method->getName() << ", ";
		for(size_t i = 1; i < errorMetrics.size(); ++i) {
			logFile << ", ";
		}
	}
	//Second row: names of each error metric.
	logFile << "\n, ";
	for(auto &method : occlusionMethods) {
		for(auto & metric : errorMetrics) {
			logFile << metric->name() << ", ";
		}
	}
	logFile << "\n";
	
	cv::Mat virtualDepth, virtualColor, groundTruthMatte, groundTruthMatte3;
	cv::Mat matte(video.colorHeight(), video.colorWidth(), CV_8UC1);
	
	//Compare occlusion methods using per-frame error metrics.

	std::cout << "Testing per-frame error metrics..." << std::endl;

	size_t gtIdx = 0;
	groundTruthMatte3 = cv::imread(baseFilename + "_gtMatte" +
		std::to_string(gtFrames[gtIdx]) + ".png");
	cv::cvtColor(groundTruthMatte3, groundTruthMatte, cv::COLOR_RGB2GRAY);
	for(size_t i = 0; i < compareFrames.size(); ++i) {
		if(i > 0 && compareFrames[i] != compareFrames[i-1]+1) {
			++gtIdx;
        	groundTruthMatte3 = cv::imread(baseFilename + "_gtMatte" +
        		std::to_string(gtFrames[gtIdx]) + ".png");
			cv::cvtColor(groundTruthMatte3, groundTruthMatte, cv::COLOR_RGB2GRAY);
		}
		logFile << compareFrames[i] << ", ";
		video.seek(compareFrames[i]);
		video.getLatestFrames();
		cv::Mat colorMat(cv::Size(video.colorWidth(), video.colorHeight()), CV_8UC3, (void*)video.colorFramePtr());
		cv::imshow("Input color", colorMat);
		cv::waitKey(1);
		for(auto &method : occlusionMethods) {
    		virtualColor = cv::imread(baseFilename + "_glColor" +
    			std::to_string(compareFrames[i]) + ".png");
    		virtualDepth = cv::imread(baseFilename + "_glDepth" +
    			std::to_string(compareFrames[i]) + ".png", cv::IMREAD_ANYDEPTH);
			method->calculateOcclusion(video.colorFramePtr(),
				video.depthFramePtr(), (unsigned short*)(virtualDepth.data), matte.data);
			for(auto &metric : errorMetrics) {
				
				cv::imshow("matte", matte);
				cv::imshow("gt", groundTruthMatte);
				cv::imshow("diff", matte - groundTruthMatte);
				cv::waitKey(1);
				//cv::imshow("diff", matte & ~groundTruthMatte);
				//cv::waitKey();
				double err = metric->error(
					matte.data, groundTruthMatte.data,
					matte.cols, matte.rows);
				logFile << err << ", ";
				cv::Mat realColor(cv::Size(video.colorWidth(), video.colorHeight()), CV_8UC3, (void*)video.colorFramePtr());
				cv::Mat augmentedIm = composeAugmentedImage(virtualColor, realColor, matte);
				cv::imwrite(method->getName() + std::to_string(compareFrames[i]) + ".png", augmentedIm);
				augmentedIm = composeAugmentedImage(virtualColor, realColor, groundTruthMatte);
				cv::imwrite("GroundTruth" + std::to_string(compareFrames[i]) + ".png", augmentedIm);
			}
		}
		logFile << "\n";
		std::cout << "\tFrame " << compareFrames[i] << " completed..." << std::endl;
	}

	logFile << "\nTemporal noise\n";
	
	std::cout << "Finding temporal noise..." << std::endl;


	for (auto &method : occlusionMethods) {
		logFile << method->getName() << ", ";
		std::vector<cv::Mat> frames;
		for (size_t i = 0; i < compareFrames.size(); ++i) {
			if (i > 0 && compareFrames[i] != compareFrames[i - 1] + 1) {
				//Find noise metric.
				float temporalNoise = temporalNoiseMetric(frames);
				logFile << temporalNoise << ", ";
				frames.clear();
			}
			video.seek(compareFrames[i]);
			video.getLatestFrames();
    		virtualColor = cv::imread(baseFilename + "_glColor" +
    			std::to_string(compareFrames[i]) + ".png");
    		virtualDepth = cv::imread(baseFilename + "_glDepth" +
    			std::to_string(compareFrames[i]) + ".png", cv::IMREAD_ANYDEPTH);
			method->calculateOcclusion(video.colorFramePtr(),
				video.depthFramePtr(), (unsigned short*)(virtualDepth.data), matte.data);
			cv::Mat matteCpy;
			matte.copyTo(matteCpy);
			frames.push_back(matteCpy);
		}
		logFile << "\n";
		std::cout << "\t" << method->getName() << " completed..." << std::endl;
	}

	return 0;
}
