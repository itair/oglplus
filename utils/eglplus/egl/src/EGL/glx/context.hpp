/**
 *  .file EGL/context.hpp
 *  .brief EGL/GLX rendering context declarations
 *
 *  @author Matus Chochlik
 *
 *  Copyright 2012-2013 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef OGLPLUS_EGL_GLX_CONTEXT_1312301524_HPP
#define OGLPLUS_EGL_GLX_CONTEXT_1312301524_HPP

#include <X11/Xlib.h>
#include <GL/glx.h>

struct eglplus_egl_glx_ContextImpl
{
	::GLXContext _glx_context;

	eglplus_egl_glx_ContextImpl( ::GLXContext);
	~eglplus_egl_glx_ContextImpl(void);

	void _cleanup( ::Display*);
};

#endif // include guard
