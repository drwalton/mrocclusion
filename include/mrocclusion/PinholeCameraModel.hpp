#ifndef MROCCLUSION_PINHOLECAMERAMODEL_HPP_INCLUDED
#define MROCCLUSION_PINHOLECAMERAMODEL_HPP_INCLUDED

#include "mrocclusion/CameraModel.hpp"

//!\brief Simple pinhole camera model (no distortion)
class PinholeCameraModel : public CameraModel {
public:
	PinholeCameraModel(size_t width, size_t height,
		float fx, float fy, float cx, float cy);
	PinholeCameraModel(const std::string &configFile);

	virtual vec3 pixelToCamSpace(
		const vec2 &pixel, float depth = 1.f) const;
		
	virtual vec2 camSpaceToPixel(
		const vec3 &camSpace) const;
		
	virtual std::string type() const;
	
	float fx() const;
	float fy() const;
	float cx() const;
	float cy() const;
private:
	float fx_, fy_, cx_, cy_;
};

#endif

