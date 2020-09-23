#include "mrocclusion/AdaptiveBilateralRgbdOcclusionMethod.hpp"
#include <thread>

inline float gaussian(float x, float sigma)
{
	x = fminf(x, 100.f); //Values above 100.f tend to be close to zero, and unstable.
	float a = x/sigma;
	return expf(-0.5f * a * a);
}

inline uchar absdiff(uchar a, uchar b) {
	return a < b ? b-a : a-b;
}

inline float colorDiff(const cv::Vec3b &c1, const cv::Vec3b &c2) {
	float diff = 0.f;
	diff += absdiff(c1[0], c2[0]);
	diff += absdiff(c1[1], c2[1]);
	diff += absdiff(c1[2], c2[2]);
	return diff;
	
//	float diff = 0.f, tmp;
//	tmp = float(c1[0]) - float(c2[0]);
//	diff += tmp*tmp;
//	tmp = float(c1[1]) - float(c2[1]);
//	diff += tmp*tmp;
//	tmp = float(c1[2]) - float(c2[2]);
//	diff += tmp*tmp;
//	return sqrtf(diff);
}

inline float spatialDiff(int r1, int c1, int r2, int c2) {
	float diff = 0.f, tmp;
	tmp = float(r1) - float(r2);
	diff += tmp*tmp;
	tmp = float(c1) - float(c2);
	diff += tmp*tmp;
	return sqrtf(diff);
}

AdaptiveBilateralRgbdOcclusionMethod::AdaptiveBilateralRgbdOcclusionMethod(
		size_t colorWidth, size_t colorHeight,
		size_t depthWidth, size_t depthHeight)
	:RgbdOcclusionMethod(
		colorWidth, colorHeight,
		depthWidth, depthHeight),
	depthSigma_(30.0),
	colorSigma_(12.0),
	maxFilterSize_(32),
	minFgBgPixels_(100),
	parallel_(true)
{}

AdaptiveBilateralRgbdOcclusionMethod::~AdaptiveBilateralRgbdOcclusionMethod() throw()
{}

void processOneRow(int r, const cv::Mat &alphaMatte,
	uchar *processptr, const cv::Vec3b *rcolorPtr,
	const cv::Mat &behind, const cv::Mat &infront, const cv::Mat &realColor,
	float colorSigma_, float depthSigma_,
	int minFgBgPixels_, int maxFilterSize_, float *rptrOut_)
{
	
	for(int c = 0; c < alphaMatte.cols; ++c) {
		if(processptr[c] == 0) {
			//Pixel not in "PROCESS" category, ignore.
			continue;
		}
		int currRad = 1;
		int nFgPixels = 0, nBgPixels = 0;
		float sumWeights = 0.f;
		float sumVals = 0.f;
		
		while(true) {
			//Find bounds of window to iterate over.
			int left = c - currRad;
			int right = c + currRad;
			int above = r - currRad;
			int below = r + currRad;
			
			cv::Vec3b centerColor = rcolorPtr[c];
			
			//Iterate over window.
			for(int r2 = above; r2 <= below; ++r2) {
        		const uchar *behindptr = behind.ptr<uchar>(r2);
        		const uchar *infrontptr = infront.ptr<uchar>(r2);
				const cv::Vec3b *r2ColorPtr = realColor.ptr<cv::Vec3b>(r2);
				for(int c2 = left; c2 <= right; ++c2) {
					if(r2 >= 0 && r2 < alphaMatte.rows &&
					   c2 >= 0 && c2 < alphaMatte.cols) {
						//In range
						if(r2 == above || r2 == below ||
							 c2 == left || c2 == right) {
							//On edge of window.
							float val;
							if(infrontptr[c2]) {
								++nFgPixels;
								val = 0.f;
							} else if(behindptr[c2]) {
								++nBgPixels;
								val = 1.f;
							} else {
								continue;
							}
        					float cDiff = colorDiff(r2ColorPtr[c2], centerColor);
							float cWeight = gaussian(cDiff, colorSigma_);
        					float sDiff = spatialDiff(r, c, r2, c2);
        					float sWeight = gaussian(sDiff, depthSigma_);
        					float weight = cWeight * sWeight;
        					
        					sumWeights += weight;
        					sumVals += weight * val;
						}
					}
				}
			}
			
			if(nFgPixels >= minFgBgPixels_ && nBgPixels >= minFgBgPixels_) {
				break;
			}
			++currRad;
			if(currRad > maxFilterSize_) {
				if(nFgPixels == 0 && nBgPixels == 0) {
					sumVals = 1.f, sumWeights = 1.f;
				}
				break;
			}
		}

		if(sumWeights != 0.f) {
			rptrOut_[c] = sumVals / sumWeights;
		}
	}
}

