#pragma once

#include "mrocclusion/DepthCam.hpp"
#include <memory>
#include <string>

namespace openni {
	class VideoStream;
}

//!\brief Implementation of DepthCam using OpenNI camera.
class OpenNiDepthCam : public DepthCam
{
public:
	explicit OpenNiDepthCam();
	explicit OpenNiDepthCam(const std::string &uri);
	virtual ~OpenNiDepthCam() throw();

	virtual bool getLatestFrames();
	virtual const unsigned short *const depthFramePtr() const;
	virtual const unsigned char  *const colorFramePtr() const;
	virtual const unsigned short *const irFramePtr() const;

	virtual size_t depthWidth() const;
	virtual size_t depthHeight() const;
	virtual size_t colorWidth() const;
	virtual size_t colorHeight() const;

	virtual vec2 getDepthFov() const;
	virtual vec2 getColorFov() const;

	openni::VideoStream &depthStream();
	openni::VideoStream &colorStream();
	const openni::VideoStream &depthStream() const;
	const openni::VideoStream &colorStream() const;

	virtual void startColor();
	virtual void stopColor();
	virtual void startDepth();
	virtual void stopDepth();
	virtual void startIr();
	virtual void stopIr();

	void scaleUpColorImage(bool s);
	bool scaleUpColorImage() const;

	virtual void setExposure(int e);
	virtual int exposure() const;

	virtual void autoExposureEnabled(bool);
	virtual bool autoExposureEnabled() const;

	virtual void autoWhitebalanceEnabled(bool w);
	bool autoWhitebalanceEnabled() const;

	void registerDepthToColor(bool enabled);

	virtual void saveCalibrationImages(const std::string &baseFilename);
	
	void startRecording(const std::string &filename);
	void stopRecording();
	
	void seek(size_t frameIdx);
	
	unsigned int depthFrameTimestamp();
	unsigned int colorFrameTimestamp();
private:
	struct Impl;
	std::unique_ptr<Impl> pimpl_;
};

