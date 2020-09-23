#include "mrocclusion/PinholeCameraModel.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <mrocclusion/Exception.hpp>


PinholeCameraModel::PinholeCameraModel(
    size_t width, size_t height,
    float fx, float fy, float cx, float cy)
	:CameraModel(width, height),
	fx_(fx), fy_(fy), cx_(cx), cy_(cy)
{}
PinholeCameraModel::PinholeCameraModel(const std::string &configFile)
	:CameraModel(0,0)
{
	boost::property_tree::ptree config;
	boost::property_tree::json_parser::read_json(configFile, config);
	
	std::string typeStr = config.get<std::string>("type");
	if(typeStr != "PINHOLE") {
		throw FileException("Supplied config file was not of correct type");
	}
	
	fx_ = config.get<float>("fx");
	fy_ = config.get<float>("fy");
	cx_ = config.get<float>("cx");
	cy_ = config.get<float>("cy");
	width_ = config.get<int>("width");
	height_ = config.get<int>("height");
}

vec3 PinholeCameraModel::pixelToCamSpace(
	const vec2 &pixel, float depth) const
{
	return vec3(
		depth*(pixel.y() - cx_)/fx_,
		depth*((height_ - pixel.x()) - cy_)/fy_,
		depth);
}

vec2 PinholeCameraModel::camSpaceToPixel(
	const vec3 &camSpace) const
{
	return vec2(
    	(fx_*camSpace.x())/camSpace.z() + cx_,
    	height_ - (fy_*camSpace.y())/camSpace.z() + cy_);
}

std::string PinholeCameraModel::type() const
{
	return "PINHOLE";
}

float PinholeCameraModel::fx() const
{
	return fx_;
}
float PinholeCameraModel::fy() const
{
	return fy_;
}
float PinholeCameraModel::cx() const
{
	return cx_;
}
float PinholeCameraModel::cy() const
{
	return cy_;
}


