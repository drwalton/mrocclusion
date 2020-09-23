#include "mrocclusion/CameraModel.hpp"
#include "mrocclusion/PinholeCameraModel.hpp"
#include "mrocclusion/SimpleOmniCameraModel.hpp"
#include "mrocclusion/ScaramuzzaOmniCameraModel.hpp"

#include <mrocclusion/Exception.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

std::unique_ptr<CameraModel> CameraModel::fromConfigFile(const std::string &file)
{
	boost::property_tree::ptree config;
	boost::property_tree::json_parser::read_json(file, config);
	
	std::string type = config.get<std::string>("type");
	
	if(type == "PINHOLE") {
		return std::unique_ptr<CameraModel>(
			new PinholeCameraModel(file));
	} else if(type == "SIMPLEOMNI") {
		return std::unique_ptr<CameraModel>(
			new SimpleOmniCameraModel(file));
	} else if(type == "SCARAMUZZAOMNI") {
		return std::unique_ptr<CameraModel>(
			new ScaramuzzaOmniCameraModel(file));
	}
	
	throw FileException("Supplied camera model not of known type!");
	return nullptr;
}

CameraModel::CameraModel(size_t width, size_t height)
	:width_(width), height_(height)
{}

