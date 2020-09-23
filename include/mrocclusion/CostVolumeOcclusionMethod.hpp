#ifndef MROCCLUSION_COSTVOLUMEOCCLUSIONMETHOD_HPP_INCLUDED
#define MROCCLUSION_COSTVOLUMEOCCLUSIONMETHOD_HPP_INCLUDED

#include <mrocclusion/RgbdOcclusionMethod.hpp>
#include <mrocclusion/RgbHistogram.hpp>

void getHistogramSampleLocs(
	const cv::Mat &infront, const cv::Mat &behind, const cv::Mat &process,
	cv::Mat *infrontSampleLocs, cv::Mat *behindSampleLocs, size_t dilateAmount);
bool fitFgBgHistograms(
	const cv::Mat &realColor,
	const cv::Mat &infrontSampleLocs, const cv::Mat &behindSampleLocs,
	RgbHistogram *infrontHist, RgbHistogram *behindHist,
	size_t minSampleCount);

class CostVolumeOcclusionMethod : public RgbdOcclusionMethod {
public:
	enum class CostFilterType {
		GUIDED, SELECTIVE_GUIDED, BILATERAL
	};

	CostVolumeOcclusionMethod(
		size_t colorWidth, size_t colorHeight,
		size_t depthWidth, size_t depthHeight);

	virtual ~CostVolumeOcclusionMethod() throw();

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
		
	virtual std::string getName() const;

	virtual void processEvent(SDL_Event &e);

	CostFilterType costFilterType() const;
	void costFilterType(CostFilterType t);

	size_t nIterations() const { return maxIterations_; }
	void nIterations(size_t n) { maxIterations_ = n; }

	size_t costFilterR() const { return costFilterR_; }
	void costFilterR(size_t n) { costFilterR_ = n; }

	float costFilterEps() const { return costFilterEps_; }
	void costFilterEps(float n) { costFilterEps_ = n; }

	size_t finalFilterR() const { return finalFilterR_; }
	void finalFilterR(size_t n) { finalFilterR_ = n; }

	float finalFilterEps() const { return finalFilterEps_; }
	void finalFilterEps(float n) { finalFilterEps_ = n; }
private:
	CostVolumeOcclusionMethod(const CostVolumeOcclusionMethod&);
	CostVolumeOcclusionMethod& operator=(const CostVolumeOcclusionMethod&);

	void calculateCosts(
		RgbHistogram &infrontHist, RgbHistogram &behindHist,
		const cv::Mat &infront, const cv::Mat &behind, const cv::Mat &process,
		const cv::Mat &realColor,
		cv::Mat *costs);
	void filterCosts(
		const cv::Mat &costs, 
		const cv::Mat &infront, const cv::Mat &behind, const cv::Mat &process,
		const cv::Mat &realColor,
		cv::Mat *filteredCosts);
	void thresholdCosts(
		const cv::Mat &costs,
		const cv::Mat &infront, const cv::Mat &behind, const cv::Mat &process, 
		cv::Mat *binaryMask);

	size_t nHistogramBins_, dilateAmt_;
	size_t costFilterR_;
	float costFilterEps_;
	size_t finalFilterR_;
	size_t maxIterations_;
	float finalFilterEps_;
	bool useOffVirtualObject_, useSelectiveGuidedFilter_;
	CostFilterType costFilterType_;
};

#endif
