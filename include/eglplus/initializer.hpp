/**
 *  @file eglplus/initializer.hpp
 *  @brief Declares and implements the EGL API initializer
 *
 *  @author Matus Chochlik
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once
#ifndef EGLPLUS_INITIALIZER_1305291005_HPP
#define EGLPLUS_INITIALIZER_1305291005_HPP

#include <eglplus/display.hpp>
#include <eglplus/eglfunc.hpp>
#include <eglplus/error.hpp>
#include <eglplus/string_query.hpp>

#include <eglplus/auxiliary/sep_str_range.hpp>

namespace eglplus {

/// Initializes and terminates the EGL library
class EGLInitializer
 : public FriendOf<Display>
{
private:
	Display _display;
	EGLint _major, _minor;
	bool _initialized;

	EGLInitializer(const EGLInitializer&);

	bool _initialize(void)
	{
		bool result = EGLPLUS_EGLFUNC(Initialize)(
			FriendOf<Display>::GetHandle(_display),
			&_major,
			&_minor
		) == EGL_TRUE;
		EGLPLUS_CHECK(EGLPLUS_ERROR_INFO(Initialize));
		return result;
	}
public:
	/// Initializes EGL on the default display
	/**
	 *  @eglsymbols
	 *  @eglfunref{GetDisplay}
	 *  @egldefref{DEFAULT_DISPLAY}
	 *  @eglfunref{Initialize}
	 */
	EGLInitializer(void)
	 : _display()
	 , _initialized(_initialize())
	{ }

	/// Initializes EGL on the specified display
	/**
	 *  @eglsymbols
	 *  @eglfunref{Initialize}
	 */
	EGLInitializer(const Display& display)
	 : _display(display)
	 , _initialized(_initialize())
	{ }

	/// Initializer is move constructible
	EGLInitializer(EGLInitializer&& tmp)
	 : _display(tmp._display)
	 , _major(tmp._major)
	 , _minor(tmp._minor)
	 , _initialized(tmp._initialized)
	{
		tmp._initialized = false;
	}

	/// Terminates EGL on the associated display
	/**
	 *  @eglsymbols
	 *  @eglfunref{Terminate}
	 */
	~EGLInitializer(void)
	{
		if(_initialized)
		{
			EGLPLUS_EGLFUNC(Terminate)(
				FriendOf<Display>::GetHandle(_display)
			);
			EGLPLUS_VERIFY(EGLPLUS_ERROR_INFO(Terminate));
		}
	}

	/// Returns a reference to the associated display
	const Display& AssociatedDisplay(void) const
	{
		return _display;
	}

	/// Returns a reference to the associated display
	const Display& Disp(void) const
	{
		return _display;
	}

	/// Returns the major version number
	/**
	 *  @eglsymbols
	 *  @eglfunref{Initialize}
	 */
	EGLint VersionMajor(void) const
	{
		return _major;
	}

	/// Returns the major version number
	/**
	 *  @eglsymbols
	 *  @eglfunref{Initialize}
	 */
	EGLint VersionMinor(void) const
	{
		return _minor;
	}

	/// EGL string queries
	/**
	 *  @eglsymbols
	 *  @eglfunref{QueryString}
	 */
	const char* QueryString(StringQuery query) const
	{
		assert(_initialized);
		const char* result = EGLPLUS_EGLFUNC(QueryString)(
				FriendOf<Display>::GetHandle(_display),
				EGLint(query)
		);
		EGLPLUS_VERIFY(EGLPLUS_ERROR_INFO(QueryString));
		return result;
	}

	/// Returns the vendor name
	/**
	 *  @eglsymbols
	 *  @eglfunref{QueryString}
	 *  @egldefref{EGL_VENDOR}
	 */
	const char* Vendor(void) const
	{
		return QueryString(StringQuery::Vendor);
	}

	/// Returns the version string
	/**
	 *  @eglsymbols
	 *  @eglfunref{QueryString}
	 *  @egldefref{EGL_VERSION}
	 */
	const char* Version(void) const
	{
		return QueryString(StringQuery::Version);
	}

#if EGLPLUS_DOCUMENTATION_ONLY
	/// Returns a range of supported API name strings
	Range<String> ClientAPIs(void) const;
#else
	aux::SepStrRange ClientAPIs(void) const
	{
		return aux::SepStrRange(QueryString(StringQuery::ClientAPIs));
	}
#endif

#if EGLPLUS_DOCUMENTATION_ONLY
	/// Returns a range of extension strings
	Range<String> Extensions(void) const;
#else
	aux::SepStrRange Extensions(void) const
	{
		return aux::SepStrRange(QueryString(StringQuery::Extensions));
	}
#endif

	/// Returns true if the EGL implementation supports client extensions
	static bool HasClientExtensions(void)
	{
		const char* ext_str = EGLPLUS_EGLFUNC(QueryString)(
				EGL_NO_DISPLAY,
				EGLint(EGL_EXTENSIONS)
		);
		EGLint err_c = EGLPLUS_EGLFUNC(GetError)();
		return (err_c == EGL_SUCCESS) && (ext_str != nullptr);
	}

#if EGLPLUS_DOCUMENTATION_ONLY
	/// Returns a range of client extension strings
	/**
	 *  @pre HasClientExtensions
	 */
	static Range<String> ClientExtensions(void);
#else
	static aux::SepStrRange ClientExtensions(void)
	{
		const char* ext_str = EGLPLUS_EGLFUNC(QueryString)(
				EGL_NO_DISPLAY,
				EGLint(EGL_EXTENSIONS)
		);
		EGLPLUS_VERIFY(EGLPLUS_ERROR_INFO(QueryString));
		return aux::SepStrRange(ext_str);
	}
#endif
};

/// Alternate name for EGLInitializer
typedef EGLInitializer LibEGL;

} // namespace eglplus

#endif // include guard
