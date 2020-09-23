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
#include <mrocclusion/Comparison.hpp>
#include <mrocclusion/RgbdOcclusionMethod.hpp>
#include <mrocclusion/NaiveRgbdOcclusionMethod.hpp>
#include <mrocclusion/MarkerTracker.hpp>
#include <mrocclusion/ArCamera.hpp>
#include <mrocclusion/TexturedPhongMesh.hpp>
#include <OpenNI.h>
#include <boost/property_tree/json_parser.hpp>
#include <mrocclusion/Matrices.hpp>
#include <mrocclusion/AntTweakBar.hpp>
#include <chrono>

size_t winWidth = 640*2, winHeight = 480*2;

enum class Mode {
	DEPTHS_AND_OUTPUT,
	WITH_AND_WITHOUT,
	OUTPUT_ONLY
};

int main(int argc, char *argv[])
{
	Mode mode = Mode::DEPTHS_AND_OUTPUT;

	if (argc <= 1) {
		std::cout << "Usage: $ OcclusionMethodsLive [config]" << std::endl;
		return 1;
	}

	GLWindow win("Live Occlusion Method Comparison", winWidth, winHeight);
	bool running = true;
	bool visualise = false;
	std::unique_ptr<OpenNiDepthCam> depthCam;

	if (argc > 2) {
		depthCam.reset(new OpenNiDepthCam(argv[2]));
	} else {
		depthCam.reset(new OpenNiDepthCam());
		depthCam->autoWhitebalanceEnabled(true);
		depthCam->registerDepthToColor(true);
		depthCam->getLatestFrames();
		depthCam->autoExposureEnabled(true);

	}

	AntTweakBar tweakBar(&win);
	std::string modeText = "Output, Real/Virtual Depths";

	PinholeCameraModel depthCamModel(MROCCLUSION_CALIB_DIR + "XtionProLive.json");

	boost::property_tree::ptree config;
	boost::property_tree::json_parser::read_json(argv[1], config);
	std::vector<std::unique_ptr<RgbdOcclusionMethod> > occlusionMethods;
	loadOcclusionMethods(config, &occlusionMethods,
		depthCam->colorWidth(), depthCam->colorHeight(),
		depthCam->depthWidth(), depthCam->depthHeight());
	NaiveRgbdOcclusionMethod naiveOcclusionMethod(
		depthCam->colorWidth(), depthCam->colorHeight(),
		depthCam->depthWidth(), depthCam->depthHeight());
	int occlusionMethodIdx = 0;

	MarkerTracker markerTracker(depthCamModel,
		config.get<float>("patternWidth"),
		config.get<float>("borderWidth"));
	ARCamera arCamera(depthCamModel);

	std::cout << "Loading mesh..."; std::cout.flush();
	TexturedPhongMesh mannequinMesh(
		MROCCLUSION_MODEL_DIR + "mannequin.jpg",
		MROCCLUSION_MODEL_DIR + "teapot.obj");
	mannequinMesh.modelToWorld(translateMat4(vec3(0.f, 0.075f, 0.3f))* rotationAboutAxis(M_PI_2, vec3(0, 1, 0)));

	TexturedPhongMesh dragonMesh(
		MROCCLUSION_MODEL_DIR + "dragon.jpg",
		MROCCLUSION_MODEL_DIR + "dragon2.obj");
	dragonMesh.modelToWorld(translateMat4(vec3(0.f, 0.027f, 0.3f))* rotationAboutAxis(-M_PI_2, vec3(0, 1, 0)));
	dragonMesh.drawShadow(false);
	TexturedPhongMesh laptopMesh(
		MROCCLUSION_MODEL_DIR + "laptop.jpg",
		MROCCLUSION_MODEL_DIR + "laptop.obj");
	laptopMesh.drawShadow(true);
	laptopMesh.modelToWorld(translateMat4(vec3(0.f, 0.05f, 0.3f)));

	std::vector<TexturedPhongMesh*> meshes{
		&laptopMesh, &dragonMesh, &mannequinMesh
	};
	int currMeshIdx = 0;

	std::cout << " Meshes loaded!" << std::endl;

	std::cout << "Loading textures..."; std::cout.flush();
	Texture rgbTexture(
		GL_TEXTURE_2D, GL_RGB8,
		depthCam->colorWidth(),
		depthCam->colorHeight(),
		0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	std::cout << " RGB "; std::cout.flush();
	Texture depthTexture(
		GL_TEXTURE_2D, GL_R16UI,
		depthCam->depthWidth(),
		depthCam->depthHeight(),
		0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, nullptr);
	std::cout << " depth "; std::cout.flush();
	Texture matteTexture(
		GL_TEXTURE_2D, GL_R8,
		depthCam->colorWidth(),
		depthCam->colorHeight(),
		0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
	std::cout << " matte "; std::cout.flush();
	RenderToTexture virtualRgbTexture(
		GL_RGB8,
		depthCam->colorWidth(),
		depthCam->colorHeight(),
		0, GL_RGB, GL_UNSIGNED_BYTE);
	std::cout << " virt "; std::cout.flush();
	RenderToTexture virtualDepthTexture(
		GL_R16UI,
		depthCam->colorWidth(),
		depthCam->colorHeight(),
		0, GL_RED_INTEGER, GL_UNSIGNED_SHORT);
	std::cout << " vdepth "; std::cout.flush();
	cv::Mat glColor(cv::Size(depthCam->colorWidth(), depthCam->colorHeight()), CV_8UC3),
		glDepth(cv::Size(depthCam->colorWidth(), depthCam->colorHeight()), CV_16UC1),
		gtMask,
		matte(cv::Size(depthCam->colorWidth(), depthCam->colorHeight()), CV_8UC1);

	std::cout << " Textures loaded!" << std::endl;

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
	bool paused = false;
	const int frames_to_capture = 12;
	int captured_frames = 0;
	float fps = 30.f;
	float timeTaken = 0.f;
	bool trackingGood = false;
	std::string occlusionMethodName = occlusionMethods[occlusionMethodIdx]->getName();
	tweakBar.addVarRO("Method", &occlusionMethodName, "Method");
	tweakBar.addVarRO("Tracking Good", &trackingGood, "Tracking Good");
	tweakBar.addVarRO("Time taken (ms)", &timeTaken, "Time taken (ms)");
	tweakBar.addVarRO("FPS", &fps, "FPS");
	tweakBar.addVarRW("Paused", &paused, "Paused");
	tweakBar.addVarRW("Model Idx", &currMeshIdx, "Model Idx", 0, meshes.size()-1);
	tweakBar.addVarRW("Method Idx", &occlusionMethodIdx, "Method Idx", 0, occlusionMethods.size()-1);
	tweakBar.addVarRW("Visualise Stages", &visualise, "Visualise Stages");
	tweakBar.addVarRO("Display Mode", &modeText, "Display Mode");
	while (running) {
		glViewport(0, 0, 640 * 2, 480 * 2);
		glClearColor(0.1f, 0.1f, 0.1f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (!paused) {
			depthCam->getLatestFrames();
		}

		rgbTexture.update(depthCam->colorFramePtr());
		depthTexture.update(depthCam->depthFramePtr());

		markerTracker.processImage(depthCam->colorFramePtr());
		trackingGood = markerTracker.trackingGood();
		if (trackingGood) {
			arCamera.updatePose(markerTracker.getModelViewMat());
		}

		std::chrono::system_clock::duration d;

		if (mode == Mode::DEPTHS_AND_OUTPUT) {

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
		}

		glEnable(GL_DEPTH_TEST);
		arCamera.bindCameraBlock();
		virtualRgbTexture.setAsRenderTarget();
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_BLEND);
		meshes[currMeshIdx]->render();
		virtualDepthTexture.setAsRenderTarget();
		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		drawVirtualDepthShader.setUniform("worldToCam", markerTracker.getModelViewMat());
		throwOnGlError();
		meshes[currMeshIdx]->render(&drawVirtualDepthShader);
		virtualDepthTexture.unsetAsRenderTarget();

		//Render virtual content into app.
		if (mode != Mode::WITH_AND_WITHOUT) {
			virtualRgbTexture.texture().bindToImageUnit(0);
			virtualDepthTexture.texture().bindToImageUnit(1);
			glViewport(0, 0, 640, 480);
			glClear(GL_DEPTH_BUFFER_BIT);
			FullScreenQuad::getInstance().render(addIfDepthNonZeroShader);
		}

		virtualDepthTexture.texture().getData(glDepth.data, glColor.rows*glColor.cols * 2);
		cv::flip(glDepth, glDepth, 0);

		auto t = std::chrono::system_clock::now();
		occlusionMethods[occlusionMethodIdx]->visualise = visualise;
		occlusionMethods[occlusionMethodIdx]->calculateOcclusion(
			depthCam->colorFramePtr(), depthCam->depthFramePtr(),
			(unsigned short*)(glDepth.data), matte.data);
		d = std::chrono::system_clock::now() - t;

		matteTexture.update(matte.data);

		if (mode == Mode::DEPTHS_AND_OUTPUT || mode == Mode::WITH_AND_WITHOUT) {
			glViewport(640, 0, 640, 480);
		} else if (mode == Mode::OUTPUT_ONLY) {
			glViewport(0, 0, 640, 480);
		}

		rgbTexture.bindToImageUnit(0);
		matteTexture.bindToImageUnit(1);
		virtualRgbTexture.texture().bindToImageUnit(2);
		FullScreenQuad::getInstance().render(composeShader);

		if (mode == Mode::DEPTHS_AND_OUTPUT) {
			glViewport(0, 480, 640, 480);
			virtualDepthTexture.texture().bindToImageUnit(0);
			FullScreenQuad::getInstance().render(showVirtualDepthShader);
		}

		if (mode == Mode::WITH_AND_WITHOUT) {
			//Calc and render result with naive approach.
			naiveOcclusionMethod.calculateOcclusion(
				depthCam->colorFramePtr(), depthCam->depthFramePtr(),
				(unsigned short*)(glDepth.data), matte.data);
			matteTexture.update(matte.data);
			glViewport(0, 0, 640, 480);
			rgbTexture.bindToImageUnit(0);
			matteTexture.bindToImageUnit(1);
			virtualRgbTexture.texture().bindToImageUnit(2);
			FullScreenQuad::getInstance().render(composeShader);
		}

		matteTexture.update(matte.data);

		timeTaken = std::chrono::duration_cast<std::chrono::milliseconds>(d).count();
		fps = 1000.f / timeTaken;
		glViewport(0, 0, winWidth, winHeight);
		occlusionMethodName = occlusionMethods[occlusionMethodIdx]->getName();
		tweakBar.draw();
		win.swapBuffers();

		while (SDL_PollEvent(&event)) {
			if (GLWindow::eventIsQuit(event)) {
				running = false;
			}
			if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_SPACE) {
					++occlusionMethodIdx; occlusionMethodIdx %= occlusionMethods.size();
				}
				if (event.key.keysym.sym == SDLK_f) {
					win.fullscreen(!win.fullscreen());
				}

				if (event.key.keysym.sym == SDLK_m) {
					if (mode == Mode::DEPTHS_AND_OUTPUT) {
						mode = Mode::OUTPUT_ONLY;
						winWidth = 640; winHeight = 480;
						modeText = "Output";
					} else if (mode == Mode::OUTPUT_ONLY) {
						mode = Mode::WITH_AND_WITHOUT;
						winWidth = 640*2; winHeight = 480;
						modeText = "With & Without Occlusion";
					} else /* Mode::WITH_AND_WITHOUT */ {
						mode = Mode::DEPTHS_AND_OUTPUT;
						winWidth = 640*2; winHeight = 480*2;
						modeText = "Output, Real/Virtual Depths";
					}
					win.size(winWidth, winHeight);
				}
			}
			occlusionMethods[occlusionMethodIdx]->processEvent(event);
			tweakBar.processEvent(event);
		}
	}

	return 0;
}
