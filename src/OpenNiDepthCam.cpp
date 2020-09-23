#include "mrocclusion/OpenNiDepthCam.hpp"

#ifdef _WIN32
#include <OpenNI.h>
#define SCALEUPCOLORIMAGE
#elif defined __linux__
#include <OpenNI.h>
#else
#include <ni2/OpenNI.h>
#endif

#include <opencv2/opencv.hpp>

struct OpenNiDepthCam::Impl
{
	openni::Device device;
	openni::VideoStream depth;
	openni::VideoStream color;
	openni::VideoStream ir;
	openni::VideoFrameRef depthFrame;
	openni::VideoFrameRef colorFrame;
	openni::VideoFrameRef irFrame;
	openni::Recorder recorder;
	bool colorRunning;
	bool depthRunning;
	bool scaleUpColorImage;
	cv::Mat scaledUpColorImage;
	bool irRunning;
	int noFrames, currFrame;

	void init(const std::string &uri)
	{
		openni::OpenNI::initialize();
		openni::Status status;
		if (uri != std::string("")) {
			status = device.open(uri.c_str());
		} else {
			status = device.open(openni::ANY_DEVICE);
		}
		if(status != ONI_STATUS_OK) {
			throw CameraException("Unable to open OpenNI device with URI \""
				+ uri + "\"");
		}
		depth.create(device, openni::SENSOR_DEPTH);
		color.create(device, openni::SENSOR_COLOR);
		ir.create(device, openni::SENSOR_IR);
		//device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);
		device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_OFF);

		currFrame = 0;
		if (uri != std::string("")) {
			device.getPlaybackControl()->setSpeed(-1.f);
			noFrames = device.getPlaybackControl()->getNumberOfFrames(depth);
	 	} else {
			noFrames = -1;
		}

		if(!device.isFile()) {
    		openni::VideoMode dm = depth.getVideoMode();
    		dm.setResolution(640, 480);
    		depth.setMirroringEnabled(false);
    		depth.setVideoMode(dm);

    		openni::VideoMode cm = color.getVideoMode();
#ifdef SCALEUPCOLORIMAGE
		cm.setResolution(320, 240);
		scaleUpColorImage = true;
#else //_WIN32
		scaleUpColorImage = false;
		cm.setResolution(640, 480);
#endif //_WIN32
    		color.setMirroringEnabled(false);
    		color.getCameraSettings()->setAutoExposureEnabled(true);
    		color.setVideoMode(cm);

    		openni::VideoMode im = ir.getVideoMode();
    		im.setResolution(640, 480);
    		ir.setMirroringEnabled(false);
    		ir.setVideoMode(im);
		}

		depth.start();
		color.start();
		depthRunning = true;
		colorRunning = true;
		irRunning = false;
	}
};

OpenNiDepthCam::OpenNiDepthCam(const std::string &uri)
:pimpl_(new Impl())
{
	pimpl_->init(uri);
	//registerDepthToColor(false);
}

OpenNiDepthCam::OpenNiDepthCam()
: pimpl_(new Impl())
{
	pimpl_->init("");
}

OpenNiDepthCam::~OpenNiDepthCam() throw()
{
	pimpl_->depth.stop();
	pimpl_->color.stop();
	pimpl_->depth.destroy();
	pimpl_->color.destroy();
	openni::OpenNI::shutdown();
}

bool OpenNiDepthCam::getLatestFrames()
{
	if (pimpl_->noFrames != -1) {
		if (pimpl_->currFrame >= pimpl_->noFrames - 1) {
			//Have run out of frames in video mode.
			return false;
		}
	}
	if (pimpl_->depthRunning)
		pimpl_->depth.readFrame(&pimpl_->depthFrame);
	if (pimpl_->colorRunning)	{
		pimpl_->color.readFrame(&pimpl_->colorFrame);
		if (pimpl_->scaleUpColorImage) {
			cv::Mat colorImage(cv::Size(320, 240), CV_8UC3,
				(void*)(pimpl_->colorFrame.getData()));
			cv::resize(colorImage, pimpl_->scaledUpColorImage, cv::Size(640, 480));
		}
	}
	if (pimpl_->irRunning)
		pimpl_->ir.readFrame(&pimpl_->irFrame);

	++pimpl_->currFrame;
	return true;
}

