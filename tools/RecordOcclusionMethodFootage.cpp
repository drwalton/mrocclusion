#include <mrocclusion/FullScreenQuad.hpp>
#include <mrocclusion/GLWindow.hpp>
#include <mrocclusion/Texture.hpp>
#include <mrocclusion/RenderToTexture.hpp>
#include <mrocclusion/Files.hpp>
#include <mrocclusion/ShaderProgram.hpp>
#include <mrocclusion/Directories.hpp>
#include <mrocclusion/Directories.hpp>
#include <mrocclusion/OpenNiDepthCam.hpp>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <mrocclusion/PinholeCameraModel.hpp>
#include <mrocclusion/MarkerTracker.hpp>
#include <mrocclusion/ArCamera.hpp>
#include <mrocclusion/TexturedPhongMesh.hpp>
#include <OpenNI.h>

enum Status {
	BEFORE_RECORDING,
	BEFORE_GT_CAP,
	BEFORE_KINFU_START,
	BEFORE_CAPTURE,
	DURING_CAPTURE,
	BEFORE_KINFU_STOP
};

Status status = BEFORE_RECORDING;
size_t winWidth = 640*2, winHeight = 480*2;

int main(int argc, char *argv[])
{
	GLWindow win("Record Occlusion Method Footage", winWidth, winHeight);
	bool running = true;

	OpenNiDepthCam depthCam;
	depthCam.autoWhitebalanceEnabled(true);
	depthCam.getLatestFrames();
	depthCam.autoExposureEnabled(true);
	
	PinholeCameraModel depthCamModel(MROCCLUSION_CALIB_DIR + "XtionProLive.json");
	MarkerTracker markerTracker(depthCamModel);
	ARCamera arCamera(depthCamModel);
	TexturedPhongMesh mesh(
		MROCCLUSION_MODEL_DIR + "mannequin.jpg",
		MROCCLUSION_MODEL_DIR + "teapot.obj");

	//Set up filenames for this recording.
	//The files consist of an openni recording and a text file
	//indicating frame numbers on which events occur.
	std::stringstream logEntry;
	std::string time = uniqueTimestamp();
	std::string baseName = MROCCLUSION_RECORDING_DIR + "/" + time;
	std::string vidName = baseName + ".oni";
	std::string logName = baseName + ".txt";
	std::string gtColorName = baseName + "_gtColor.png";
	std::string gtDepthName = baseName + "_gtDepth.png";
	std::string gtGlColor = baseName + "_gtGlColor.png";
	std::string gtGlMask = baseName + "_gtGlMask.png";
	std::string txtToShow = "Ready to start recording.";
	bool groundTruthCaptured = false;
	std::ofstream logFile(logName);

	Texture rgbTexture(
		GL_TEXTURE_2D, GL_RGB8,
		depthCam.colorWidth(),
		depthCam.colorHeight(),
		0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	Texture depthTexture(
		GL_TEXTURE_2D, GL_R16UI,
		depthCam.depthWidth(),
		depthCam.depthHeight(),
		0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, nullptr);
	RenderToTexture virtualRgbTexture(
		GL_RGB8,
		depthCam.colorWidth(),
		depthCam.colorHeight(),
		0, GL_RGB, GL_UNSIGNED_BYTE);
	RenderToTexture virtualDepthTexture(
		GL_R16UI,
		depthCam.colorWidth(),
		depthCam.colorHeight(),
		0, GL_RED_INTEGER, GL_UNSIGNED_SHORT);
	cv::Mat glColor(cv::Size(depthCam.colorWidth(), depthCam.colorHeight()), CV_8UC3),
		glDepth(cv::Size(depthCam.colorWidth(), depthCam.colorHeight()), CV_16UC1),
		gtMask;

	ShaderProgram showImageShader(std::vector<std::string> {
		MROCCLUSION_SHADER_DIR + "FullScreenTex.vert",
		MROCCLUSION_SHADER_DIR + "FullScreenTexFlip.frag"
	});
	ShaderProgram showDepthShader(std::vector<std::string> {
		MROCCLUSION_SHADER_DIR + "FullScreenTex.vert",
		MROCCLUSION_SHADER_DIR + "FullScreenDepthTexFlip.frag"
	});
	ShaderProgram drawVirtualDepthShader(std::vector<std::string> {
		MROCCLUSION_SHADER_DIR + "RenderVirtualDepth.vert",
		MROCCLUSION_SHADER_DIR + "RenderVirtualDepth.frag"
	});
	ShaderProgram addIfDepthNonZeroShader(std::vector<std::string> {
		MROCCLUSION_SHADER_DIR + "FullScreenTex.vert",
		MROCCLUSION_SHADER_DIR + "FullScreenDepthDepthNonZero.frag"
	});
	showImageShader.setUniform("tex", 0);
	showDepthShader.setUniform("tex", 0);
	addIfDepthNonZeroShader.setUniform("tex", 0);
	addIfDepthNonZeroShader.setUniform("depthTex", 1);
	glClearColor(0.1f, 0.1f, 0.1f, 1.f);

	SDL_Event event;
	size_t frameNo = 0;
	const int frames_to_capture = 12;
	int captured_frames = 0;
	while(running) {
		glViewport(0, 0, 640*2, 480*2);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		depthCam.getLatestFrames();
		rgbTexture.update(depthCam.colorFramePtr());
		depthTexture.update(depthCam.depthFramePtr());
		
		markerTracker.processImage(depthCam.colorFramePtr());
		if (markerTracker.trackingGood()) {
			arCamera.updatePose(markerTracker.getModelViewMat());
		}

		glViewport(0, 480, 640, 480);
		rgbTexture.bindToImageUnit(0);
		FullScreenQuad::getInstance().render(showImageShader);

		glViewport(640, 480, 640, 480);
		depthTexture.bindToImageUnit(0);
		FullScreenQuad::getInstance().render(showDepthShader);

		glViewport(0, 0, 640, 480);
		rgbTexture.bindToImageUnit(0);
		FullScreenQuad::getInstance().render(showImageShader);

		win.updateText(txtToShow);
		win.drawText(30, 30);
		win.swapBuffers();

		arCamera.bindCameraBlock();
		virtualRgbTexture.setAsRenderTarget();
		mesh.render();
		virtualDepthTexture.setAsRenderTarget();
		mesh.render(&drawVirtualDepthShader);
		virtualDepthTexture.unsetAsRenderTarget();

		//Render virtual content into app.
		virtualRgbTexture.texture().bindToImageUnit(0);
		virtualDepthTexture.texture().bindToImageUnit(1);
		glViewport(0, 0, 640, 480);
		FullScreenQuad::getInstance().render(addIfDepthNonZeroShader);


		if (status != BEFORE_RECORDING) {
			++frameNo;
		}

		if (status == DURING_CAPTURE) {
			virtualRgbTexture.texture().getData(glColor.data, glColor.rows*glColor.cols * 3);
			virtualDepthTexture.texture().getData(glDepth.data, glColor.rows*glColor.cols * 2);
			cv::imwrite(baseName + "_glColor" +
				std::to_string(frameNo) + ".png", glColor);
			cv::imwrite(baseName + "_glDepth" +
				std::to_string(frameNo) + ".png", glDepth);
			++captured_frames;
			gtMask = glDepth > 0;

			if (captured_frames >= frames_to_capture) {
				captured_frames = 0;
				logFile << "E\n" << frameNo << "\n";
				txtToShow = "Capture finished at frame " +
					std::to_string(frameNo) + ". Waiting to capture ground truth...";
				status = BEFORE_GT_CAP;
			}
		}

		while (SDL_PollEvent(&event)) {
			if (GLWindow::eventIsQuit(event)) {
				running = false;
			}
			if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_SPACE) {
					if (status == BEFORE_RECORDING) {
						status = BEFORE_KINFU_START;
						depthCam.startRecording(vidName);
						txtToShow = "Started recording. Ready to start kinfu.";
					} else if (status == BEFORE_KINFU_START) {
						logFile << "K\n" << frameNo << "\n";
						txtToShow = "Started kinfu at frame " +
							std::to_string(frameNo) + ". Waiting to capture...";
						status = BEFORE_CAPTURE;
					} else if (status == BEFORE_CAPTURE) {
						logFile << "C\n" << frameNo << "\n";
						txtToShow = "Captured frame " +
							std::to_string(frameNo) + ".  Please wait for capture...";
						status = DURING_CAPTURE;
					}
				}
			}
		}

	}

	return 0;
}
