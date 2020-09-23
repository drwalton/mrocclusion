#ifndef BLOB_REMOVAL_HPP_INCLUDED
#define BLOB_REMOVAL_HPP_INCLUDED

#include <opencv2/opencv.hpp>

void removeSmallBlobs(cv::Mat &image, int maxArea, uchar blobVal = 0, uchar bgVal = 255);

#endif //BLOB_REMOVAL_HPP_INCLUDED

