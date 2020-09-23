#include "mrocclusion/InftamRgbdSlam.hpp"

#include <ITMLib.h>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <Eigen/Dense>

const mat4 infiniTAMToOpenGL((mat4() <<
	1, 0, 0, 0,
	0, -1, 0, 0,
	0, 0, -1, 0,
	0, 0, 0, 1
	).finished());

const mat4 openGLToInfiniTAM((mat4() <<
	1, 0, 0, 0,
	0, -1, 0, 0,
	0, 0, -1, 0,
	0, 0, 0, 1
	).finished());

void rgb888ToRgba8888(
	const unsigned char *rgb, unsigned char *rgba, size_t nPixels)
{
	for (size_t i = 0; i < nPixels; ++i) {
		memcpy(rgba + i * 4, rgb + i * 3, 3);
		rgba[i * 4 + 3] = 255;
	}
}
ITMPose gluePoseToItm(const WorldToCamTransform p)
{
	mat4 itmMatGlue = openGLToInfiniTAM * p * infiniTAMToOpenGL;
	Matrix4f itmMat;
	memcpy(&itmMat, itmMatGlue.data(), 16 * sizeof(float));
	return ITMPose(itmMat);
}
WorldToCamTransform itmPoseToGlue(const ITMPose &p)
{
	Matrix4f itmMat = p.GetM();
	mat4 itmMatGlue;
	memcpy(itmMatGlue.data(), &itmMat, 16 * sizeof(float));
	return
		infiniTAMToOpenGL *
		itmMatGlue *
		openGLToInfiniTAM;
}

Matrix3f glueMat3ToItam(const mat3 &m)
{
	Matrix3f im;
	memcpy(&im, m.data(), 9 * sizeof(float));
	return im;
}

ITMIntrinsics glueIntrinsicsToItm(const PinholeCameraModel &m)
{
	ITMIntrinsics intrinsics;
	intrinsics.SetFrom(
		m.fx(),
		m.fy(),
		m.cx(),
		m.cy(),
		m.width(),
		m.height()
	);
	return intrinsics;
}

struct InftamRgbdSlam::Impl
{
	Impl(const PinholeCameraModel &colorCamModel,
		const PinholeCameraModel &depthCamModel,
		const mat4 &rgbToDepth,
		bool meshingEnabled,
		bool useImu)
		:nColorPixels(colorCamModel.width() * colorCamModel.height()),
		nDepthPixels(depthCamModel.width() * depthCamModel.height()),
		colorImage(
			Vector2i(colorCamModel.width(), colorCamModel.height()), MEMORYDEVICE_CPU),
		raycastImage(
			Vector2i(colorCamModel.width(), colorCamModel.height()), MEMORYDEVICE_CPU),
		freeRaycastImage(
			Vector2i(colorCamModel.width(), colorCamModel.height()), MEMORYDEVICE_CPU),
		depthImage(
			Vector2i(depthCamModel.width(), depthCamModel.height()), MEMORYDEVICE_CPU),
		raycastResultImage(
			Vector2i(depthCamModel.width(), depthCamModel.height()), MEMORYDEVICE_CPU)
	{
#ifdef __APPLE__
		settings.deviceType = ITMLibSettings::DEVICE_CPU;
#else
		settings.deviceType = ITMLibSettings::DEVICE_CUDA;
#endif
		if (useImu) {
			settings.trackerType = ITMLibSettings::TrackerType::TRACKER_IMU;
		}
		settings.useMeshingEngine = meshingEnabled;

		std::cout << "Infinitam stuff:\n"
			<< "\tfx: " << rgbdCalib.intrinsics_d.projectionParamsSimple.fx
			<< "\n\tfy: " << rgbdCalib.intrinsics_d.projectionParamsSimple.fy
			<< "\n\tcx: " << rgbdCalib.intrinsics_d.projectionParamsSimple.px
			<< "\n\tcy: " << rgbdCalib.intrinsics_d.projectionParamsSimple.py << std::endl;
		std::cout << "My stuff\n"
			<< "\tfx: " << depthCamModel.fx()
			<< "\n\tfy: " << depthCamModel.fy()
			<< "\n\tcx: " << depthCamModel.cx()
			<< "\n\tcy: " << depthCamModel.cy() << std::endl;

		
		rgbdCalib.intrinsics_d.projectionParamsSimple.fx = depthCamModel.fx();
		rgbdCalib.intrinsics_d.projectionParamsSimple.fy = depthCamModel.fy();
		rgbdCalib.intrinsics_d.projectionParamsSimple.px = depthCamModel.cx();
		rgbdCalib.intrinsics_d.projectionParamsSimple.py = depthCamModel.cy();
		rgbdCalib.intrinsics_rgb.projectionParamsSimple.fx = depthCamModel.fx();
		rgbdCalib.intrinsics_rgb.projectionParamsSimple.fy = depthCamModel.fy();
		rgbdCalib.intrinsics_rgb.projectionParamsSimple.px = depthCamModel.cx();
		rgbdCalib.intrinsics_rgb.projectionParamsSimple.py = depthCamModel.cy();

		Vector2i imageSize_rgb(colorCamModel.width(), colorCamModel.height());
		Vector2i imageSize_d(depthCamModel.width(), depthCamModel.height());

		engine.reset(new ITMMainEngine(&settings, &rgbdCalib, imageSize_rgb, imageSize_d));
	}


