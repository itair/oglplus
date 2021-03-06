/**
 *  .file oglplus/auxiliary/prog_pl_stages.hpp
 *  .brief Helper class used by ProgramPipeline::UseStages
 *
 *  @author Matus Chochlik
 *
 *  Copyright 2010-2013 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once
#ifndef OGLPLUS_AUX_PROG_PL_STAGES_1107121519_HPP
#define OGLPLUS_AUX_PROG_PL_STAGES_1107121519_HPP

#include <oglplus/config.hpp>
#include <oglplus/glfunc.hpp>
#include <oglplus/error.hpp>

namespace oglplus {

#if GL_VERSION_4_1 || GL_ARB_separate_shader_objects

class ProgramPipelineOps;

namespace aux {

class ProgPLUseStages
{
private:
	GLuint _pipeline, _program;
	GLbitfield _bits;

	GLbitfield _forward(void)
	OGLPLUS_NOEXCEPT(true)
	{
		GLbitfield res = _bits;
		_bits = 0;
		return res;
	}

	friend class oglplus::ProgramPipelineOps;

#if !OGLPLUS_NO_DELETED_FUNCTIONS
	ProgPLUseStages(void) = delete;
	ProgPLUseStages(const ProgPLUseStages&) = delete;
#else
	ProgPLUseStages(void);
	ProgPLUseStages(const ProgPLUseStages&);
#endif

	ProgPLUseStages(GLuint pipeline, GLuint program, GLbitfield bits)
	OGLPLUS_NOEXCEPT(true)
	 : _pipeline(pipeline)
	 , _program(program)
	 , _bits(bits)
	{ }

	inline ProgPLUseStages _make(GLbitfield bit)
	OGLPLUS_NOEXCEPT(true)
	{
		return ProgPLUseStages(_pipeline, _program, _forward() | bit);
	}
public:
	inline ProgPLUseStages Vertex(void)
	OGLPLUS_NOEXCEPT(true)
	{
		return _make(GL_VERTEX_SHADER_BIT);
	}

	inline ProgPLUseStages TessControl(void)
	OGLPLUS_NOEXCEPT(true)
	{
		return _make(GL_TESS_CONTROL_SHADER_BIT);
	}

	inline ProgPLUseStages TessEvaluation(void)
	OGLPLUS_NOEXCEPT(true)
	{
		return _make(GL_TESS_EVALUATION_SHADER_BIT);
	}

	inline ProgPLUseStages Geometry(void)
	OGLPLUS_NOEXCEPT(true)
	{
		return _make(GL_GEOMETRY_SHADER_BIT);
	}

	inline ProgPLUseStages Fragment(void)
	OGLPLUS_NOEXCEPT(true)
	{
		return _make(GL_FRAGMENT_SHADER_BIT);
	}

	inline ProgPLUseStages All(void)
	OGLPLUS_NOEXCEPT(true)
	{
		return _make(GL_ALL_SHADER_BITS);
	}

	inline ProgPLUseStages(ProgPLUseStages&& temp)
	OGLPLUS_NOEXCEPT(true)
	 : _pipeline(temp._pipeline)
	 , _program(temp._program)
	 , _bits(temp._forward())
	{ }

	void DoIt(void)
	{
		if(_bits)
		{
			assert(_pipeline);
			assert(_program);
			OGLPLUS_GLFUNC(UseProgramStages)(
				_pipeline,
				_bits,
				_program
			);
			OGLPLUS_VERIFY(OGLPLUS_ERROR_INFO(UseProgramStages));
			_bits = 0;
		}
	}

	inline ~ProgPLUseStages(void)
	{
		try{ DoIt(); }
		catch(...){ }
	}
};

} // namespace aux

#endif // program pipeline

} // namespace oglplus

#endif // include guard
