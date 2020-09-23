#ifndef MROCCLUSION_BILATERALRGBDOCCLUSIONMETHOD_HPP_INCLUDED
#define MROCCLUSION_BILATERALRGBDOCCLUSIONMETHOD_HPP_INCLUDED

#include "mrocclusion/RgbdOcclusionMethod.hpp"

///\brief Occlusion method using a selective cross bilateral filter.
class BilateralRgbdOcclusionMethod : public RgbdOcclusionMethod {
public:
	BilateralRgbdOcclusionMethod(
		size_t colorWidth, size_t colorHeight,
		size_t depthWidth, size_t depthHeight);
	virtual ~BilateralRgbdOcclusionMethod() throw();

	virtual void processEvent(SDL_Event &e);

	virtual void calculateOcclusion(
		const unsigned char * rgbdColor, const unsigned short * rgbdDepth,
		const unsigned short * virtualDepth, unsigned char * matte);

	virtual std::string getName() const { return "Bilateral" + std::to_string(filterSize_) + " " + std::to_string(colorSigma_) + " " + std::to_string(depthSigma_); }
	
	bool parallel() { return parallel_; }
	void parallel(bool p) { parallel_ = p; }

	bool usePixelsOffVirtualObject() { return useOffVirtualObject_; }
	void usePixelsOffVirtualObject(bool p) { useOffVirtualObject_ = p; }

	float colorSigma() const { return colorSigma_; }
	float depthSigma() const { return depthSigma_; }
	void colorSigma(float s)  { colorSigma_ = s; }
	void depthSigma(float s)  { depthSigma_ = s; }
	size_t filterSize() const { return filterSize_; }
	void filterSize(size_t s) { filterSize_ = s; }

private:
	bool parallel_;
	float colorSigma_, depthSigma_;
	size_t filterSize_;
	bool useOffVirtualObject_;
};

#endif
