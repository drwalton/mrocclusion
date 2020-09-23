#ifndef MROCCLUSION_ITERATIVERGBDOCCLUSIONMETHOD_HPP_INCLUDED
#define MROCCLUSION_ITERATIVERGBDOCCLUSIONMETHOD_HPP_INCLUDED

#include "mrocclusion/RgbdOcclusionMethod.hpp"

///\brief Occlusion method using a non-linear least squares optimisation.
class IterativeRgbdOcclusionMethod : public RgbdOcclusionMethod {
public:
	IterativeRgbdOcclusionMethod(
		size_t colorWidth, size_t colorHeight,
		size_t depthWidth, size_t depthHeight);
	virtual ~IterativeRgbdOcclusionMethod() throw();


	virtual void calculateOcclusion(
		const unsigned char *rgbdColor,
		const unsigned short *rgbdDepth,
		const unsigned short *virtualDepth,
		unsigned char * matte);
		
	virtual std::string getName() const { return "Iterative"; }

	bool usePixelsOffVirtualObject() { return useOffVirtualObject_; }
	void usePixelsOffVirtualObject(bool p) { useOffVirtualObject_ = p; }
private:
	bool useOffVirtualObject_;
	float gradientWeight_, colorWeight_;
	int maxIterations_;
	bool printProgress_;
};

#endif
