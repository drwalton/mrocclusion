#ifndef MROCCLUSION_OCCLUSION_ERRORMETRICS_HPP_INCLUDED
#define MROCCLUSION_OCCLUSION_ERRORMETRICS_HPP_INCLUDED

#include <cstdlib>
#include <string>

class ErrorMetric
{
public:
	ErrorMetric();
	virtual ~ErrorMetric() throw();
	
	virtual double error(unsigned char *matte, unsigned char *groundTruth,
		size_t width, size_t height) const = 0;
	virtual std::string name() const = 0;
};

class SadErrorMetric final : public ErrorMetric
{
public:
	SadErrorMetric();
	virtual ~SadErrorMetric() throw();
	
	virtual double error(unsigned char *matte, unsigned char *groundTruth,
		size_t width, size_t height) const;
	virtual std::string name() const;
};

class MseErrorMetric final : public ErrorMetric
{
public:
	MseErrorMetric();
	virtual ~MseErrorMetric() throw();
	
	virtual double error(unsigned char *matte, unsigned char *groundTruth,
		size_t width, size_t height) const;
	virtual std::string name() const;
};

class GradientErrorMetric final : public ErrorMetric
{
public:
	GradientErrorMetric();
	virtual ~GradientErrorMetric() throw();
	
	virtual double error(unsigned char *matte, unsigned char *groundTruth,
		size_t width, size_t height) const;
	virtual std::string name() const;
};

#endif
