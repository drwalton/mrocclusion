//#include "mrocclusion/GuidedRgbdOcclusionMethod.hpp"
#include "mrocclusion/SelectiveGuidedFilter.hpp"

int radius = 5;
float eps = 160.f;

int main(int argc, char *argv[]) {
	if(argc != 4) {
		std::cout << "usage testGuidedFilter [I] [p]" << std::endl;
		return 1;
	}

	cv::Mat I = cv::imread(argv[1]);
	cv::cvtColor(I, I, cv::COLOR_RGB2GRAY);
	cv::Mat p = cv::imread(argv[2]);
	cv::cvtColor(p, p, cv::COLOR_RGB2GRAY);
	cv::Mat mask = cv::imread(argv[3]);
	cv::cvtColor(mask, mask, cv::COLOR_RGB2GRAY);
	cv::Mat processMask = mask, useMask = mask;
	processMask.setTo(255);
	useMask.setTo(255);

	cv::imshow("p", p);
	cv::imshow("I", I);
	cv::waitKey(1);
	cv::Mat q;

	selectiveGuidedFilter(p, I, mask, &q, radius, eps);

	cv::Mat b(p.rows, p.cols, CV_32FC1);
	cv::Mat pf;
	p.convertTo(pf, CV_32F);
	selectiveBoxFilter(pf, radius, mask, &b);
	cv::imshow("b", b/255.f);
	
	cv::imshow("p", p);
	cv::imshow("I", I);
	cv::imshow("q", q);
	
	cv::waitKey();

	return 0;
}
