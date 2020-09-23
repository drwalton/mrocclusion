#include "mrocclusion/RgbdOcclusionMethod.hpp"
#include "mrocclusion/CrabbOcclusionMethod.hpp"
#include "mrocclusion/BilateralRgbdOcclusionMethod.hpp"
#include "mrocclusion/CostVolumeOcclusionMethod.hpp"
#include "mrocclusion/GpuCvfOcclusionMethod.hpp"
#include "mrocclusion/GrabcutRgbdOcclusionMethod.hpp"
#include "mrocclusion/GuidedRgbdOcclusionMethod.hpp"
#include "mrocclusion/IterativeRgbdOcclusionMethod.hpp"
#include "mrocclusion/NaiveRgbdOcclusionMethod.hpp"
#include "mrocclusion/SelectiveGuidedOcclusionMethod.hpp"
#include "mrocclusion/AdaptiveManifoldOcclusionMethod.hpp"
#include "mrocclusion/AdaptiveBilateralRgbdOcclusionMethod.hpp"
#include "mrocclusion/BaselineBilateralOcclusionMethod.hpp"
#include "mrocclusion/BaselineGuidedOcclusionMethod.hpp"
#include "mrocclusion/BaselineManifoldOcclusionMethod.hpp"
#include "mrocclusion/BlobRemoval.hpp"

RgbdOcclusionMethod::RgbdOcclusionMethod(
	size_t colorWidth, size_t colorHeight, 
	size_t depthWidth, size_t depthHeight)
	:colorWidth(colorWidth), colorHeight(colorHeight),
	depthWidth(depthWidth), depthHeight(depthHeight),
	debugMode(false),
	visualise(false),
	behindErodeAmt_(2), infrontErodeAmt_(2)
{
}

RgbdOcclusionMethod::~RgbdOcclusionMethod() throw()
{
}


void RgbdOcclusionMethod::processEvent(SDL_Event & e)
{}


std::unique_ptr<RgbdOcclusionMethod> RgbdOcclusionMethod::factory(
	const std::string &methodName,
	size_t colorWidth, size_t colorHeight,
	size_t depthWidth, size_t depthHeight)
{
	if(methodName == "Bilateral") {
		return std::unique_ptr<RgbdOcclusionMethod>(
			new BilateralRgbdOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight));
	} else if(methodName == "AdaptiveBilateral") {
		AdaptiveBilateralRgbdOcclusionMethod *ptr = new 
			AdaptiveBilateralRgbdOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight);
		return std::unique_ptr<RgbdOcclusionMethod>(ptr);
	} else if(methodName == "CostVolumeBilateral") {
		CostVolumeOcclusionMethod *ptr = new CostVolumeOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight);
		ptr->costFilterType(CostVolumeOcclusionMethod::CostFilterType::BILATERAL);
		return std::unique_ptr<RgbdOcclusionMethod>(ptr);
	} else if(methodName == "CostVolumeGuided") {
		CostVolumeOcclusionMethod *ptr = new CostVolumeOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight);
		ptr->costFilterType(CostVolumeOcclusionMethod::CostFilterType::GUIDED);
		return std::unique_ptr<RgbdOcclusionMethod>(ptr);
	} else if(methodName == "CostVolumeSelectiveGuided") {
		CostVolumeOcclusionMethod *ptr = new CostVolumeOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight);
		ptr->costFilterType(CostVolumeOcclusionMethod::CostFilterType::SELECTIVE_GUIDED);
		return std::unique_ptr<RgbdOcclusionMethod>(ptr);
	} else if(methodName == "GrabCut"){
		return std::unique_ptr<RgbdOcclusionMethod>(
			new GrabcutRgbdOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight));
	} else if(methodName == "Guided"){
		return std::unique_ptr<RgbdOcclusionMethod>(
			new GuidedRgbdOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight));
	} else if(methodName == "Iterative"){
		return std::unique_ptr<RgbdOcclusionMethod>(
			new IterativeRgbdOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight));
	} else if(methodName == "SelectiveGuided"){
		return std::unique_ptr<RgbdOcclusionMethod>(
			new SelectiveGuidedOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight));
	} else if(methodName == "AdaptiveManifold"){
		return std::unique_ptr<RgbdOcclusionMethod>(
			new AdaptiveManifoldOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight));
	} else if(methodName == "NaiveInfront"){
		return std::unique_ptr<RgbdOcclusionMethod>(
			new NaiveRgbdOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight, true));
	} else if(methodName == "NaiveBehind"){
		return std::unique_ptr<RgbdOcclusionMethod>(
			new NaiveRgbdOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight, false));
	} else {
		throw std::runtime_error("Unrecognised occlusion method name provided"
			" to RgbdOcclusionMethod::factory()");
	}
}

