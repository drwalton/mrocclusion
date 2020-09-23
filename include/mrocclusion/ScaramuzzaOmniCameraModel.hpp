#ifndef MROCCLUSION_SCARAMUZZAOMNICAMERAMODEL_HPP_INCLUDED
#define MROCCLUSION_SCARAMUZZAOMNICAMERAMODEL_HPP_INCLUDED

#include "mrocclusion/CameraModel.hpp"
#include <opencv2/opencv.hpp>

//!\brief Camera model of Scaramuzza et al.
//!       See https://sites.google.com/site/scarabotix/ocamcalib-toolbox
class ScaramuzzaOmniCameraModel : public CameraModel {
public:
	ScaramuzzaOmniCameraModel(
		size_t width, size_t height,
    	float c, float d, float e, float xc, float yc,
    	const std::vector<float> &polyCoeffts);
	ScaramuzzaOmniCameraModel(const std::string &configFile);

	virtual vec3 pixelToCamSpace(
		const vec2 &pixel, float depth = 1.f) const;
		
	virtual vec2 camSpaceToPixel(
		const vec3 &camSpace) const;
		
	virtual std::string type() const;
	
	float cx() const;
	float cy() const;
	
	float c() const;
	float d() const;
	float e() const;
	
	const std::vector<float> &polyCoeffts() const;

	cv::Mat validityMask;
private:
	float c_, d_, e_, cx_, cy_;
	std::vector<float> polyCoeffts_;
};

#endif
