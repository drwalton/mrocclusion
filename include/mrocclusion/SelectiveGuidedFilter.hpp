#ifndef MROCCLUSION_SELECTIVEGUIDEDFILTER_HPP_INCLUDED
#define MROCCLUSION_SELECTIVEGUIDEDFILTER_HPP_INCLUDED

#include <opencv2/opencv.hpp>

void selectiveBoxFilter(
	const cv::Mat &inputImage, size_t r,
	const cv::Mat &mask,
	cv::Mat *outputImage);

void selectiveGuidedFilter(
	const cv::Mat &inputImage, const cv::Mat &guidanceImage,
	const cv::Mat &mask, cv::Mat *outputImage,
	size_t r, float eps);

//!\brief Selective guided filter.
//!       This version of the guided filter only affects pixels set to true in
//!       the process mask, and only uses information from pixels set to true
//!       in the use mask.
//!\param inputImage The image to filter. Should be Should be 1 or 3-channel,
//!       unsigned char or 32-bit floating-point type.
//!\param guidanceImage The guidance image (I). Should be 1 or 3-channel,
//!       unsigned char or 32-bit floating-point type.
//!\param processMask Mask indicating pixels that should be written to by this
//!       filter. Information from these pixels will be taken from the guidance
//!       image, but not from the input image.
//!\param useMask Mask indicating which pixels to use in the filtering process.
//!       pixels not set to true in this mask will not be involved in the filter.
//!\param[out] outputImage Output image (CV_8UC1 or CV_8UC3)
//!\param r Radius parameter for the guided filter.
//!\param eps Epsilon parameter for the guided filter.
void selectiveGuidedFilter(
	const cv::Mat &inputImage, const cv::Mat &guidanceImage,
	const cv::Mat &processMask, const cv::Mat &useMask,
	cv::Mat *outputImage,
	size_t r, float eps);

//!\brief Selective normalized box filter.
//!\param r Radius parameter for the box filter. The filter kernel is a square
//!       with sides of length (r*2 + 1).
//!\param processMask Mask indicating pixels that should be written to by this
//!       filter. Information from these pixels will be taken from the guidance
//!       image, but not from the input image.
//!\param useMask Mask indicating which pixels to use in the filtering process.
//!       pixels not set to true in this mask will not be involved in the filter.
void selectiveBoxFilter(
	const cv::Mat &inputImage, size_t r,
	const cv::Mat &processMask, const cv::Mat &useMask,
	cv::Mat *outputImage);

#endif
