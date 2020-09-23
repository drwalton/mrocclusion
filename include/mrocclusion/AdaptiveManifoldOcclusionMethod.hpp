#ifndef MROCCLUSION_ADAPTIVEMANIFOLDOCCLUSIONMETHOD_HPP_INCLUDED
#define MROCCLUSION_ADAPTIVEMANIFOLDOCCLUSIONMETHOD_HPP_INCLUDED

#include "mrocclusion/RgbdOcclusionMethod.hpp"

///\brief Method for finding occlusion matte given output from a real RGBD
///       camera, as well as the depth of virtual content to be added
///       to the scene.
class AdaptiveManifoldOcclusionMethod : public RgbdOcclusionMethod, private NonCopyable {
public:
	AdaptiveManifoldOcclusionMethod(
		size_t colorWidth, size_t colorHeight,
		size_t depthWidth, size_t depthHeight);

	virtual ~AdaptiveManifoldOcclusionMethod() throw();

	float r() const;
	float s() const;
	void r(float r);
	void s(float e);

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
		
	virtual std::string getName() const { return "AdaptiveManifold"; }

	virtual void processEvent(SDL_Event &event);
protected:
private:
	float r_;
	float s_;
};

#endif
