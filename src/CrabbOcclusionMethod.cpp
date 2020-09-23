#include "mrocclusion/CrabbOcclusionMethod.hpp"
#include <boost/timer/timer.hpp>
#include <thread>

const float HIGH_PROBABILITY = 1.f;
const float LOW_PROBABILITY = 0.35f;
const float PROB_CONFIDENCE_THRESHOLD = 0.8f;
const uchar FG_LABEL = 255;
const uchar BG_LABEL = 0;
const uchar UNCERTAIN_LABEL = 128;

const float SPARSE_MATTE_INVALID_VALUE = -1.f;

const cv::Size BOX_FILTER_SIZE(15, 15);

inline float gaussian(float x, float sigma)
{
	x = fminf(x, 100.f); //Values above 100.f tend to be close to zero, and unstable.
	float a = x/sigma;
	return expf(-0.5f * a * a);
}

inline float colorDiff(const cv::Vec3b &c1, const cv::Vec3b &c2) {
	float diff = 0.f, tmp;
	tmp = float(c1[0]) - float(c2[0]);
	diff += tmp*tmp;
	tmp = float(c1[1]) - float(c2[1]);
	diff += tmp*tmp;
	tmp = float(c1[2]) - float(c2[2]);
	diff += tmp*tmp;
	return sqrtf(diff);
}

inline float spatialDiff(int r1, int c1, int r2, int c2) {
	float diff = 0.f, tmp;
	tmp = float(r1) - float(r2);
	diff += tmp*tmp;
	tmp = float(c1) - float(c2);
	diff += tmp*tmp;
	return sqrtf(diff);
}

CrabbOcclusionMethod::CrabbOcclusionMethod()
	:depthSigma_(30.0),
	colorSigma_(12.0),
	filterSize_(32),
	parallel_(true),
	RgbdOcclusionMethod(640, 480, 640, 480)
{}

CrabbOcclusionMethod::~CrabbOcclusionMethod() throw()
{}

