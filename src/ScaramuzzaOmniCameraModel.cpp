#include "mrocclusion/ScaramuzzaOmniCameraModel.hpp"

#include <mrocclusion/Exception.hpp>

#include <cfloat>
#include <boost/math/tools/rational.hpp>
#include <boost/current_function.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>


double polyval(const std::vector<float> &coeffts, double x) {
	std::vector<double> dCoeffts;
	for (size_t i = coeffts.size() - 1; i < coeffts.size(); --i) {
		dCoeffts.push_back(coeffts[i]);
	}
	return boost::math::tools::evaluate_polynomial<double>(dCoeffts.data(), x, dCoeffts.size());
}


ScaramuzzaOmniCameraModel::ScaramuzzaOmniCameraModel(
	size_t width, size_t height,
	float c, float d, float e, float xc, float yc,
	const std::vector<float> &polyCoeffts)
	:CameraModel(width, height),
	c_(c), d_(d), e_(e),
	cx_(xc), cy_(yc),
	polyCoeffts_(polyCoeffts)
{
}

ScaramuzzaOmniCameraModel::ScaramuzzaOmniCameraModel(const std::string &configFile)
	:CameraModel(0,0)
{
	boost::property_tree::ptree config;
	boost::property_tree::json_parser::read_json(configFile, config);
	
	std::string typeStr = config.get<std::string>("type");
	if(typeStr != "SCARAMUZZAOMNI") {
		throw FileException("Supplied config file was not of correct type");
	}
	
	cx_ = config.get<float>("cx");
	cy_ = config.get<float>("cy");
	c_ = config.get<float>("c");
	d_ = config.get<float>("d");
	e_ = config.get<float>("e");
	width_ = config.get<int>("width");
	height_ = config.get<int>("height");
	BOOST_FOREACH(
		const boost::property_tree::ptree::value_type& child,
		config.get_child("polyCoeffts")) {
		polyCoeffts_.push_back(child.second.get<float>(""));
	}

	if (config.count("validityMask")) {
		boost::filesystem::path configFilePath(configFile);
		std::string maskFilename = config.get<std::string>("validityMask");
		boost::filesystem::path folder = configFilePath.parent_path();
		cv::Mat mask = cv::imread(folder.string() + "/" + maskFilename);
		std::vector<cv::Mat> channels;
		cv::split(mask, channels);
		validityMask = channels[0];
	}
}

vec3 ScaramuzzaOmniCameraModel::pixelToCamSpace(
	const vec2 &pixel, float depth) const
{
	throw std::runtime_error(BOOST_CURRENT_FUNCTION + std::string("Not implemented!"));
}

vec2 ScaramuzzaOmniCameraModel::camSpaceToPixel(
	const vec3 &camSpace) const
{
	//if (p.z() < 0.f) return vec2(-1.f, -1.f);
	double norm = sqrtl(double(camSpace.x())*double(camSpace.x()) +
		double(camSpace.y())*double(camSpace.y()));
	if (norm == 0.f) {
		norm = DBL_EPSILON;
	}
	else if (norm != norm) {
		return vec2(-1, -1);
	}
	double theta = atan2l(-camSpace.z() , norm);
	double rho = polyval(polyCoeffts_, theta);
	double uPrime = (camSpace.x() / norm) * rho;
	double vPrime = (camSpace.y() / norm) * rho;
	Eigen::Vector2f imPos(
						  uPrime*c_ + vPrime*d_ + cy_,
						  uPrime*e_ + vPrime + cx_);
	return imPos;
}

std::string ScaramuzzaOmniCameraModel::type() const
{
	return "SCARAMUZZAOMNI";
}

float ScaramuzzaOmniCameraModel::cx() const
{
	return cx_;
}
float ScaramuzzaOmniCameraModel::cy() const
{
	return cy_;
}

float ScaramuzzaOmniCameraModel::c() const
{
	return c_;
}
float ScaramuzzaOmniCameraModel::d() const
{
	return d_;
}
float ScaramuzzaOmniCameraModel::e() const
{
	return e_;
}

const std::vector<float> &ScaramuzzaOmniCameraModel::polyCoeffts() const
{
	return polyCoeffts_;
}


