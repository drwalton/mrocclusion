#include <mrocclusion/FullScreenQuad.hpp>
#include <mrocclusion/GLWindow.hpp>
#include <mrocclusion/Texture.hpp>
#include <mrocclusion/RenderToTexture.hpp>
#include <mrocclusion/Files.hpp>
#include <mrocclusion/ShaderProgram.hpp>
#include <mrocclusion/AntTweakBar.hpp>
#include <mrocclusion/Directories.hpp>
#include <mrocclusion/Directories.hpp>
#include <mrocclusion/OpenNiDepthCam.hpp>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <mrocclusion/PinholeCameraModel.hpp>
#include <mrocclusion/Comparison.hpp>
#include <mrocclusion/GpuCvfOcclusionMethod.hpp>
#include <mrocclusion/MarkerTracker.hpp>
#include <mrocclusion/ArCamera.hpp>
#include <mrocclusion/TexturedPhongMesh.hpp>
#include <OpenNI.h>
#include <boost/property_tree/json_parser.hpp>
#include <mrocclusion/Matrices.hpp>
#include <chrono>

size_t winWidth = 640*2, winHeight = 480*2;

int main(int argc, char *argv[])
{
	if(argc <= 1) {
		std::cout << "Usage: $ OcclusionMethodsLive [config]" << std::endl;
		return 1;
	}

	GLWindow win("Record Occlusion Method Footage", winWidth, winHeight);
	bool running = true;

	OpenNiDepthCam depthCam;
	depthCam.autoWhitebalanceEnabled(true);
	depthCam.getLatestFrames();
	depthCam.autoExposureEnabled(true);
	
	PinholeCameraModel depthCamModel(MROCCLUSION_CALIB_DIR + "XtionProLive.json");

	GpuCvfOcclusionMethod occlusionMethod(depthCam.colorWidth(), depthCam.colorHeight(),
		depthCam.depthWidth(), depthCam.depthHeight());
	float costEps = occlusionMethod.costFilterEps(), finalEps = occlusionMethod.finalFilterEps();
	int costR = occlusionMethod.costFilterR(), finalR = occlusionMethod.finalFilterR();

	AntTweakBar tweakBar(&win);
	tweakBar.addVarRW("costEps", &costEps, "costEps", 0.f, 100.f, 0.001f, "Regularisation for cost filter");
	tweakBar.addVarRW("finalEps", &finalEps, "finalEps", 0.f, 300.f, 0.001f, "Regularisation for final filter");
	tweakBar.addVarRW("costR", &costR, "costR", 0, 50, "Radius of cost filter");
	tweakBar.addVarRW("finalR", &finalR, "finalR", 0, 50, "Radius of final filter");

	boost::property_tree::ptree config;
	boost::property_tree::json_parser::read_json(argv[1], config);
	MarkerTracker markerTracker(depthCamModel, 
		config.get<float>("patternWidth"),
		config.get<float>("borderWidth"));
	ARCamera arCamera(depthCamModel);
	TexturedPhongMesh mesh(
		MROCCLUSION_MODEL_DIR + "mannequin.jpg",
		MROCCLUSION_MODEL_DIR + "teapot.obj");
	mesh.modelToWorld(translateMat4(vec3(0.f, 0.075f, 0.f)));
	mesh.drawShadow(true);
	mesh.shadowColor(vec4(0.9f, 0.9f, 0.9f, 0.f));
	bool tweakBarHidden = false;

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
	Texture matteTexture(
		GL_TEXTURE_2D, GL_R8,
		depthCam.colorWidth(),
		depthCam.colorHeight(),
		0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
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
		gtMask,
		matte(cv::Size(depthCam.colorWidth(), depthCam.colorHeight()), CV_8UC1);

	ShaderProgram showImageShader(std::vector<std::string> {
		MROCCLUSION_SHADER_DIR + "FullScreenTex.vert",
		MROCCLUSION_SHADER_DIR + "FullScreenTexFlip.frag"
	});
	ShaderProgram showDepthShader(std::vector<std::string> {
		MROCCLUSION_SHADER_DIR + "FullScreenTex.vert",
		MROCCLUSION_SHADER_DIR + "FullScreenDepthTexFlip.frag"
	});
	ShaderProgram showVirtualDepthShader(std::vector<std::string> {
		MROCCLUSION_SHADER_DIR + "FullScreenTex.vert",
		MROCCLUSION_SHADER_DIR + "FullScreenDepthTex.frag"
	});
	ShaderProgram drawVirtualDepthShader(std::vector<std::string> {
		MROCCLUSION_SHADER_DIR + "RenderVirtualDepth.vert",
		MROCCLUSION_SHADER_DIR + "RenderVirtualDepth.frag"
	});
	ShaderProgram addIfDepthNonZeroShader(std::vector<std::string> {
		MROCCLUSION_SHADER_DIR + "FullScreenTex.vert",
		MROCCLUSION_SHADER_DIR + "FullScreenTex_DepthNonZero.frag"
	});
	ShaderProgram composeShader(std::vector<std::string> {
		MROCCLUSION_SHADER_DIR + "FullScreenTex.vert",
		MROCCLUSION_SHADER_DIR + "FullScreenTex_ComposeOcclusion.frag"
	});
	showImageShader.setUniform("tex", 0);
	showDepthShader.setUniform("tex", 0);
	showVirtualDepthShader.setUniform("tex", 0);
	addIfDepthNonZeroShader.setUniform("tex", 0);
	addIfDepthNonZeroShader.setUniform("depthTex", 1);
	composeShader.setUniform("realTex", 0);
	composeShader.setUniform("matte", 1);
	composeShader.setUniform("virtualTex", 2);

	SDL_Event event;
	size_t frameNo = 0;
	const int frames_to_capture = 12;
	int captured_frames = 0;
	while(running) {
		glViewport(0, 0, 640*2, 480*2);
		glClearColor(0.1f, 0.1f, 0.1f, 1.f);
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
		glDepthMask(GL_FALSE);
		FullScreenQuad::getInstance().render(showImageShader);
		glDepthMask(GL_TRUE);

		glEnable(GL_DEPTH_TEST);
		arCamera.bindCameraBlock();
		virtualRgbTexture.setAsRenderTarget();
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_BLEND);
		mesh.render();
		virtualDepthTexture.setAsRenderTarget();
		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		drawVirtualDepthShader.setUniform("worldToCam", markerTracker.getModelViewMat());
		throwOnGlError();
		mesh.render(&drawVirtualDepthShader);
		virtualDepthTexture.unsetAsRenderTarget();

		//Render virtual content into app.
		virtualRgbTexture.texture().bindToImageUnit(0);
		virtualDepthTexture.texture().bindToImageUnit(1);
		glViewport(0, 0, 640, 480);
		glClear(GL_DEPTH_BUFFER_BIT);
		FullScreenQuad::getInstance().render(addIfDepthNonZeroShader);


		virtualDepthTexture.texture().getData(glDepth.data, glColor.rows*glColor.cols * 2);
		cv::flip(glDepth, glDepth, 0);

		occlusionMethod.costFilterEps(costEps);
		occlusionMethod.costFilterR(costR);
		occlusionMethod.finalFilterEps(finalEps);
		occlusionMethod.finalFilterR(finalR);
		auto t = std::chrono::system_clock::now();
		occlusionMethod.calculateOcclusion(
			depthCam.colorFramePtr(), depthCam.depthFramePtr(),
			(unsigned short*)(glDepth.data), &matteTexture);
		auto d = std::chrono::system_clock::now() - t;


		glFinish();
		cv::Mat matteMat(cv::Size(640, 480), CV_8UC1);
		matteTexture.getData(matteMat.data, 640 * 480);
		cv::imshow("Matte", matteMat);
		cv::waitKey(1);
		glFinish();
		glViewport(640, 0, 640, 480);
		rgbTexture.bindToImageUnit(0);
		matteTexture.bindToImageUnit(1);
		virtualRgbTexture.texture().bindToImageUnit(2);
		FullScreenQuad::getInstance().render(composeShader);

		glViewport(0, 480, 640, 480);
		virtualDepthTexture.texture().bindToImageUnit(0);
		FullScreenQuad::getInstance().render(showVirtualDepthShader);

		std::stringstream txt;
		txt << (markerTracker.trackingGood() ? "Tracking Good " : "Tracking Lost ")
			<< std::chrono::duration_cast<std::chrono::milliseconds>(d).count() << "ms";
		win.setTitle(txt.str());

		if (!tweakBarHidden) {
			tweakBar.draw();
		}

		win.swapBuffers();

		while (SDL_PollEvent(&event)) {
			if (GLWindow::eventIsQuit(event)) {
				running = false;
			}

			if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_h) {
					tweakBarHidden = !tweakBarHidden;
				}
			}

			occlusionMethod.processEvent(event);
			tweakBar.processEvent(event);
		}

	}

	return 0;
}
