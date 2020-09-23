#ifndef DEPTHCAM_HPP_INCLUDED
#define DEPTHCAM_HPP_INCLUDED

#include <memory>
#include "Exception.hpp"
#include <mrocclusion/VectorTypes.hpp>

//!\brief Abstract class defining an RGBD camera.
class DepthCam
{
public:
	virtual ~DepthCam() throw();

	virtual bool getLatestFrames() = 0;
	
	//!\brief Return a pointer to the most recently acquired depth frame.
	virtual const unsigned short *const depthFramePtr() const = 0;
	
	//!\brief Return a pointer to the most recently acquired color frame.
	//!\note The color image is given as a contiguous array of RGB888 values.
	virtual const unsigned char  *const colorFramePtr() const = 0;
	
	//!\brief Return a pointer to the most recently acquired IR frame.
	virtual const unsigned short *const irFramePtr()    const = 0;

	virtual void startColor() = 0;
	virtual void stopColor() = 0;
	virtual void startDepth() = 0;
	virtual void stopDepth() = 0;
	virtual void startIr() = 0;
	virtual void stopIr() = 0;

	virtual size_t depthWidth() const = 0;
	virtual size_t depthHeight() const = 0;
	virtual size_t colorWidth() const = 0;
	virtual size_t colorHeight() const = 0;

	virtual vec2 getDepthFov() const = 0;
	virtual vec2 getColorFov() const = 0;

	virtual void setExposure(int e) {}
	virtual int exposure() const = 0;

	virtual void autoExposureEnabled(bool) {}
	virtual bool autoExposureEnabled() const { return true; }

	virtual void autoWhitebalanceEnabled(bool w) {}
	virtual bool autoWhitebalanceEnabled() const { return true; }

	virtual void saveCalibrationImages(const std::string &filename);
protected:
	explicit DepthCam();
};

std::unique_ptr<DepthCam> openDefaultDepthCam();

#endif
