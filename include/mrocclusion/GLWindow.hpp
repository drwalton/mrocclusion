#ifndef MROCCLUSION_GLWINDOW_HPP_INCLUDED
#define MROCCLUSION_GLWINDOW_HPP_INCLUDED

#include <GL/glew.h>
#include <string>
#include <SDL.h>
#include "Exception.hpp"
#include <memory>

//!\brief Class encapsulating generating a window and associated GL context.
class GLWindow final
{
public:
	//!\brief Constructor
	//!\param name Title of window, displayed at top.
	//!\param x X co-ordinate of upper left corner of window, upon creation.
	//!\param y Y co-ordinate of upper left corner of window, upon creation.
	//!\param resizeable Determines if window size can be modified by dragging
	//!       corners.
	GLWindow(
		const std::string &name, size_t width, size_t height,
		size_t x = 0, size_t y = 50, bool resizeable = false, bool fullscreen = false);
	~GLWindow() throw();

	void makeCurrent();
	void swapBuffers();

	static bool eventIsQuit(const SDL_Event &e);

	void updateText(const std::string &newText);

	//!\brief Draw the current text to the window, at the specified location.
	//!\note This will change the following OpenGL state:
	//! * GL_DEPTH_TEST will be disabled.
	//! * GL_CULL_FACE will be disabled.
	//! * GL_DEPTH_MASK will be set to GL_FALSE.
	//! * GL_BLEND will be enabled (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
	//! * The viewport will be set to the full window.
	//! These changes are necessary to draw the text. Please change state after calling this function if required.
	void drawText(int x, int y);

	void setTitle(const std::string &title);

	void size(int *w, int *h) const;
	void size(int w, int h);

	Uint32 id();

	void fullscreen(bool enabled);
	bool fullscreen() const;

private:
	GLWindow(const GLWindow &);
	GLWindow &operator=(const GLWindow&);
	
	static size_t count;
	struct Impl;
	std::unique_ptr<Impl> pimpl;
	size_t width, height;
};

#endif
