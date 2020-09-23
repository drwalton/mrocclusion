#include "mrocclusion/GuidedFilterGpu.hpp"
#include <mrocclusion/FullScreenQuad.hpp>
#include <mrocclusion/ShaderProgram.hpp>
#include <mrocclusion/GLWindow.hpp>
#include <mrocclusion/Directories.hpp>
#include <mrocclusion/Texture.hpp>
#include <mrocclusion/AntTweakBar.hpp>
#include <iostream>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <opencv2/ximgproc.hpp>
#include <mrocclusion/guidedfilter.h>

int main(int argc, char *argv[]) {

	cv::Mat guidanceImage, inputImage;
	{
		guidanceImage = cv::imread(argv[1]);

		cv::Mat inputImage3 = cv::imread(argv[2]);
		std::vector<cv::Mat> channels;
		cv::split(inputImage3, channels);
		inputImage = channels[0];
	}

	GLWindow win("Guided filter output", inputImage.cols, inputImage.rows);
	GuidedFilterGpu filter(inputImage.cols, inputImage.rows);

	ShaderProgram showImageShader(std::vector<std::string> {
		MROCCLUSION_SHADER_DIR + "FullScreenTex.vert",
		MROCCLUSION_SHADER_DIR + "FullScreenTexFlipRedToGray.frag"
	});
	showImageShader.setUniform("tex", 0);

	Texture guidanceImageTex(GL_TEXTURE_2D, GL_RGBA, guidanceImage.cols, guidanceImage.rows, 
		0, GL_RGB, GL_UNSIGNED_BYTE, guidanceImage.data);
	Texture inputImageTex(GL_TEXTURE_2D, GL_RED, guidanceImage.cols, guidanceImage.rows, 
		0, GL_RED, GL_UNSIGNED_BYTE, inputImage.data);
	Texture outputImageTex(GL_TEXTURE_2D, GL_RED, guidanceImage.cols, guidanceImage.rows, 
		0, GL_RED, GL_UNSIGNED_BYTE, inputImage.data);

	int filterRadius = 5;
	float filterEps = 0.01f;

	AntTweakBar tweakBar(&win, "Guided Filter");
	tweakBar.addVarRW("Radius", &filterRadius, "Radius", 0, 20, "Radius of guided filter");
	tweakBar.addVarRW("Epsilon", &filterEps, "Epsilon", 0.0f, 100.0f, .001f, "Regularisation level of guided filter");

	bool running = true;
	SDL_Event event;

	auto lastLoopTime = std::chrono::system_clock::now();

	cv::Mat diff_r, diff_g, diff_b;
	diff_r = cv::Mat(inputImage.size(), CV_32FC1);
	diff_g = cv::Mat(inputImage.size(), CV_32FC1);
	diff_b = cv::Mat(inputImage.size(), CV_32FC1);
	diff_g.setTo(0.f);

	cv::Mat diffIm;
	cv::merge(std::vector<cv::Mat>{ diff_r, diff_g, diff_b }, diffIm);

	while (running) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		filter.setup(&guidanceImageTex, filterRadius, filterEps);
		filter.filterUChar(&inputImageTex, &outputImageTex);

		glDisable(GL_DEPTH_TEST);
		outputImageTex.bindToImageUnit(0);
		FullScreenQuad::getInstance().render(showImageShader);

		tweakBar.draw();
		cv::Mat guidanceF, inputF;
		guidanceImage.convertTo(guidanceF, CV_32F);
		inputImage.convertTo(inputF, CV_32F);
		guidanceF /= 255.f;
		inputF /= 255.f;
		cv::Mat result, ourResult;
		cv::Mat outputImageMat(guidanceF.size(), CV_8UC1);
		outputImageTex.getData(outputImageMat.data, 640*480);
		outputImageMat.convertTo(ourResult, CV_32FC1);
		ourResult = ourResult / 255.f;
		double a, b;

		cv::ximgproc::guidedFilter(guidanceF, inputF, result, filterRadius, 80.f);
		cv::Mat resultU(result.size(), CV_8UC1);
		result *= 255.f;
		result.convertTo(resultU, CV_8U);
		resultU.convertTo(result, CV_32F);
		result /= 255.f;
		//GuidedFilter filter(guidanceF, filterRadius, filterEps);
		//result = filter.filter(inputF);

		cv::minMaxIdx(ourResult, &a, &b);
		std::cout << "1min " << a << " max " << b << std::endl;
		cv::minMaxIdx(result, &a, &b);
		std::cout << "2min " << a << " max " << b << std::endl;

		diff_r = ourResult - result;
		diff_b = result - ourResult;
		cv::imshow("Diff im", diff_r);
		cv::imshow("Diff im2", diff_b);

		cv::imshow("Opencv result", result);
		cv::waitKey(1);

		win.swapBuffers();
		while (SDL_PollEvent(&event)) {
			tweakBar.processEvent(event);
			if (GLWindow::eventIsQuit(event)) {
				running = false;
			}
		}

		auto d = std::chrono::system_clock::now() - lastLoopTime;
		std::stringstream title;
		title << "Guided Filter " << std::chrono::duration_cast<std::chrono::milliseconds>(d).count() << "ms";
		win.setTitle(title.str());
		lastLoopTime = std::chrono::system_clock::now();
	}

	return 0;
}