	void processFrames(
		const unsigned char *colorFrame,
		const unsigned short *depthFrame,
		const mat3 *imuMeasurement)
	{
		rgb888ToRgba8888(colorFrame,
			reinterpret_cast<unsigned char*>(
				colorImage.GetData(MEMORYDEVICE_CPU)), nColorPixels);
		memcpy(depthImage.GetData(MEMORYDEVICE_CPU),
			depthFrame, nDepthPixels * sizeof(unsigned short));
		if (imuMeasurement == nullptr) {
			engine->ProcessFrame(&colorImage, &depthImage);
		} else {
			Matrix3f r = glueMat3ToItam(*imuMeasurement);
			ITMIMUMeasurement itamImu(r);
			engine->ProcessFrame(&colorImage, &depthImage, &itamImu);
		}
	}

	WorldToCamTransform currTransform()
	{
		ITMTrackingState *state = engine->GetTrackingState();
		return itmPoseToGlue(*state->pose_d);
	}

	bool trackingGood()
	{
		return currTransform() == currTransform();
	}

	const unsigned char *reconstructionRaycast()
	{
		//engine->GetImage(&raycastImage,
		//	ITMMainEngine::InfiniTAM_IMAGE_ORIGINAL_RGB);

		//cv::Mat im1(cv::Size(640, 480), CV_8UC4,
		//	raycastImage.GetData(MEMORYDEVICE_CPU));
		//cv::imshow("input", im1);

		ITMTrackingState *state = engine->GetTrackingState();
		engine->GetImage(&raycastImage,
			ITMMainEngine::InfiniTAM_IMAGE_SCENERAYCAST,
			state->pose_d, &rgbdCalib.intrinsics_d);

		//cv::Mat im(cv::Size(640, 480), CV_8UC4,
		//	raycastImage.GetData(MEMORYDEVICE_CPU));
		//cv::imshow("reconstruction", im);
		//cv::waitKey(1);
		return reinterpret_cast<const unsigned char*>(
			raycastImage.GetData(MEMORYDEVICE_CPU));
	}
	virtual const unsigned char *reconstructionRaycast(
		const WorldToCamTransform &camPose,
		const PinholeCameraModel &camModel)
	{
		ITMPose pose = gluePoseToItm(camPose);
		ITMIntrinsics intrinsics = glueIntrinsicsToItm(camModel);
		if (freeRaycastImage.noDims[0] != camModel.width() ||
			freeRaycastImage.noDims[1] != camModel.height()) {
			Vector2i newSize(camModel.width(), camModel.height());
			freeRaycastImage.ChangeDims(newSize);
		}
		engine->GetImage(&freeRaycastImage,
			ITMMainEngine::InfiniTAM_IMAGE_FREECAMERA_COLOUR_FROM_VOLUME,
			&pose, &intrinsics);
		return reinterpret_cast<const unsigned char*>(
			freeRaycastImage.GetData(MEMORYDEVICE_CPU));
	}

	void meshScene()
	{
		auto t = std::chrono::system_clock::now();
		engine->UpdateMesh();
		auto d = std::chrono::system_clock::now() - t;
		std::cout << "Meshing took " << std::chrono::duration_cast<std::chrono::milliseconds>(d).count() << std::endl;
	}

	void saveSceneToMesh(const std::string &filename)
	{
		engine->SaveSceneToMesh(filename.c_str());
	}

	size_t nColorPixels, nDepthPixels;
	ITMLibSettings settings;
	ITMRGBDCalib rgbdCalib;
	std::unique_ptr<ITMMainEngine> engine;
	ITMUChar4Image colorImage, raycastImage, freeRaycastImage;
	ITMFloat4Image raycastResultImage;
	ITMShortImage depthImage;
	std::unique_ptr<unsigned short[]> expectedDepths;
};

