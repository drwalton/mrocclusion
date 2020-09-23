#include "mrocclusion/GuidedFilterGpu.hpp"
#include <mrocclusion/GLWindow.hpp>
#include <mrocclusion/Texture.hpp>
#include <iostream>
#include <chrono>
#include <opencv2/opencv.hpp>

int main(int argc, char *argv[]) {

	GLWindow win("hidden", 0, 0);

	{
		GuidedFilterGpu filter(640, 480);

		float *testIm = new float[640 * 480];
		float *testSat = new float[640 * 480];

		for (size_t i = 0; i < 640 * 480; ++i) {
			if(i%2 == 0) testIm[i] = 1;
			else testIm[i] = 0.0001;
		}

		filter.testSat(testIm, testSat);
		for (size_t i = 0; i < 640; ++i) {
			std::cout << testSat[479 * 640 + i] << std::endl;
		}
	}
/*
		cv::Mat im(cv::Size(640, 480), CV_32FC1, testSat);
		cv::imshow("sat", im / (640.f*480.f));
		cv::waitKey();

		for (size_t i = 0; i < 640; ++i) {
			std::cout << testSat[i] << std::endl;
		}
		for (size_t i = 0; i < 640; ++i) {
			std::cout << testSat[i] << std::endl;
		}

		std::cout << testSat[(640 * 480) - 1] << std::endl;
		std::cout << testSat[(640 * 480) - 480] << std::endl;

		for (size_t i = 0; i < 640; ++i) {
			std::cout << testSat[i + 479*640] << std::endl;
		}

		for (size_t i = 0; i < 640; ++i) {
			std::cout << testSat[i + 640] << std::endl;
		}

		//filter.testBoxFilter(testIm, testSat);
		filter.testNormalBoxFilter(testIm, testSat);
		for (size_t i = 0; i < 10; ++i) {
			std::cout << testSat[i + 640 * i] << std::endl;
		}
		delete(testIm);
		delete(testSat);
	}
	*/
	
	cv::Mat image = cv::imread(argv[1]);
	cv::Mat fltImage;
	image.convertTo(fltImage, CV_32F);
	fltImage /= 255.f;
	std::vector<cv::Mat> channels;
	GuidedFilterGpu filter(image.cols, image.rows);
	cv::split(fltImage, channels);

	cv::Mat image2 = cv::imread(argv[2]);
	cv::Mat fltImage2;
	image2.convertTo(fltImage2, CV_32F);
	fltImage2 /= 255.f;
	std::vector<cv::Mat> channels2;
	cv::split(fltImage2, channels2);
	fltImage2 = channels2[0];
	cv::Mat result(image.rows, image.cols, CV_32FC1);

	filter.testGuidedFilter(
			(float*)(channels[0].data),
			(float*)channels[1].data,
			(float*)channels[2].data,
			(float*)fltImage2.data,
			(float*)result.data,
		5, 0.1f);

	cv::imshow("I", fltImage);
	cv::imshow("p", fltImage2);
	cv::imshow("result", result);
	cv::waitKey();

	filter.testNormalBoxFilter(
		(float*)(channels[0].data),
		(float*)result.data);

	cv::waitKey();
	/*
	delete(testIm);
	delete(testSat);
	*/

	return 0;
}
