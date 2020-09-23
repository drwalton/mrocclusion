#ifndef MROCCLUSION_ITERATIVERGBDOCCLUSIONMETHODMOG_HPP_INCLUDED
#define MROCCLUSION_ITERATIVERGBDOCCLUSIONMETHODMOG_HPP_INCLUDED

#include "mrocclusion/RgbdOcclusionMethod.hpp"

///\brief Occlusion method using a non-linear least squares optimisation.
///       Uses a Mixture of Gaussians model fitted to the colours of the pixels around the 
///       unknown region.
class IterativeRgbdOcclusionMethodMOG : public RgbdOcclusionMethod {
public:
	IterativeRgbdOcclusionMethodMOG(
		size_t colorWidth, size_t colorHeight,
		size_t depthWidth, size_t depthHeight);
	virtual ~IterativeRgbdOcclusionMethodMOG() throw();


	virtual void calculateOcclusion(
		const unsigned char *rgbdColor, 
		const unsigned short *rgbdDepth,
		const unsigned short *virtualDepth,
		unsigned char * matte);

	virtual std::string getName() const { return "IterativeMOG"; }
	
	bool usePixelsOffVirtualObject() { return useOffVirtualObject_; }
	void usePixelsOffVirtualObject(bool p) { useOffVirtualObject_ = p; }
private:
	bool useOffVirtualObject_;
	float gradientWeight_, colorWeight_;
	int maxIterations_;
	bool printProgress_;
	size_t nGaussians_, dilateAmount_, maxNSamples_;

	IterativeRgbdOcclusionMethodMOG(const IterativeRgbdOcclusionMethodMOG&);
	IterativeRgbdOcclusionMethodMOG& operator=(const IterativeRgbdOcclusionMethodMOG&);
};

#endif
