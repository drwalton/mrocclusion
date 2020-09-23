#include "mrocclusion/BlobRemoval.hpp"

void removeSmallBlobs(cv::Mat &image, int maxArea, uchar blobVal, uchar bgVal)
{
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::Mat findIm = image == blobVal;
	cv::findContours(findIm, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0,0));
	
	for(auto &contour : contours) {
		cv::Rect boundingRect = cv::boundingRect(contour);
		if(boundingRect.area() < maxArea) {
			image(boundingRect) = bgVal;
		}
	}
	
}