std::unique_ptr<RgbdOcclusionMethod> RgbdOcclusionMethod::factory(const boost::property_tree::ptree & method, size_t colorWidth, size_t colorHeight, size_t depthWidth, size_t depthHeight)
{
	std::string methodName = method.get<std::string>("type");
	if(methodName == "Bilateral") {
		BilateralRgbdOcclusionMethod *m =
			new BilateralRgbdOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight);
		if(method.count("colorSigma")) {
			m->colorSigma(method.get<float>("colorSigma"));
		}
		if(method.count("depthSigma")) {
			m->depthSigma(method.get<float>("depthSigma"));
		}
		if(method.count("filterSize")) {
			m->filterSize(method.get<float>("filterSize"));
		}
		if (method.count("erodeAmt")) {
			m->erodeAmt(method.get<int>("erodeAmt"));
		}
		return std::unique_ptr<RgbdOcclusionMethod>(m);
	} else if(methodName == "AdaptiveBilateral") {
		AdaptiveBilateralRgbdOcclusionMethod *ptr = new 
			AdaptiveBilateralRgbdOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight);
		if (method.count("minFgBgPixels")) {
			ptr->minFgBgPixels(method.get<int>("minFgBgPixels"));
		}
		if (method.count("maxFilterSize")) {
			ptr->maxFilterSize(method.get<int>("maxFilterSize"));
		}
		if (method.count("depthSigma")) {
			ptr->depthSigma(method.get<float>("depthSigma"));
		}
		if (method.count("colorSigma")) {
			ptr->colorSigma(method.get<float>("colorSigma"));
		}
		return std::unique_ptr<RgbdOcclusionMethod>(ptr);
	} else if(methodName == "CostVolumeBilateral") {
		CostVolumeOcclusionMethod *ptr = new CostVolumeOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight);
		ptr->costFilterType(CostVolumeOcclusionMethod::CostFilterType::BILATERAL);
		return std::unique_ptr<RgbdOcclusionMethod>(ptr);
	} else if(methodName == "BaselineBilateral") {
		BaselineBilateralOcclusionMethod *ptr = new BaselineBilateralOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight);
		return std::unique_ptr<RgbdOcclusionMethod>(ptr);
	} else if(methodName == "BaselineManifold") {
		BaselineManifoldOcclusionMethod *ptr = new BaselineManifoldOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight);
		return std::unique_ptr<RgbdOcclusionMethod>(ptr);
	} else if(methodName == "BaselineGuided") {
		BaselineGuidedOcclusionMethod *ptr = new BaselineGuidedOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight);
		return std::unique_ptr<RgbdOcclusionMethod>(ptr);
	} else if(methodName == "Crabb") {
		CrabbOcclusionMethod *ptr = new CrabbOcclusionMethod(
			);
		return std::unique_ptr<RgbdOcclusionMethod>(ptr);
	} else if(methodName == "CostVolumeGuided") {
		CostVolumeOcclusionMethod *ptr = new CostVolumeOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight);
		ptr->costFilterType(CostVolumeOcclusionMethod::CostFilterType::GUIDED);
		if (method.count("costFilterR")) {
			ptr->costFilterR(method.get<int>("costFilterR"));
		}
		if (method.count("costFilterEps")) {
			ptr->costFilterEps(method.get<float>("costFilterEps"));
		}
		if (method.count("finalFilterR")) {
			ptr->finalFilterR(method.get<int>("finalFilterR"));
		}
		if (method.count("finalFilterEps")) {
			ptr->finalFilterEps(method.get<float>("finalFilterEps"));
		}
		if (method.count("erodeAmt")) {
			ptr->erodeAmt(method.get<int>("erodeAmt"));
		}
		return std::unique_ptr<RgbdOcclusionMethod>(ptr);
	} else if(methodName == "CostVolumeGpu") {
		GpuCvfOcclusionMethod *ptr = new GpuCvfOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight);
		if (method.count("nHistogramBins")) {
			ptr->nHistogramBins(method.get<int>("nHistogramBins"));
		}
		if (method.count("costFilterR")) {
			ptr->costFilterR(method.get<int>("costFilterR"));
		}
		if (method.count("costFilterEps")) {
			ptr->costFilterEps(method.get<float>("costFilterEps"));
		}
		if (method.count("finalFilterR")) {
			ptr->finalFilterR(method.get<int>("finalFilterR"));
		}
		if (method.count("finalFilterEps")) {
			ptr->finalFilterEps(method.get<float>("finalFilterEps"));
		}
		if (method.count("erodeAmt")) {
			ptr->erodeAmt(method.get<int>("erodeAmt"));
		}
		return std::unique_ptr<RgbdOcclusionMethod>(ptr);
	} else if(methodName == "CostVolumeSelectiveGuided") {
		CostVolumeOcclusionMethod *ptr = new CostVolumeOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight);
		ptr->costFilterType(CostVolumeOcclusionMethod::CostFilterType::SELECTIVE_GUIDED);
		return std::unique_ptr<RgbdOcclusionMethod>(ptr);
	} else if(methodName == "GrabCut"){
		return std::unique_ptr<RgbdOcclusionMethod>(
			new GrabcutRgbdOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight));
	} else if(methodName == "Guided"){
		return std::unique_ptr<RgbdOcclusionMethod>(
			new GuidedRgbdOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight));
	} else if(methodName == "Iterative"){
		return std::unique_ptr<RgbdOcclusionMethod>(
			new IterativeRgbdOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight));
	} else if(methodName == "SelectiveGuided"){
		return std::unique_ptr<RgbdOcclusionMethod>(
			new SelectiveGuidedOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight));
	} else if(methodName == "AdaptiveManifold"){
		return std::unique_ptr<RgbdOcclusionMethod>(
			new AdaptiveManifoldOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight));
	} else if(methodName == "NaiveInfront"){
		return std::unique_ptr<RgbdOcclusionMethod>(
			new NaiveRgbdOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight, true));
	} else if(methodName == "NaiveBehind"){
		return std::unique_ptr<RgbdOcclusionMethod>(
			new NaiveRgbdOcclusionMethod(
			colorWidth, colorHeight, depthWidth, depthHeight, false));
	} else {
		throw std::runtime_error("Unrecognised occlusion method name provided"
			" to RgbdOcclusionMethod::factory()");
	}
}

