#ifndef MROCCLUSION_RGBDOCCLUSIONMETHOD_HPP_INCLUDED
#define MROCCLUSION_RGBDOCCLUSIONMETHOD_HPP_INCLUDED

#include "opencv2/opencv.hpp"
#include "mrocclusion/NonCopyable.hpp"
#include <boost/property_tree/ptree.hpp>
#include <SDL.h>
#include <memory>

///\brief Method for finding occlusion matte given output from a real RGBD
///       camera, as well as the depth of virtual content to be added
///       to the scene.
class RgbdOcclusionMethod : private NonCopyable {
public:
	RgbdOcclusionMethod(
		size_t colorWidth, size_t colorHeight,
		size_t depthWidth, size_t depthHeight);

	virtual ~RgbdOcclusionMethod() throw();

	///\brief Generate and return the occlusion matte.
	///\param rgbdColor Real RGBD color image (RGB888 format).
	///\param rgbdDepth Real RGBD depth image (16-bit unsigned int, depth in mm).
	///                 Should be registered to the color image.
	///\param virtualDepth Virtual depth image (16-bit unsigned int, depth in mm).
	///\param[out] matte Output matte (8-bit unsigned integer matte).
	///                  255 indicates virtual object only, 0 indicates real only.
	virtual void calculateOcclusion(
		const unsigned char *rgbdColor,
		const unsigned short *rgbdDepth,
		const unsigned short *virtualDepth,
		unsigned char * matte) = 0;

	virtual void processEvent(SDL_Event &e);
	
	virtual std::string getName() const = 0;
	
	static std::unique_ptr<RgbdOcclusionMethod> factory(
		const std::string &methodName,
    	size_t colorWidth, size_t colorHeight,
    	size_t depthWidth, size_t depthHeight);

	static std::unique_ptr<RgbdOcclusionMethod> factory(
		const boost::property_tree::ptree &method,
    	size_t colorWidth, size_t colorHeight,
    	size_t depthWidth, size_t depthHeight);

	void setDebugMode(bool d);

	void erodeAmt(size_t a);
	void infrontErodeAmt(size_t a);
	void behindErodeAmt(size_t a);
	bool visualise;

protected:
	size_t colorWidth, colorHeight;
	size_t depthWidth, depthHeight;
	bool debugMode;
	void classifyPixels(
		const cv::Mat & realColor, const cv::Mat & realDepth_,
		const cv::Mat & virtualDepth,
		cv::Mat & process, cv::Mat & behind, cv::Mat & infront,
		bool usePixelsOffVirtualObject);
	cv::Mat makePixelClassesMat(const cv::Mat & process, const cv::Mat &behind, const cv::Mat &infront);

private:
	size_t behindErodeAmt_, infrontErodeAmt_;
	RgbdOcclusionMethod(const RgbdOcclusionMethod&);
	RgbdOcclusionMethod& operator=(const RgbdOcclusionMethod&);
};

#endif