void AdaptiveBilateralRgbdOcclusionMethod::calculateOcclusion(
	const unsigned char * rgbdColor, const unsigned short * rgbdDepth, 
	const unsigned short * virtualDepth, unsigned char * matte)
{
	const cv::Mat realColor(cv::Size(colorWidth, colorHeight), CV_8UC3, (void*)(rgbdColor));
	const cv::Mat realDepth(cv::Size(depthWidth, depthHeight), CV_16UC1, (void*)(rgbdDepth));
	const cv::Mat virtualDepthMat(cv::Size(depthWidth, depthHeight), CV_16UC1, (void*)(virtualDepth));
	cv::Mat process, behind, infront;
	classifyPixels(realColor, realDepth, virtualDepthMat,
		process, behind, infront, false);
	cv::Mat ignore = ~infront & ~behind & (virtualDepth == 0);
	
	cv::Mat alphaMatte(ignore.size(), CV_32FC1);
	alphaMatte.setTo(0.f, ~process);
	alphaMatte.setTo(1.f, behind & virtualDepth != 0);
	
	std::vector<std::thread> threads(alphaMatte.rows);
	
	for(int r = 0; r < alphaMatte.rows; ++r) {
	
		float *rptrOut = alphaMatte.ptr<float>(r);
		const cv::Vec3b *rcolorPtr = realColor.ptr<cv::Vec3b>(r);
		
		uchar *processptr = process.ptr<uchar>(r);
		
		if(!parallel_) {
			processOneRow(r, alphaMatte, processptr, rcolorPtr, behind, infront,
				realColor, colorSigma_, depthSigma_, minFgBgPixels_,
				maxFilterSize_, rptrOut);
		} else {
			threads[r] = std::thread([r, &alphaMatte, processptr, rcolorPtr,
				&behind, &infront, &realColor, this, rptrOut]() {
    			processOneRow(r, alphaMatte, processptr, rcolorPtr, behind, infront,
    				realColor, colorSigma_, depthSigma_, minFgBgPixels_,
    				maxFilterSize_, rptrOut);
			});
		}
	}
	
	for(std::thread &t : threads) {
		t.join();
	}
	
	alphaMatte *= 255.f;
	cv::Mat matteByte;
	alphaMatte.convertTo(matteByte, CV_8UC1);
	cv::Mat matteMat(cv::Size(alphaMatte.cols, alphaMatte.rows), CV_8UC1, matte);
	matteByte.copyTo(matteMat);
}

std::string AdaptiveBilateralRgbdOcclusionMethod::getName() const
{
	std::string name = "Adaptive";
	return name;
}

AdaptiveBilateralRgbdOcclusionMethod &AdaptiveBilateralRgbdOcclusionMethod::maxFilterSize(int s)
{
	maxFilterSize_ = s;
	return *this;
}

AdaptiveBilateralRgbdOcclusionMethod &AdaptiveBilateralRgbdOcclusionMethod::depthSigma(double s)
{
	depthSigma_ = s;
	return *this;
}

AdaptiveBilateralRgbdOcclusionMethod &AdaptiveBilateralRgbdOcclusionMethod::colorSigma(double s)
{
	colorSigma_ = s;
	return *this;
}

AdaptiveBilateralRgbdOcclusionMethod & AdaptiveBilateralRgbdOcclusionMethod::minFgBgPixels(int m)
{
	minFgBgPixels_ = m;
	return *this;
}