const unsigned short *const OpenNiDepthCam::depthFramePtr() const
{
	return reinterpret_cast<const unsigned short*>(pimpl_->depthFrame.getData());
}

const unsigned char *const OpenNiDepthCam::colorFramePtr() const
{
	if (pimpl_->scaleUpColorImage) {
		return pimpl_->scaledUpColorImage.data;
	} else {
		return reinterpret_cast<const unsigned char*>(pimpl_->colorFrame.getData());
	}
}

const unsigned short *const OpenNiDepthCam::irFramePtr() const
{
	return reinterpret_cast<const unsigned short*>(pimpl_->irFrame.getData());
}

size_t OpenNiDepthCam::depthWidth() const
{
	openni::VideoMode m = pimpl_->depth.getVideoMode();
	return m.getResolutionX();
}
size_t OpenNiDepthCam::depthHeight() const
{
	openni::VideoMode m = pimpl_->depth.getVideoMode();
	return m.getResolutionY();
}
size_t OpenNiDepthCam::colorWidth() const
{
	openni::VideoMode m = pimpl_->color.getVideoMode();
	if (scaleUpColorImage()) {
		return m.getResolutionX() * 2;
	}
	else {
		return m.getResolutionX();
	}

}
size_t OpenNiDepthCam::colorHeight() const
{
	openni::VideoMode m = pimpl_->color.getVideoMode();
	if (scaleUpColorImage()) {
		return m.getResolutionY() * 2;
	}
	else {
		return m.getResolutionY();
	}
}

vec2 OpenNiDepthCam::getDepthFov() const
{
	return vec2(
		depthStream().getHorizontalFieldOfView(),
		depthStream().getVerticalFieldOfView());
}
vec2 OpenNiDepthCam::getColorFov() const
{
	return vec2(
		colorStream().getHorizontalFieldOfView(),
		colorStream().getVerticalFieldOfView());
}

openni::VideoStream &OpenNiDepthCam::depthStream()
{
	return pimpl_->depth;
}
openni::VideoStream &OpenNiDepthCam::colorStream()
{
	return pimpl_->color;
}
const openni::VideoStream &OpenNiDepthCam::depthStream() const
{
	return pimpl_->depth;
}
const openni::VideoStream &OpenNiDepthCam::colorStream() const
{
	return pimpl_->color;
}

void OpenNiDepthCam::scaleUpColorImage(bool s)
{
	pimpl_->scaleUpColorImage = s;
	if (s) {
		stopColor();
		auto vm = pimpl_->color.getVideoMode();
		vm.setResolution(320, 240);
		pimpl_->color.setVideoMode(vm);
		startColor();
	} else {
		stopColor();
		auto vm = pimpl_->color.getVideoMode();
		vm.setResolution(640, 480);
		pimpl_->color.setVideoMode(vm);
		startColor();
	}
}

bool OpenNiDepthCam::scaleUpColorImage() const
{
	return pimpl_->scaleUpColorImage;
}

void OpenNiDepthCam::startColor()
{
	pimpl_->color.start();
	pimpl_->colorRunning = true;
}

void OpenNiDepthCam::stopColor()
{
	pimpl_->color.stop();
	pimpl_->colorRunning = false;
}
void OpenNiDepthCam::startDepth()
{
	pimpl_->depth.start();
	pimpl_->depthRunning = true;
}
void OpenNiDepthCam::stopDepth()
{
	pimpl_->depth.stop();
	pimpl_->depthRunning = false;
}
void OpenNiDepthCam::startIr()
{
	pimpl_->ir.start();
	pimpl_->irRunning = true;
}
void OpenNiDepthCam::stopIr()
{
	pimpl_->ir.stop();
	pimpl_->irRunning = false;
}

