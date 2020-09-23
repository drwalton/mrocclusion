#include "mrocclusion/RgbHistogram.hpp"
#include <cstring>

RgbHistogram::RgbHistogram(size_t binSize)
	:nBins_((1 << 8) / binSize),
	binSize_(binSize), 
	valid_(false),
	nVals_(0)
{
	probabilities_.resize(nBins_*nBins_*nBins_);
	memset(probabilities_.data(), 0, probabilities_.size()*sizeof(float));
}

RgbHistogram::~RgbHistogram() throw()
{}

void RgbHistogram::addRgbVal(const unsigned char *rgb)
{
	probabilities_[rgb2Index(rgb)] += 1.f;
	nVals_ += 1.f;
}

void RgbHistogram::calcProbabilities()
{
	for(size_t i = 0; i < probabilities_.size(); ++i) {
		probabilities_[i] /= nVals_;
	}
	valid_ = true;
}

bool RgbHistogram::valid() const
{
	return valid_;
}

size_t RgbHistogram::rgb2Index(const unsigned char *rgb) const
{
	size_t i = rgb[0] / binSize_;
	size_t j = rgb[1] / binSize_;
	size_t k = rgb[2] / binSize_;
	size_t index = i*nBins_*nBins_ + j*nBins_ + k;
	return index;
}

float RgbHistogram::probability(const unsigned char *val) const
{
	return probabilities_[rgb2Index(val)];
}

size_t RgbHistogram::nBins() const
{
	return nBins_;
}

size_t RgbHistogram::nSamples() const
{
	return nVals_;
}
