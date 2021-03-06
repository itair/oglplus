/**
 *  @file eglplus/error_code.hpp
 *  @brief EGL error code enumeration
 *
 *  @author Matus Chochlik
 *
 *  Copyright 2010-2013 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once
#ifndef EGLPLUS_ERROR_CODE_1305302126_HPP
#define EGLPLUS_ERROR_CODE_1305302126_HPP

#include <eglplus/enumerations.hpp>

namespace eglplus {

/// EGL error code enumeration
/**
 *  @ingroup eglplus_enumerations
 */
EGLPLUS_ENUM_CLASS_BEGIN(ErrorCode, EGLenum)
#include <eglplus/enums/error_code.ipp>
EGLPLUS_ENUM_CLASS_END(ErrorCode)

#if !EGLPLUS_NO_ENUM_VALUE_NAMES
#include <eglplus/enums/error_code_names.ipp>
#endif

#if !EGLPLUS_ENUM_VALUE_RANGES
#include <eglplus/enums/error_code_range.ipp>
#endif

} // namespace eglplus

#endif // include guard
