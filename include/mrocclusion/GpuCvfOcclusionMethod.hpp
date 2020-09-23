#ifndef MROCCLUSION_GPUCVFOCCLUSIONMETHOD_HPP_INCLUDED
#define MROCCLUSION_GPUCVFOCCLUSIONMETHOD_HPP_INCLUDED

#include <mrocclusion/RgbdOcclusionMethod.hpp>
#include <mrocclusion/RgbHistogram.hpp>
#include <mrocclusion/GuidedFilterGpu.hpp>
#include <mrocclusion/Texture.hpp>
#include <mrocclusion/ShaderProgram.hpp>
#include <mrocclusion/CostVolumeOcclusionMethod.hpp>

class GpuCvfOcclusionMethod : public RgbdOcclusionMethod {
public:
	GpuCvfOcclusionMethod(
		size_t colorWidth, size_t colorHeight,
		size_t depthWidth, size_t depthHeight);

	virtual ~GpuCvfOcclusionMethod() throw();

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
		unsigned char * matte);

	void calculateOcclusion(
		const unsigned char *rgbdColor,
		const unsigned short *rgbdDepth,
		const unsigned short *virtualDepth,
		Texture * matte);
		
	virtual std::string getName() const;

	virtual void processEvent(SDL_Event &e);

	size_t nIterations() const { return maxIterations_; }
	void nIterations(size_t n) { maxIterations_ = n; }

	size_t costFilterR() const { return costFilterR_; }
	void costFilterR(size_t n) { costFilterR_ = n; }

	float costFilterEps() const { return costFilterEps_; }
	void costFilterEps(float n) { costFilterEps_ = n; }

	size_t finalFilterR() const { return finalFilterR_; }
	void finalFilterR(size_t n) { finalFilterR_ = n; }

	size_t nHistogramBins() const { return nHistogramBins_; }
	void nHistogramBins(size_t n) { nHistogramBins_ = n; }

	float finalFilterEps() const { return finalFilterEps_; }
	void finalFilterEps(float n) { finalFilterEps_ = n; }
private:
	GpuCvfOcclusionMethod(const GpuCvfOcclusionMethod&);
	GpuCvfOcclusionMethod& operator=(const GpuCvfOcclusionMethod&);

	void calculateCosts(
		RgbHistogram &infrontHist, RgbHistogram &behindHist,
		const cv::Mat &infront, const cv::Mat &behind, const cv::Mat &process,
		const cv::Mat &realColor,
		cv::Mat *costs);
	GuidedFilterGpu filter;
	ShaderProgram snapToZeroOne;
	Texture matteTex, colorTex, zeroOneTex;

	size_t nHistogramBins_, dilateAmt_;
	size_t costFilterR_;
	float costFilterEps_;
	size_t finalFilterR_;
	size_t maxIterations_;
	float finalFilterEps_;
	bool useOffVirtualObject_, useSelectiveGuidedFilter_;
};

#endif