void RgbdOcclusionMethod::setDebugMode(bool d)
{
	debugMode = d;
}

void RgbdOcclusionMethod::erodeAmt(size_t a)
{
	infrontErodeAmt(a);
	behindErodeAmt(a);
}

void RgbdOcclusionMethod::infrontErodeAmt(size_t a)
{
	infrontErodeAmt_ = a;
}

void RgbdOcclusionMethod::behindErodeAmt(size_t a)
{
	behindErodeAmt_ = a;
}

void RgbdOcclusionMethod::classifyPixels(
	const cv::Mat &realColor, const cv::Mat &realDepth_,
	const cv::Mat &virtualDepth,
	cv::Mat &process, cv::Mat &behind, cv::Mat &infront,
	bool usePixelsOffVirtualObject)
{
	double minVirtualDepth, maxVirtualDepth;
	cv::minMaxLoc(virtualDepth, &minVirtualDepth, &maxVirtualDepth, nullptr, nullptr, virtualDepth != 0);

	cv::Mat realDepth;
	if (!realColor.empty() && (realColor.size() != realDepth.size())) {
		cv::resize(realDepth_, realDepth, realColor.size());
	}
	else {
		realDepth = realDepth_;
	}

	//Pixels behind virtual object (virtual object not occluded).
	behind = (realDepth > virtualDepth) & (realDepth != 0) & (virtualDepth != 0);

	//Pixels in front of virtual object (virtual object occluded).
	infront = (realDepth < virtualDepth) & (realDepth != 0);

	if (usePixelsOffVirtualObject) {
		behind |= (virtualDepth == 0) & (realDepth > maxVirtualDepth);
		infront |= (virtualDepth == 0) & (realDepth < minVirtualDepth) & (realDepth != 0);
	}

		removeSmallBlobs(behind, 100);


		//Remove pixels far from occlusion edges from  process category
		//{
		//	const size_t MAX_DIST = 65;
		//	const int MAX_BLOB_SIZE = 100;
		//	cv::Mat nearInfront, nearBehind;
		//	cv::Size sz(MAX_DIST, MAX_DIST);
		//	cv::boxFilter(infront, nearInfront, -1, sz);
		//	cv::boxFilter(behind, nearBehind, -1, sz);
		//	infront.setTo(255, process & (nearBehind == 0));
		//	behind.setTo(255, process & (nearInfront == 0));
		//	//removeSmallBlobs(behind, MAX_BLOB_SIZE);
		//	process.setTo(0, infront | behind);
		//	
		//}

	//Initial estimate for process.
	process = ~infront & ~behind & (virtualDepth != 0);
	{
		//Erode the "behind" and "infront" pixels slightly, around the processMask region.
		cv::Mat behindCutter, infrontCutter;
		cv::dilate(process | infront, behindCutter, cv::Mat());
		for (size_t i = 1; i < behindErodeAmt_; ++i) {
			cv::dilate(behindCutter, behindCutter, cv::Mat());
		}
		cv::dilate(process | behind, infrontCutter, cv::Mat());
		for (size_t i = 1; i < infrontErodeAmt_; ++i) {
			cv::dilate(infrontCutter, infrontCutter, cv::Mat());
		}
		behind &= ~behindCutter;
		infront &= ~infrontCutter;
		process = ~infront & ~behind & (virtualDepth != 0);
	}

	if (debugMode) {
		cv::imshow("virtualDepth", virtualDepth != 0);
		cv::imshow("behind", behind);
		cv::imshow("infront", infront);
		cv::imshow("process", process);
		cv::waitKey(1);
	}
}

cv::Mat RgbdOcclusionMethod::makePixelClassesMat(const cv::Mat & process, const cv::Mat & behind, const cv::Mat & infront)
{
	cv::Mat mat(process.size(), CV_8UC3);
	mat.setTo(cv::Scalar(0, 0, 0));
	mat.setTo(cv::Scalar(0, 0, 255), process);
	mat.setTo(cv::Scalar(255, 0, 0), infront);
	mat.setTo(cv::Scalar(0, 255, 0), behind);
	return mat;
}
