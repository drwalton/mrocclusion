#ifndef MROCCLUSION_CAMERAMODEL_HPP_INCLUDED
#define MROCCLUSION_CAMERAMODEL_HPP_INCLUDED

#include <mrocclusion/VectorTypes.hpp>
#include <memory>

//!\brief Abstract class representing a camera model. Can convert from
//!       pixel coordinates to camera space coordinates.
//!\note All pixel coordinates are supplied as a vec2, where the components
//!      are (row, column).
//!\note All camera space coordinates are given with the camera looking along
//!      the positive z-axis, with x being left to right and y being bottom to top.
//!\note To convert to the OpenGL coordinate space, negate the z component.
class CameraModel {
public:
	CameraModel(size_t width, size_t height);

	static std::unique_ptr<CameraModel> fromConfigFile(const std::string &file);
	
	//!\brief Convert a pixel coordinate in image space to a point in camera space.
	virtual vec3 pixelToCamSpace(
		const vec2 &pixel, float depth = 1.f) const = 0;
		
	virtual vec2 camSpaceToPixel(
		const vec3 &camSpace) const = 0;
	
	virtual std::string type() const = 0;

	size_t width() const { return width_; };
	size_t height() const { return height_; };
protected:
	size_t width_, height_;
};

#endif

