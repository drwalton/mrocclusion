#include <SDL.h>
#include "mrocclusion/OpenNiDepthCam.hpp"
#include "mrocclusion/RgbdOcclusionMethod.hpp"
#include "mrocclusion/ErrorMetrics.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/foreach.hpp"
#include <fstream>
#include "mrocclusion/Files.hpp"
#include "mrocclusion/Directories.hpp"
#include "mrocclusion/InftamRgbdSlam.hpp"
#include "mrocclusion/Comparison.hpp"

const bool useInftamMeshingEngine = false;

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
	if(argc != 2) {
		std::cout << "Usage: $ CompareOcclusionMethodsInftam [config]" << std::endl;
		return 1;
	}
	
	boost::property_tree::ptree config;
	boost::property_tree::json_parser::read_json(argv[1], config);
	
	std::string baseFilename = config.get<std::string>("baseFilename");
	std::string vidFile = baseFilename + ".oni";
	std::vector<size_t> compareFrames;
	std::vector<size_t> gtFrames;
	std::vector<size_t> kinfuFrames;
	std::vector<float> temporalNoises;
	readAnnotationFile(baseFilename + ".txt", &compareFrames, &gtFrames, &kinfuFrames);
	PinholeCameraModel colorCamModel(config.get<std::string>("colorConfig"));
	PinholeCameraModel depthCamModel(config.get<std::string>("depthConfig"));
	mat4 rgbToDepth = loadRigidTransformFromFile(
		config.get<std::string>("rgbToDepth"));
	
	std::ofstream logFile(
		MROCCLUSION_RESULTS_DIR + "OcclusionComparisonInftam_" +
		uniqueTimestamp() + ".csv");
	
	logFile << "Video file," << vidFile << "\n";
	
	OpenNiDepthCam video(vidFile);
	InftamRgbdSlam slam(colorCamModel, depthCamModel, rgbToDepth, useInftamMeshingEngine);
	
	std::vector<std::unique_ptr<ErrorMetric> > errorMetrics;
	loadErrorMetrics(&errorMetrics);
	
	//Add titles to table (1x row).
	//First row: frame no, and names of each method.
	logFile << "Frame No., ";
	for(auto & metric : errorMetrics) {
		logFile << metric->name() << ", ";
	}
	logFile << "\n";
	
	cv::Mat virtualDepth, virtualColor, groundTruthMatte, groundTruthMatte3;
	cv::Mat matte(video.colorHeight(), video.colorWidth(), CV_8UC1);
	cv::Mat raycastMat(video.colorHeight(), video.colorWidth(), CV_8UC4);
	
	size_t compareIdx = 0;
	size_t currFrame = 0;
	for(size_t i = 0; i < kinfuFrames.size(); ++i) {
		video.seek(kinfuFrames[i]);
		currFrame = kinfuFrames[i];
		
		//First, process kinfu frames.
		while(currFrame < compareFrames[compareIdx]) {
			video.getLatestFrames();
			slam.processFrames(video.colorFramePtr(), video.depthFramePtr());
			cv::Mat raycastMat(cv::Size(video.colorWidth(), video.colorHeight()), CV_8UC4, (void*)(slam.reconstructionRaycast()));
			cv::imshow("InfiniTAM", raycastMat);
			cv::Mat colorMat(cv::Size(video.colorWidth(), video.colorHeight()), CV_8UC3, (void*)(video.colorFramePtr()));
			cv::imshow("Input Color", colorMat);
			cv::waitKey(50);
			++currFrame;
		}
		
		//Then, perform comparison.
		groundTruthMatte3 = cv::imread(baseFilename + "_gtMatte" +
			std::to_string(gtFrames[i]) + ".png");
		cv::cvtColor(groundTruthMatte3, groundTruthMatte, cv::COLOR_RGB2GRAY);
		cv::imshow("gt", groundTruthMatte);
		std::vector<cv::Mat> mattes;

		cv::Mat augmentedIm;
		do {
			video.getLatestFrames();
			slam.processFrames(video.colorFramePtr(), video.depthFramePtr());
			cv::Mat raycastMat(cv::Size(video.colorWidth(), video.colorHeight()), CV_8UC4, (void*)(slam.reconstructionRaycast()));
			cv::imshow("InfiniTAM", raycastMat);
			cv::Mat colorMat(cv::Size(video.colorWidth(), video.colorHeight()), CV_8UC3, (void*)(video.colorFramePtr()));
			cv::imshow("Input Color", colorMat);
			cv::waitKey(30);

    		virtualColor = cv::imread(baseFilename + "_glColor" +
    			std::to_string(compareFrames[compareIdx]) + ".png");
    		virtualDepth = cv::imread(baseFilename + "_glDepth" +
    			std::to_string(compareFrames[compareIdx]) + ".png", cv::IMREAD_ANYDEPTH);
			cv::Mat inftamDepth(virtualDepth.size(), CV_16UC1,
				(unsigned char*)slam.expectedDepthMap());
			matte = (virtualDepth != 0) & (inftamDepth > virtualDepth);
			cv::Mat matteCpy;
			matte.copyTo(matteCpy);
			mattes.push_back(matteCpy);
			
			
			cv::imshow("InfiniTAM matte", matte);
			cv::imshow("GT matte", groundTruthMatte);
			cv::imshow("diff", matte - groundTruthMatte);
			cv::waitKey(1);

			augmentedIm = composeAugmentedImage(virtualColor, colorMat, matte);
			cv::imwrite("Infinitam" + std::to_string(compareFrames[compareIdx]) + ".png", augmentedIm);
			
			for(auto &metric : errorMetrics) {
				double err = metric->error(
					matte.data, groundTruthMatte.data,
					matte.cols, matte.rows);
				logFile << currFrame << ", " << err << ", ";
			}
			++currFrame;
			++compareIdx;
			logFile << "\n";
		} while(compareIdx < compareFrames.size() &&
			compareFrames[compareIdx] == compareFrames[compareIdx-1]+1);
		temporalNoises.push_back(temporalNoiseMetric(mattes));
	}

	logFile << "\n\nTemporal noise\n";
	for (float f : temporalNoises) {
		logFile << f << "\n";
	}

	return 0;
}
