#ifndef MROCCLUSION_NAIVERGBDOCCLUSIONMETHOD_HPP_INCLUDED
#define MROCCLUSION_NAIVERGBDOCCLUSIONMETHOD_HPP_INCLUDED

#include "mrocclusion/RgbdOcclusionMethod.hpp"

///\brief Occlusion method using a selective cross bilateral filter.
class NaiveRgbdOcclusionMethod : public RgbdOcclusionMethod {
public:
	NaiveRgbdOcclusionMethod(
		size_t colorWidth, size_t colorHeight,
		size_t depthWidth, size_t depthHeight,
		bool assumeInfront = true);
	virtual ~NaiveRgbdOcclusionMethod() throw();

	virtual void calculateOcclusion(
		const unsigned char *RgbdColor, const unsigned short *RgbdDepth,
		const unsigned short *virtualDepth,
		unsigned char * matte);

	virtual void processEvent(SDL_Event &e);

	virtual std::string getName() const;
private:
	bool assumeInfront_;
};

#endif