void processOneRow(cv::Mat &alphaMatte,
	uchar *trimapPtr, uchar *vdepthptr, int winSize, int r, cv::Vec3b *rcolorPtr,
	cv::Mat &sparseAlphaMatte, cv::Mat &realColor, float depthSigma_,
	float colorSigma_, float *rptrOut)
{
	
		for(int c = 0; c < alphaMatte.cols; ++c) {
			if(trimapPtr[c] != UNCERTAIN_LABEL) {
				//Skip further processing if this pixel is already known to be
				// FG or BG.
				#ifdef SHOW_OUTCOMES
				outcomes.at<cv::Vec3b>(r,c) = cv::Vec3b(255,255,255);
				#endif
				continue;
			}
			if(!vdepthptr[c]) {
				//Virtual object isn't here - continue.
				#ifdef SHOW_OUTCOMES
				outcomes.at<cv::Vec3b>(r,c) = cv::Vec3b(0,0,255);
				#endif
				continue;
			}
			
			//Find bounds of window to iterate over.
			int left = std::max(0, c - winSize);
			int right = std::min(alphaMatte.cols-1, c + winSize);
			int above = std::max(0, r - winSize);
			int below = std::min(alphaMatte.rows-1, r + winSize);
			
			float sumWeights = 0.f;
			float sumVals = 0.f;
			
			cv::Vec3b centerColor = rcolorPtr[c];
			
			//Iterate over window.
			for(size_t r2 = above; r2 <= below; ++r2) {
				float *r2ptr = sparseAlphaMatte.ptr<float>(r2);
				cv::Vec3b *r2ColorPtr = realColor.ptr<cv::Vec3b>(r2);
				for(size_t c2 = left; c2 <= right; ++c2) {
					float val = r2ptr[c2];
					if(val == SPARSE_MATTE_INVALID_VALUE) {
						//Don't use this value in filtering.
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
			
			if(sumWeights != 0.f) {
				rptrOut[c] = sumVals / sumWeights;
				#ifdef SHOW_OUTCOMES
				outcomes.at<cv::Vec3b>(r,c) = cv::Vec3b(0, 255, 0);
				#endif
			}
		}
}

void CrabbOcclusionMethod::calculateOcclusion(const unsigned char * rgbdColor, const unsigned short * rgbdDepth, const unsigned short * virtualDepth, unsigned char * matte)
{
	cv::Mat rgbdColorMat(cv::Size(colorWidth, colorHeight), CV_8UC3, (void*)rgbdColor);
	cv::Mat rgbdDepthMat(cv::Size(colorWidth, colorHeight), CV_16UC1, (void*)rgbdDepth);
	cv::Mat virtualDepthMat(cv::Size(colorWidth, colorHeight), CV_16UC1, (void*)virtualDepth);
	cv::Mat virtualColorMat(cv::Size(colorWidth, colorHeight), CV_8UC3);
	cv::Mat matteMat = calculateOcclusion(rgbdColorMat, rgbdDepthMat, virtualColorMat, virtualDepthMat);
	cv::Mat matteOut(cv::Size(colorWidth, colorHeight), CV_8UC1, (void*)(matte));
	matteMat.copyTo(matteOut);
}

cv::Mat CrabbOcclusionMethod::calculateOcclusion(
	cv::Mat realColor, cv::Mat realDepth_,
	cv::Mat virtualColor, cv::Mat virtualDepth) const
{
	//0. Set depth threshold
	cv::Scalar avgVirtDepth = cv::mean(virtualDepth, virtualDepth != 0);
	float depthThreshold = avgVirtDepth(0);
	

	//1. Trimap generation
	cv::Mat initialProbs(realColor.size(), CV_32FC1);
	
	initialProbs.setTo(HIGH_PROBABILITY, realDepth_ < depthThreshold);
	initialProbs.setTo(0.f, realDepth_ > depthThreshold);
	initialProbs.setTo(LOW_PROBABILITY, realDepth_ == 0);
	
	cv::Mat probs;
	cv::boxFilter(initialProbs, probs, -1, BOX_FILTER_SIZE);
	
	cv::Mat trimap(probs.size(), CV_8UC1);
	trimap.setTo(UNCERTAIN_LABEL);
	trimap.setTo(FG_LABEL, probs > PROB_CONFIDENCE_THRESHOLD);
	trimap.setTo(BG_LABEL, probs < (1.f - PROB_CONFIDENCE_THRESHOLD));
	
	//2. Generate sparse alpha matte.
	cv::Mat sparseAlphaMatte(realDepth_.size(), CV_32FC1);
	sparseAlphaMatte.setTo(1.f, realDepth_ < depthThreshold);
	sparseAlphaMatte.setTo(0.f, realDepth_ > depthThreshold);
	sparseAlphaMatte.setTo(SPARSE_MATTE_INVALID_VALUE, realDepth_ == 0.f);
	
	cv::Mat alphaMatte;
	sparseAlphaMatte.copyTo(alphaMatte);
	
	#ifdef SHOW_OUTCOMES
	cv::Mat outcomes(trimap.size(), CV_8UC3);
	#endif
	
	//3. Filter sparse alpha matte.
	const int winSize = filterSize_;
	cv::Mat vdepthMask = virtualDepth != 0;
	std::vector<std::thread> threads(alphaMatte.rows);

	for(int r = 0; r < alphaMatte.rows; ++r) {
		float *rptrOut = alphaMatte.ptr<float>(r);
		cv::Vec3b *rcolorPtr = realColor.ptr<cv::Vec3b>(r);
		uchar *trimapPtr = trimap.ptr<uchar>(r);
		uchar *vdepthptr = vdepthMask.ptr<uchar>(r);
		
		if(!parallel_) {
    		processOneRow(alphaMatte, trimapPtr, vdepthptr, winSize, r,
				rcolorPtr, sparseAlphaMatte, realColor, depthSigma_,
				colorSigma_, rptrOut);
		} else {
			threads[r] = std::thread([r, &alphaMatte, &sparseAlphaMatte, trimapPtr,
				vdepthptr, rcolorPtr, &realColor, this, rptrOut, winSize]() {
        		processOneRow(alphaMatte, trimapPtr, vdepthptr, winSize, r,
    				rcolorPtr, sparseAlphaMatte, realColor, depthSigma_,
    				colorSigma_, rptrOut);
			});
		}
	}
	
	if(parallel_) {
		for(std::thread &t : threads) {
			t.join();
		}
	}
	
	#ifdef SHOW_OUTCOMES
	cv::imshow("OUTCOMES", outcomes);
	#endif
	
	double minV, maxV;
	cv::minMaxLoc(alphaMatte, &minV, &maxV);
	
	
	//The output matte separates FG/BG. Convert this to a virtual object matte.
	cv::Mat occlusionMatte = 1.f - alphaMatte;
	occlusionMatte.setTo(0.f, virtualDepth == 0);
	
	occlusionMatte *= 255.f;
	cv::Mat matteByte;
	occlusionMatte.convertTo(matteByte, CV_8UC1);
	return matteByte;
}

cv::Mat CrabbOcclusionMethod::illustrateOcclusion(
	cv::Mat realDepth, cv::Mat virtualDepth, bool showText) const
{
	return occlusion_;
}

std::string CrabbOcclusionMethod::getName() const
{
	return "Crabb";
}

cv::Mat CrabbOcclusionMethod::kernelSizes() const
{
	return kernelSizes_;
}

int CrabbOcclusionMethod::filterSize() const
{
	return filterSize_;
}

CrabbOcclusionMethod &CrabbOcclusionMethod::filterSize(int f)
{
	filterSize_ = f;
	return *this;
}

double CrabbOcclusionMethod::depthSigma() const
{
	return depthSigma_;
}
CrabbOcclusionMethod &CrabbOcclusionMethod::depthSigma(double s)
{
	depthSigma_ = s;
	return *this;
}

double CrabbOcclusionMethod::colorSigma() const
{
	return colorSigma_;
}
CrabbOcclusionMethod &CrabbOcclusionMethod::colorSigma(double s)
{
	colorSigma_ = s;
	return *this;
}