void OpenNiDepthCam::saveCalibrationImages(const std::string &basefilename)
{
	if (pimpl_->colorRunning) {
		pimpl_->color.readFrame(&pimpl_->colorFrame);
		pimpl_->color.stop();
		pimpl_->ir.start();
		pimpl_->ir.readFrame(&pimpl_->irFrame);
		pimpl_->ir.stop();
		pimpl_->color.start();
	}
	else {
		pimpl_->ir.stop();
		pimpl_->color.start();
		//Allow white balance to work for a few frames.
		for (size_t i = 0; i < 10; ++i) {
			pimpl_->color.readFrame(&pimpl_->colorFrame);
		}
		pimpl_->color.stop();
		pimpl_->ir.start();
		pimpl_->ir.readFrame(&pimpl_->irFrame);
	}
	cv::Mat irFrameIm(cv::Size(640, 480), CV_16UC1,
		const_cast<void*>(pimpl_->irFrame.getData()));
	static cv::Mat scaleIrFrameIm(irFrameIm.size(), CV_32FC1);
	irFrameIm.convertTo(scaleIrFrameIm, CV_32FC1);
	//scaleIrFrameIm *= 255.f / float(USHRT_MAX);
	static cv::Mat ucharIrFrameIm(irFrameIm.size(), CV_8UC1);
	scaleIrFrameIm.convertTo(ucharIrFrameIm, CV_8UC1);
	cv::Mat colorFrameIm(cv::Size(640, 480), CV_8UC3,
		const_cast<void*>(pimpl_->colorFrame.getData()));
	static cv::Mat colorFrameImBgr(colorFrameIm.size(), CV_8UC3);
	cv::cvtColor(colorFrameIm, colorFrameImBgr, cv::COLOR_RGB2BGR);
	cv::imwrite(basefilename + "_color.png", colorFrameImBgr);
	cv::imwrite(basefilename + "_ir.png", ucharIrFrameIm);
}

void OpenNiDepthCam::setExposure(int e)
{
	openni::CameraSettings *settings = pimpl_->color.getCameraSettings();
	settings->setAutoExposureEnabled(false);
	settings->setExposure(e);
	//settings->setGain(e);
	std::cout << "OpenNI Exposure: " << e << std::endl;
}

void OpenNiDepthCam::autoWhitebalanceEnabled(bool w)
{
	openni::CameraSettings *settings = pimpl_->color.getCameraSettings();
	settings->setAutoWhiteBalanceEnabled(w);
	std::cout << "OpenNI White Balance: " << w << std::endl;
}

void OpenNiDepthCam::autoExposureEnabled(bool e)
{
	openni::CameraSettings *settings = pimpl_->color.getCameraSettings();
	settings->setAutoExposureEnabled(e);
}

bool OpenNiDepthCam::autoExposureEnabled() const
{
	openni::CameraSettings *settings = pimpl_->color.getCameraSettings();
	return settings->getAutoExposureEnabled();
}

int OpenNiDepthCam::exposure() const
{
	openni::CameraSettings *settings = pimpl_->color.getCameraSettings();
	return settings->getExposure();
}

bool OpenNiDepthCam::autoWhitebalanceEnabled() const
{
	openni::CameraSettings *settings = pimpl_->color.getCameraSettings();
	return settings->getAutoWhiteBalanceEnabled();
}

void OpenNiDepthCam::registerDepthToColor(bool e)
{
	if (e) {
		pimpl_->device.setImageRegistrationMode(
			openni::ImageRegistrationMode::IMAGE_REGISTRATION_DEPTH_TO_COLOR);
	} else {
		pimpl_->device.setImageRegistrationMode(
			openni::ImageRegistrationMode::IMAGE_REGISTRATION_OFF);
	}
}

void OpenNiDepthCam::startRecording(const std::string &filename)
{
	pimpl_->recorder.create(filename.c_str());
	pimpl_->recorder.attach(pimpl_->depth);
	pimpl_->recorder.attach(pimpl_->color);
	pimpl_->recorder.start();
}

void OpenNiDepthCam::stopRecording()
{
	pimpl_->recorder.stop();
	pimpl_->recorder.destroy();
}

void OpenNiDepthCam::seek(size_t frameIdx)
{
	openni::PlaybackControl *p = pimpl_->device.getPlaybackControl();
	p->seek(pimpl_->color, frameIdx); //note that this also syncs the depth stream!
}

uint OpenNiDepthCam::depthFrameTimestamp()
{
	return pimpl_->depthFrame.getTimestamp();
}

uint OpenNiDepthCam::colorFrameTimestamp()
{
	return pimpl_->colorFrame.getTimestamp();
}

