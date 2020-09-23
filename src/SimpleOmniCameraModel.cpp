#include "mrocclusion/SimpleOmniCameraModel.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <mrocclusion/Exception.hpp>


SimpleOmniCameraModel::SimpleOmniCameraModel(
		size_t width, size_t height,
		float fx, float fy, float cx, float cy, float e)
	:CameraModel(width, height),
	fx_(fx), fy_(fy), cx_(cx), cy_(cy), e_(e)
{}

SimpleOmniCameraModel::SimpleOmniCameraModel(const std::string &configFile)
:CameraModel(0,0)
{
	boost::property_tree::ptree config;
	boost::property_tree::json_parser::read_json(configFile, config);
	
	std::string typeStr = config.get<std::string>("type");
	if(typeStr != "SIMPLEOMNI") {
		throw FileException("Supplied config file was not of correct type");
	}
	
	fx_ = config.get<float>("fx");
	fy_ = config.get<float>("fy");
	cx_ = config.get<float>("cx");
	cy_ = config.get<float>("cy");
	e_ = config.get<float>("e");
	width_ = config.get<int>("width");
	height_ = config.get<int>("height");
}

vec3 SimpleOmniCameraModel::pixelToCamSpace(
	const vec2 &pixel, float depth) const
{
	vec2 pn((pixel.y() - cy_) / fy_, (pixel.x() - cx_) / fx_);
	float p2 = pn.x()*pn.x() + pn.y()*pn.y();
	float num = e_ + sqrtf(1.f + (1 - e_*e_)*p2);
	vec3 dir = (vec3(pn.x(), pn.y(), 1.f) * num / (1.f + p2)) - vec3(0.f, 0.f, e_);
	return dir / depth;
}

vec2 SimpleOmniCameraModel::camSpaceToPixel(
	const vec3 &camSpace) const
{
	float den = camSpace.z() + camSpace.norm() * e_;
	vec2 i;
	i.x() = fx_ * (camSpace.x() / den) + cx_;
	i.y() = fy_ * (camSpace.y() / den) + cy_;
	return i;
}

std::string SimpleOmniCameraModel::type() const
{
	return "SIMPLEOMNI";
}

float SimpleOmniCameraModel::fx() const
{
	return fx_;
}
float SimpleOmniCameraModel::fy() const
{
	return fy_;
}
float SimpleOmniCameraModel::cx() const
{
	return cx_;
}
float SimpleOmniCameraModel::cy() const
{
	return cy_;
}
float SimpleOmniCameraModel::e() const
{
	return e_;
}


