#ifndef MROCCLUSION_GRABCUTRGBDOCCLUSIONMETHOD_HPP_INCLUDED
#define MROCCLUSION_GRABCUTRGBDOCCLUSIONMETHOD_HPP_INCLUDED

#include "mrocclusion/RgbdOcclusionMethod.hpp"

///\brief Occlusion method using the grabcuts algorithm.
///\note The matte returned by this method will contain values of 
///      0 or 255 only.
class GrabcutRgbdOcclusionMethod : public RgbdOcclusionMethod {
public:
	GrabcutRgbdOcclusionMethod(
		size_t colorWidth, size_t colorHeight,
		size_t depthWidth, size_t depthHeight);
	virtual ~GrabcutRgbdOcclusionMethod() throw();

	virtual void calculateOcclusion(
		const unsigned char *rgbdColor, 
		const unsigned short *rgbdDepth,
		const unsigned short *virtualDepth,
		unsigned char * matte);
	
	bool usePixelsOffVirtualObject() const { return useOffVirtualObject_; }
	void usePixelsOffVirtualObject(bool p) { useOffVirtualObject_ = p; }
	
	virtual std::string getName() const { return "GrabCut"; }

	virtual void processEvent(SDL_Event &event);
	
	int iters() const {return iters_;}
	void iters(int i) { iters_ = i;}
private:
	bool useOffVirtualObject_;
	int iters_;
};

#endif
