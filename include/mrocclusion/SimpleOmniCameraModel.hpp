#ifndef MROCCLUSION_SIMPLEOMNICAMERAMODEL_HPP_INCLUDED
#define MROCCLUSION_SIMPLEOMNICAMERAMODEL_HPP_INCLUDED

#include "mrocclusion/CameraModel.hpp"


//!\brief Simplified omnidirectional camera model.
class SimpleOmniCameraModel : public CameraModel {
public:
	SimpleOmniCameraModel(
		size_t width, size_t height,
		float fx, float fy, float cx, float cy, float e);
	SimpleOmniCameraModel(const std::string &configFile);

	virtual vec3 pixelToCamSpace(
		const vec2 &pixel, float depth = 1.f) const;
		
	virtual vec2 camSpaceToPixel(
		const vec3 &camSpace) const;
		
	virtual std::string type() const;
	
	float fx() const;
	float fy() const;
	float cx() const;
	float cy() const;
	float e() const;
private:
	float fx_, fy_, cx_, cy_, e_;
};


#endif
