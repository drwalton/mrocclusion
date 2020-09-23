#ifndef MROCCLUSION_RGBHISTOGRAM_HPP_INCLUDED
#define MROCCLUSION_RGBHISTOGRAM_HPP_INCLUDED

#include <vector>

///\brief Class for fitting a histogram to a set of RGB values.
class RgbHistogram final
{
public:
	explicit RgbHistogram(size_t binWidth);
	~RgbHistogram() throw();
	
	void addRgbVal(const unsigned char *val);
	void calcProbabilities();

	bool valid() const;
	
	float probability(const unsigned char *val) const;

	size_t nBins() const;
	size_t nSamples() const;
private:
	size_t rgb2Index(const unsigned char *rgb) const;

	std::vector<float> probabilities_;
	size_t nBins_, binSize_;
	float nVals_;
	bool valid_;
};

#endif
