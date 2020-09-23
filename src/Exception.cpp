#include "mrocclusion/Exception.hpp"
#include <string>
#include <GL/glew.h>
#include "mrocclusion/ShaderProgram.hpp"

FileException::FileException(const std::string &what)
:std::runtime_error(what.c_str())
{}

FileException::~FileException() throw()
{}

GraphicsException::GraphicsException(const std::string &what)
:std::runtime_error(what.c_str())
{}

GraphicsException::~GraphicsException() throw()
{}

CameraException::CameraException(const std::string &what)
:std::runtime_error(what.c_str())
{}

CameraException::~CameraException() throw()
{}

DepthCamException::DepthCamException(const std::string &what)
:CameraException(what.c_str())
{}

DepthCamException::~DepthCamException() throw()
{}

ShaderException::ShaderException(const std::string &what)
:GraphicsException(what.c_str())
{}

ShaderException::~ShaderException() throw()
{}

void throwOnGlError_(const std::string &info)
{
	GLenum err = glGetError();
	std::string errname = glErrToString(err);
	if (err != GL_NO_ERROR) {
		throw GraphicsException(info + " " + errname);
	}
}

