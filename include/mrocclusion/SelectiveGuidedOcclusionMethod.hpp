#ifndef MROCCLUSION_SELECTIVEGUIDEDOCCLUSIONMETHOD_HPP_INCLUDED
#define MROCCLUSION_SELECTIVEGUIDEDOCCLUSIONMETHOD_HPP_INCLUDED

#include <mrocclusion/RgbdOcclusionMethod.hpp>
#include <mrocclusion/RgbHistogram.hpp>

class SelectiveGuidedOcclusionMethod : public RgbdOcclusionMethod {
public:
	SelectiveGuidedOcclusionMethod(
		size_t colorWidth, size_t colorHeight,
		size_t depthWidth, size_t depthHeight);

	virtual ~SelectiveGuidedOcclusionMethod() throw();

	///\brief Generate and return the occlusion matte.
	///\param rgbdColor Real RGBD color image (RGB888 format).
	///\param rgbdDepth Real RGBD depth image (16-bit unsigned int, depth in mm).
	///                 Should be registered to the color image.
	///\param virtualDepth Virtual depth image (16-bit unsigned int, depth in mm).
	///\param[out] matte Output matte (8-bit unsigned integer matte).
	///                  255 indicates virtual object only, 0 indicates real only.
	virtual void calculateOcclusion(
		const unsigned char *rgbdColor,
		const unsigned short *rgbdDepth,
		const unsigned short *virtualDepth,
		unsigned char * matte);
		
	virtual std::string getName() const { return "SelectiveGuided"; }

	virtual void processEvent(SDL_Event &e);

	size_t r() const;
	float eps() const;

	void r(size_t r);
	void eps(float e);
	
private:
	SelectiveGuidedOcclusionMethod(const SelectiveGuidedOcclusionMethod&);
	SelectiveGuidedOcclusionMethod& operator=(const SelectiveGuidedOcclusionMethod&);

	size_t r_;
	float eps_;
	bool useOffVirtualObject_;
};

#endif
