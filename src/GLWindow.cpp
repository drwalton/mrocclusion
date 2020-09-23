#include "mrocclusion/GLWindow.hpp"
#include "mrocclusion/Directories.hpp"
#include "mrocclusion/Texture.hpp"
#include "mrocclusion/ShaderProgram.hpp"
#include "mrocclusion/FullScreenQuad.hpp"
#include <iostream>
#include <GL/glew.h>
#ifdef WIN32
#include <SDL_ttf.h>
#elif defined __linux__
#include <SDL/SDL_ttf.h>
#else
#include <SDL2/SDL_ttf.h>
#endif

size_t GLWindow::count = 0;

struct GLWindow::Impl {
	TTF_Font *textFont;
	SDL_Surface *textSurface;
	SDL_Renderer *renderer;
	std::unique_ptr<Texture> texture;
	std::unique_ptr<ShaderProgram> textShader;
	SDL_Color textColor;
	SDL_Window *window;
	SDL_GLContext context;
};

GLWindow::GLWindow(
	const std::string &name, size_t width, size_t height,
	size_t x, size_t y, bool resizeable, bool fullscreen)
	:pimpl(new Impl()),
	width(width), height(height)
{
	if (resizeable && fullscreen) {
		throw GraphicsException("Can't have a resizeable fullscreen window!");
	}

	bool multisample = false;
	if (count == 0) {
		if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
			throw GraphicsException(std::string("Unable to init SDL!"));
		}
	}
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
#if __APPLE__
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
#endif
	if (multisample) {
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);

	Uint32 winFlags = SDL_WINDOW_OPENGL;
	if (width == 0 && height == 0) {
		winFlags |= SDL_WINDOW_HIDDEN;
	} else {
		winFlags |= SDL_WINDOW_SHOWN;
	}
	
	if(resizeable) {
		winFlags |= SDL_WINDOW_RESIZABLE;
	} else if (fullscreen) {
		winFlags |= SDL_WINDOW_FULLSCREEN;
	}

	pimpl->window = SDL_CreateWindow(name.c_str(), int(x), int(y),
		int(width), int(height), winFlags);
	pimpl->context = SDL_GL_CreateContext(pimpl->window);
	pimpl->renderer = SDL_CreateRenderer(pimpl->window, -1, 0);
	makeCurrent();

	glewExperimental = GL_TRUE;
	GLenum r = glewInit();
	if (r != GLEW_OK) {
		throw GraphicsException("Couldn't initialise GLEW. Error message: " + std::string((const char*)glewGetErrorString(r)));
	}
	glGetError();

	const GLubyte* buf;
	buf = glGetString(GL_VERSION);
	std::cout << "GL Version " << buf << std::endl;


	if (!glewIsSupported("GL_VERSION_4_1")) {
		throw GraphicsException("OpenGL 4.1 required, but not supported.");
	}

	++count;

	TTF_Init();
	std::string fontFilename(MROCCLUSION_FONT_DIR + "default.ttf");
	pimpl->textFont = TTF_OpenFont(fontFilename.c_str(), 14);
	if(pimpl->textFont == nullptr) {
		const char *err = TTF_GetError();
		std::stringstream errs;
		errs << "Error loading font: " << fontFilename << "\nError message: " << err;
		throw std::runtime_error(errs.str());
	}

	pimpl->textColor = { 0, 0, 255 };

	pimpl->textSurface = nullptr;
	updateText("DEFAULT TEXT");

	pimpl->textShader.reset(new ShaderProgram(std::vector<std::string>{
		MROCCLUSION_SHADER_DIR + "FullScreenTex.vert",
		MROCCLUSION_SHADER_DIR + "FullScreenTexFlip.frag"
	}));
	glEnable(GL_MULTISAMPLE);
	throwOnGlError();
}

GLWindow::~GLWindow() throw()
{
	SDL_GL_DeleteContext(pimpl->context);
	SDL_DestroyWindow(pimpl->window);
	--count;
	if (count == 0) {
		SDL_Quit();
	}

	TTF_CloseFont(pimpl->textFont);
	SDL_FreeSurface(pimpl->textSurface);
	SDL_DestroyRenderer(pimpl->renderer);
}

void GLWindow::makeCurrent()
{
	SDL_GL_MakeCurrent(pimpl->window, pimpl->context);
}

void GLWindow::swapBuffers()
{
	SDL_GL_SwapWindow(pimpl->window);
}

bool GLWindow::eventIsQuit(const SDL_Event &e)
{
	if (e.type == SDL_QUIT) {
		return true;
	}
	if (e.type == SDL_WINDOWEVENT) {
		if (e.window.event == SDL_WINDOWEVENT_CLOSE) {
			return true;
		}
	}
	return false;
}

void GLWindow::updateText(const std::string &newText)
{
	if (pimpl->textSurface != nullptr) {
		SDL_FreeSurface(pimpl->textSurface);
	}
	pimpl->textSurface = TTF_RenderUTF8_Blended(pimpl->textFont, newText.c_str(), pimpl->textColor);
	pimpl->texture.reset(new Texture(GL_TEXTURE_2D, GL_RGBA,
		pimpl->textSurface->w, pimpl->textSurface->h,
		0, GL_RGBA, GL_UNSIGNED_BYTE, pimpl->textSurface->pixels));
}

void GLWindow::drawText(int x, int y)
{
	//SDL_Rect renderRect = { x, y, pimpl->textSurface->w, pimpl->textSurface->h };

	glViewport(GLsizei(x), GLsizei(height - y),
		GLsizei(pimpl->textSurface->w), GLsizei(pimpl->textSurface->h));
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	pimpl->texture->bindToImageUnit(1);
	pimpl->textShader->setUniform("tex", 1);
	FullScreenQuad::getInstance().render(*pimpl->textShader);
	pimpl->texture->unbind();

	glViewport(0, 0, GLsizei(width), GLsizei(height));
}

void GLWindow::setTitle(const std::string & title)
{
	SDL_SetWindowTitle(pimpl->window, title.c_str());
}

void GLWindow::size(int * w, int * h) const
{
	SDL_GetWindowSize(pimpl->window, w, h);
}

void GLWindow::size(int w, int h)
{
	SDL_SetWindowSize(pimpl->window, w, h);
}

Uint32 GLWindow::id()
{
	return SDL_GetWindowID(pimpl->window);
}


void GLWindow::fullscreen(bool enabled)
{
	SDL_SetWindowFullscreen(pimpl->window, enabled ? SDL_TRUE : SDL_FALSE);
}

bool GLWindow::fullscreen() const
{
	return SDL_GetWindowFlags(pimpl->window) & SDL_WINDOW_FULLSCREEN;
}

