#pragma once

#include "onair/platform.h"

#if ONAIR_IOS
	#include <OpenGLES/ES2/gl.h>
	#include <OpenGLES/ES2/glext.h>
	#define OPENGL_ES 1
#elif defined(__APPLE__)
	#include <OpenGL/OpenGL.h>
	#include <OpenGL/gl3.h>
#elif defined(__ANDROID__)
	#include <GLES2/gl2.h>
	#include <GLES2/gl2ext.h>
	#define OPENGL_ES 1
#elif defined(__linux__)
	#if !defined(GL_GLEXT_PROTOTYPES)
		#define GL_GLEXT_PROTOTYPES
	#endif
	#include <GL/gl.h>
#endif