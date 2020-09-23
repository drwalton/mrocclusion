#define _USE_MATH_DEFINES
#include "mrocclusion/MarkerTracker.hpp"
#include <iostream>
#include <ARToolKitPlus/TrackerSingleMarker.h>
#include <mrocclusion/Directories.hpp>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <mrocclusion/Matrices.hpp>
#include <cmath>


const float zFar = 50.f;
const float zNear = 0.01f;

mat4 artoolkitToHqar((mat4() << 
	1, 0, 0, 0,
	0, -1, 0, 0,
	0, 0, -1, 0,
	0, 0, 0, 1).finished());

struct MarkerTracker::Impl {
	ARToolKitPlus::TrackerSingleMarker tracker;
	int thresh;
	int foundMarkerId;
	bool trackingGood;
	
	Impl(const PinholeCameraModel &model, float patternWidth, float borderWidth)
    	:tracker(model.width(), model.height(), 8, 6, 6, 6, 0),
    	trackingGood(false)
	{
		std::string tmpCalibFile = MROCCLUSION_CALIB_DIR + "tmp.txt";
		{
			std::ofstream tmpFile(tmpCalibFile);
			tmpFile <<
				"ARToolKitPlus_CamCal_Rev02\n" <<
				model.width() << " " <<
				model.height() << " " <<
				model.cx() << " " <<
				model.cy() << " " <<
				model.fx() << " " <<
				model.fy() << " 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0";
			tmpFile.close();
		}
		tracker.init(tmpCalibFile.c_str(), zNear, zFar);

		tracker.getCamera()->printSettings();
		tracker.setPatternWidth(patternWidth);
		tracker.setBorderWidth(borderWidth);
		tracker.setThreshold(150);
		tracker.setMarkerMode(ARToolKitPlus::MARKER_ID_SIMPLE);
		tracker.setUndistortionMode(ARToolKitPlus::UNDIST_NONE);
	}
	~Impl() throw()
	{
	}
	
	void processImage(const unsigned char *image)
	{
		std::vector<int> trackResult = tracker.calc(image);
		if(trackResult.size() == 0) {
			trackingGood = false;
			return;
		}
		foundMarkerId = trackResult[0];
		tracker.selectBestMarkerByCf();
		trackingGood = true;
	}
	
	mat4 getModelViewMat() const
	{
		const ARFloat *mat = tracker.getModelViewMatrix();
		mat4 m;
		memcpy(m.data(), mat, 16*sizeof(float));
		m =  artoolkitToHqar * m * artoolkitToHqar * 
			angleAxisMat4(M_PI, vec3(0, 0, 1)) * 
			angleAxisMat4(-M_PI_2, vec3(1, 0, 0));
		return m;
	}
	
	float confidence() const
	{
		if (trackingGood) {
			return tracker.getConfidence();
		} else {
			return 0.f;
		}
	}
};

MarkerTracker::MarkerTracker(const PinholeCameraModel &model,
	float patternWidth, float borderWidth)
	:pimpl_(new Impl(model, patternWidth, borderWidth))
{
}

MarkerTracker::~MarkerTracker() throw()
{}

void MarkerTracker::processImage(const unsigned char *image)
{
	pimpl_->processImage(image);
}

bool MarkerTracker::trackingGood() const
{
	return pimpl_->trackingGood;
}

float MarkerTracker::confidence() const
{
	return pimpl_->confidence();
}

mat4 MarkerTracker::getModelViewMat() const
{
	return pimpl_->getModelViewMat();
}


