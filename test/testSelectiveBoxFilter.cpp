#include "mrocclusion/GuidedRgbdOcclusionMethod.hpp"
#include "mrocclusion/guidedfilter.h"

int radius = 4;
float eps = .16f;

int main(int argc, char *argv[]) {
	if(argc != 3) {
		std::cout << "usage testGuidedFilter [I] [p]" << std::endl;
		return 1;
	}

	cv::Mat I = cv::imread(argv[1]);
	cv::Mat p = cv::imread(argv[2]);
	cv::imshow("p", p);
	cv::imshow("I", I);
	cv::waitKey(1);
	cv::Mat q;

	guidedFilter(p, I, q, radius, eps);
	
	cv::imshow("p", p);
	cv::imshow("I", I);
	cv::imshow("q", q);

	cv::Mat q2 = guidedFilter(I, p, radius*2, 2055.f);

	cv::imshow("q (from opencv)", q2);
	
	cv::Mat p1; cv::cvtColor(p, p1, cv::COLOR_RGB2GRAY);
	cv::imshow("p=q", p1 == q);
	
	cv::waitKey();

	return 0;
}
