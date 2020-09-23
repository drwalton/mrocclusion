#ifndef CRABBOCCLUSIONMETHOD_HPP_INCLUDED
#define CRABBOCCLUSIONMETHOD_HPP_INCLUDED

#include "mrocclusion/RgbdOcclusionMethod.hpp"

///\brief Implementation of Crabb et al., used as a bilateral filter.
class CrabbOcclusionMethod final : public RgbdOcclusionMethod
{
public:
	explicit CrabbOcclusionMethod();

	virtual ~CrabbOcclusionMethod() throw();

	virtual void calculateOcclusion(
		const unsigned char *rgbdColor,
		const unsigned short *rgbdDepth,
		const unsigned short *virtualDepth,
		unsigned char * matte);
	virtual cv::Mat calculateOcclusion(
		cv::Mat realColor, cv::Mat realDepth,
		cv::Mat virtualColor, cv::Mat virtualDepth) const;

	virtual std::string getName() const;

	virtual cv::Mat illustrateOcclusion(
		cv::Mat realDepth, cv::Mat virtualDepth, bool showText = true) const;
	
	cv::Mat kernelSizes() const;
	
	int filterSize() const;
	CrabbOcclusionMethod &filterSize(int f);
	
	double depthSigma() const;
	CrabbOcclusionMethod &depthSigma(double s);
	
	double colorSigma() const;
	CrabbOcclusionMethod &colorSigma(double s);
private:
	double depthSigma_, colorSigma_;
	bool parallel_;
	bool visualizeKernelSizes_;
	cv::Mat occlusion_, kernelSizes_;
	int filterSize_;
};

#endif //BILATERALOCCLUSIONMETHOD_HPP_INCLUDED