InftamRgbdSlam::InftamRgbdSlam(
	const PinholeCameraModel &colorCamModel,
	const PinholeCameraModel &depthCamModel,
	const mat4 &rgbToDepth,
	bool meshingEnabled,
	bool useImu)
	:RgbdSlam(colorCamModel, depthCamModel, rgbToDepth),
	pimpl_(new Impl(colorCamModel, depthCamModel, rgbToDepth, meshingEnabled, useImu))
{}

InftamRgbdSlam::~InftamRgbdSlam() throw()
{}

void InftamRgbdSlam::processFrames(
	const unsigned char *colorFrame,
	const unsigned short *depthFrame,
	const mat3 *imuMeasurement)
{
	pimpl_->processFrames(colorFrame, depthFrame, imuMeasurement);
}

WorldToCamTransform InftamRgbdSlam::currTransform()
{
	return pimpl_->currTransform();
}

bool InftamRgbdSlam::trackingGood()
{
	return pimpl_->trackingGood();
}

const unsigned char *InftamRgbdSlam::reconstructionRaycast()
{
	return pimpl_->reconstructionRaycast();
}

const unsigned char * InftamRgbdSlam::reconstructionRaycast(
	const WorldToCamTransform &camPose, const PinholeCameraModel &camModel)
{
	return pimpl_->reconstructionRaycast(camPose, camModel);
}

vec4 *InftamRgbdSlam::pointCloudPositions()
{
	//TODO test, check if we need to call cloud->updateHostFromDevice() or something.
	ITMPointCloud *cloud = pimpl_->engine->GetTrackingState()->pointCloud;
	Vector4f *locations = cloud->locations->GetData(MEMORYDEVICE_CPU);
	return reinterpret_cast<vec4*>(locations);
}

vec4 *InftamRgbdSlam::pointCloudColors()
{
	ITMPointCloud *cloud = pimpl_->engine->GetTrackingState()->pointCloud;
	Vector4f *colors = cloud->colours->GetData(MEMORYDEVICE_CPU);
	return reinterpret_cast<vec4*>(colors);
}

unsigned short *InftamRgbdSlam::expectedDepthMap()
{
	if(!pimpl_->expectedDepths) {
		pimpl_->expectedDepths.reset(new unsigned short[pimpl_->nDepthPixels]);
	}
	ITMTrackingState *state = pimpl_->engine->GetTrackingState();
	/*
	pimpl_->engine->GetImage(&pimpl_->raycastImage,
		ITMMainEngine::InfiniTAM_IMAGE_SCENERAYCAST,
		state->pose_d, &pimpl_->rgbdCalib.intrinsics_d);
	*/
	ITMRenderState *renderState = pimpl_->engine->renderState_live;
	if(state) {
		pimpl_->raycastResultImage.ChangeDims(Vector2i(640, 480));
		pimpl_->raycastResultImage.SetFrom(renderState->raycastResult, ORUtils::MemoryBlock<Vector4f>::CUDA_TO_CPU);

		vec4 *result = reinterpret_cast<vec4*>(
			pimpl_->raycastResultImage.GetData(MEMORYDEVICE_CPU));
		if (result) {
			mat4 transform = currTransform().inverse();
			for (size_t i = 0; i < pimpl_->nDepthPixels; ++i) {
				if (result[i].w() != 0) {
					vec4 camSpace = transform * result[i];
					
					pimpl_->expectedDepths[i] = static_cast<unsigned short>(
						camSpace.z()*5.f);
				} else {
					pimpl_->expectedDepths[i] = 0;
				}
			}
		}
	}
	/*
	vec4 *positions = pointCloudPositions();
	for(size_t i = 0; i < pimpl_->nDepthPixels; ++i) {
		vec4 pos = positions[i];
		float fltDepth = pos.z();
		unsigned short depth = static_cast<unsigned short>(fltDepth*1000.f);
		pimpl_->expectedDepths[i] = depth;
		if(i == 61619) {
			std::cout << "A" << std::endl;
		}
		//TODO find, handle invalid pixels.
	}
	*/
	return pimpl_->expectedDepths.get();
}

void InftamRgbdSlam::doIntegration(bool i)
{
	if (!i) {
		pimpl_->engine->turnOffIntegration();
	} else {
		pimpl_->engine->turnOnIntegration();
	}
}

void InftamRgbdSlam::saveSceneToMesh(const std::string &filename)
{
	pimpl_->saveSceneToMesh(filename);
}

void InftamRgbdSlam::meshScene()
{
	pimpl_->meshScene();
}

