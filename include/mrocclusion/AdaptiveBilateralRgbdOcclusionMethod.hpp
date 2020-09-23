#ifndef MROCCLUSION_ADAPTIVEBILATERALOCCLUSIONMETHOD_HPP_INCLUDED
#define MROCCLUSION_ADAPTIVEBILATERALOCCLUSIONMETHOD_HPP_INCLUDED

#include "mrocclusion/RgbdOcclusionMethod.hpp"

class AdaptiveBilateralRgbdOcclusionMethod : public RgbdOcclusionMethod
{
public:
	AdaptiveBilateralRgbdOcclusionMethod(
		size_t colorWidth, size_t colorHeight,
		size_t depthWidth, size_t depthHeight);
	virtual ~AdaptiveBilateralRgbdOcclusionMethod() throw();
	
	virtual void calculateOcclusion(
		const unsigned char * rgbdColor, const unsigned short * rgbdDepth,
		const unsigned short * virtualDepth, unsigned char * matte);
	
	virtual std::string getName() const;
	
	cv::Mat kernelSizes() const;
	
	int maxFilterSize() const;
	AdaptiveBilateralRgbdOcclusionMethod &maxFilterSize(int f);
	
	double depthSigma() const;
	AdaptiveBilateralRgbdOcclusionMethod &depthSigma(double s);
	
	double colorSigma() const;
	AdaptiveBilateralRgbdOcclusionMethod &colorSigma(double s);

	AdaptiveBilateralRgbdOcclusionMethod &minFgBgPixels(int m);
private:
	double depthSigma_, colorSigma_;
	int maxFilterSize_;
	int minFgBgPixels_;
	bool parallel_;
	cv::Mat kernelSizes_;
};

#endif
