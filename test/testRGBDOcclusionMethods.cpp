#include "mrocclusion/DepthCam.hpp"
#include "mrocclusion/Directories.hpp"

#include "mrocclusion/GuidedRgbdOcclusionMethod.hpp"
#include "mrocclusion/SelectiveGuidedOcclusionMethod.hpp"
#include "mrocclusion/CostVolumeOcclusionMethod.hpp"
#include "mrocclusion/BilateralRgbdOcclusionMethod.hpp"

#include <mrocclusion/Directories.hpp>
#include <mrocclusion/FullScreenQuad.hpp>
#include <mrocclusion/GLWindow.hpp>
#include <mrocclusion/ShaderProgram.hpp>
#include <mrocclusion/Texture.hpp>

#include <iostream>
#include <thread>

size_t winWidth = 640 * 2;
size_t winHeight = 480 * 2;
size_t currMethodIndex = 0;
size_t squareRad = 40;

int main(int argc, char *argv[]) {
	GLWindow win("Occlusion Test", winWidth, winHeight);
	bool running = true;

	std::unique_ptr<DepthCam> depthCam = openDefaultDepthCam();
	depthCam->autoWhitebalanceEnabled(true);
	depthCam->getLatestFrames();
	depthCam->autoExposureEnabled(true);

	Texture rgbTexture(
		GL_TEXTURE_2D, GL_RGB8,
		depthCam->colorWidth(),
		depthCam->colorHeight(),
		0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	Texture depthTexture(
		GL_TEXTURE_2D, GL_R16UI,
		depthCam->depthWidth(),
		depthCam->depthHeight(),
		0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, nullptr);
	Texture matteTexture(
		GL_TEXTURE_2D, GL_R8,
		depthCam->depthWidth(),
		depthCam->depthHeight(),
		0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

	std::vector<std::unique_ptr<RgbdOcclusionMethod>> occlusionMethods;
	size_t currOcclusionMethodIdx = 0;
	occlusionMethods.emplace_back(new SelectiveGuidedOcclusionMethod(
		depthCam->colorWidth(), depthCam->colorHeight(), depthCam->depthWidth(), depthCam->depthHeight()
	));
	occlusionMethods.emplace_back(new GuidedRgbdOcclusionMethod(
		depthCam->colorWidth(), depthCam->colorHeight(), depthCam->depthWidth(), depthCam->depthHeight()
	));
	occlusionMethods.emplace_back(new CostVolumeOcclusionMethod(
		depthCam->colorWidth(), depthCam->colorHeight(), depthCam->depthWidth(), depthCam->depthHeight()
	));
	occlusionMethods.emplace_back(new BilateralRgbdOcclusionMethod(
		depthCam->colorWidth(), depthCam->colorHeight(), depthCam->depthWidth(), depthCam->depthHeight()
	));

	cv::Mat virtualDepth(cv::Size(depthCam->depthWidth(), depthCam->depthHeight()), CV_16UC1);
	cv::Mat matte(cv::Size(depthCam->colorWidth(), depthCam->colorHeight()), CV_8UC1);
	virtualDepth.setTo(0);
	size_t objRad = 50;
	virtualDepth(cv::Rect(640 / 2 - objRad, 480 / 2 - objRad, objRad * 2, objRad * 2)).setTo(1000);

	ShaderProgram showImageShader(std::vector<std::string> {
		MROCCLUSION_SHADER_DIR + "FullScreenTex.vert",
		MROCCLUSION_SHADER_DIR + "FullScreenTexFlip.frag"
	});
	ShaderProgram showDepthShader(std::vector<std::string> {
		MROCCLUSION_SHADER_DIR + "FullScreenTex.vert",
		MROCCLUSION_SHADER_DIR + "FullScreenDepthTexFlip.frag"
	});
	showImageShader.setUniform("tex", 0);
	showDepthShader.setUniform("tex", 0);
	glClearColor(0.1f, 0.1f, 0.1f, 1.f);

	bool newFramesAvailable;
	std::thread getFramesThread([&newFramesAvailable, &depthCam, &running]() {
		while (running) {
			if (!newFramesAvailable) {
				depthCam->getLatestFrames();
				newFramesAvailable = true;
			}
		}
	});

	SDL_Event event;
	while (running) {
		glViewport(0, 0, 640*2, 480*2);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (newFramesAvailable) {
			rgbTexture.update(depthCam->colorFramePtr());
			depthTexture.update(depthCam->depthFramePtr());
			occlusionMethods[currOcclusionMethodIdx]->calculateOcclusion(
				depthCam->colorFramePtr(), depthCam->depthFramePtr(), 
				reinterpret_cast<unsigned short*>(virtualDepth.data), matte.data
			);
			matteTexture.update(matte.data);
			newFramesAvailable = false;
		}

		glViewport(0, 480, 640, 480);
		rgbTexture.bindToImageUnit(0);
		FullScreenQuad::getInstance().render(showImageShader);

		glViewport(640, 480, 640, 480);
		depthTexture.bindToImageUnit(0);
		FullScreenQuad::getInstance().render(showDepthShader);

		glViewport(0, 0, 640, 480);
		matteTexture.bindToImageUnit(0);
		FullScreenQuad::getInstance().render(showImageShader);

		std::stringstream text;
		text << "Auto WB: " << (depthCam->autoWhitebalanceEnabled() ? "on" : "off")
			<< " Exposure: " << depthCam->exposure();
		win.updateText(text.str());
		win.drawText(30, 30);

		while (SDL_PollEvent(&event)) {
			if (GLWindow::eventIsQuit(event)) {
				running = false;
			}
			if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_SPACE) {
					++currOcclusionMethodIdx;
					currOcclusionMethodIdx %= occlusionMethods.size();
				}
			}

			occlusionMethods[currOcclusionMethodIdx]->processEvent(event);
		}
		win.swapBuffers();
	}

	getFramesThread.join();

	return 0;
}
