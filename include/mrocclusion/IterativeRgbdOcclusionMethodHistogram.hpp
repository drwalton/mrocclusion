#ifndef MROCCLUSION_ITERATIVERGBDOCCLUSIONMETHODHISTOGRAM_HPP_INCLUDED
#define MROCCLUSION_ITERATIVERGBDOCCLUSIONMETHODHISTOGRAM_HPP_INCLUDED

#include "mrocclusion/RgbdOcclusionMethod.hpp"
#include "mrocclusion/RgbHistogram.hpp"

bool fitFgBgHistograms(
	cv::Mat process, cv::Mat behind, cv::Mat infront,
	cv::Mat realColor,
	RgbHistogram *infrontHist, RgbHistogram *behindHist,
	size_t dilateAmount, size_t minSampleCount);

///\brief Occlusion method using a non-linear least squares optimisation.
///       Part of the error function uses a histogram, fitted to pixels near the
///       unknown region.
class IterativeRgbdOcclusionMethodHistogram : public RgbdOcclusionMethod {
public:
	IterativeRgbdOcclusionMethodHistogram(
		size_t colorWidth, size_t colorHeight,
		size_t depthWidth, size_t depthHeight);
	virtual ~IterativeRgbdOcclusionMethodHistogram() throw();


	virtual void calculateOcclusion(
		const unsigned char *rgbdColor,
		const unsigned short *rgbdDepth,
		const unsigned short *virtualDepth,
		unsigned char * matte);
		
	virtual std::string getName() const { return "IterativeHistogram"; }

	bool usePixelsOffVirtualObject() { return useOffVirtualObject_; }
	void usePixelsOffVirtualObject(bool p) { useOffVirtualObject_ = p; }
private:
	bool useOffVirtualObject_;
	float gradientWeight_, colorWeight_;
	int maxIterations_;
	bool printProgress_;
	size_t dilateAmount_, maxNSamples_, binWidth_;
};

#endif
