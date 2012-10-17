/**
 *  @file oglplus/text/pango_cairo/layout.hpp
 *  @brief Pango/Cairo-based text rendering - layout.
 *
 *  @author Matus Chochlik
 *
 *  Copyright 2010-2012 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef OGLPLUS_TEXT_PANGO_CAIRO_LAYOUT_HPP
#define OGLPLUS_TEXT_PANGO_CAIRO_LAYOUT_HPP

#include <oglplus/config.hpp>
#include <oglplus/vector.hpp>
#include <oglplus/text/common.hpp>
#include <oglplus/text/pango_cairo/fwd.hpp>
#include <oglplus/text/pango_cairo/handle.hpp>
#include <oglplus/text/pango_cairo/layout_storage.hpp>
#include <oglplus/texture.hpp>


namespace oglplus {
namespace text {

class PangoCairoLayout
{
private:
	friend class PangoCairoRenderer;

	PangoCairoRendering& _parent;

	PangoCairoFont _font;
	const GLsizei _capacity;
	int _curr_width;
	int _width;
	int _height;

	Vec4f _log_coords, _tex_coords;

	PangoCairoHandle< ::cairo_surface_t*> _surface;

	PangoCairoLayoutData _data;

	PangoCairoLayout(const PangoCairoLayout&);
public:
	PangoCairoLayout(
		PangoCairoRendering& parent,
		const PangoCairoFont& font,
		GLsizei capacity
	): _parent(parent)
	 , _font(font)
	 , _capacity(capacity)
	 , _curr_width(0)
	 , _width(font._essence->ApproxGlyphWidth()*_capacity*1.2f)
	 , _height(font._essence->Height())
	 , _surface(
		::cairo_image_surface_create(CAIRO_FORMAT_A8, _width, _height),
		::cairo_surface_destroy
	)
	{
		PangoCairoAllocateLayoutData(_parent, _data, _width, _height);
	}

	PangoCairoLayout(PangoCairoLayout&& tmp)
	 : _parent(tmp._parent)
	 , _font(std::move(tmp._font))
	 , _capacity(tmp._capacity)
	 , _curr_width(tmp._curr_width)
	 , _width(tmp._width)
	 , _height(tmp._height)
	 , _surface(std::move(tmp._surface))
	 , _data(std::move(tmp._data))
	{ }

	~PangoCairoLayout(void)
	{
		PangoCairoDeallocateLayoutData(_parent, _data);
	}

	TextureUnitSelector Use(void) const
	{
		return PangoCairoUseLayoutData(_parent, _data);
	}

	GLsizei Capacity(void) const
	{
		return _capacity;
	}

	GLfloat Width(void) const
	{
		return GLfloat(_curr_width) / GLfloat(_height);
	}

	void Set(const char* c_str, const std::size_t size)
	{
		// create a cairo renderer
		PangoCairoHandle< ::cairo_t*> cairo(
			::cairo_create(_surface),
			::cairo_destroy
		);
		// clear the surface
		::cairo_save(cairo);
		::cairo_set_source_rgba(cairo, 0, 0, 0, 0);
		::cairo_set_operator(cairo, CAIRO_OPERATOR_SOURCE);
		::cairo_paint(cairo);
		::cairo_restore(cairo);

		// if the new text string is empty, quit
		if(size == 0) return;

		// create a layout
		PangoCairoHandle< ::PangoLayout*, ::gpointer> layout(
			::pango_cairo_create_layout(cairo),
			::g_object_unref
		);
		// set the text
		::pango_layout_set_text(layout, c_str, size);
		::pango_layout_set_font_description(
			layout,
			_font._essence->_font_desc
		);
		// check the required layout dimensions
		int req_width = 0;
		int req_height = 0;
		{
			::PangoRectangle ink_rect, log_rect;
			::pango_layout_get_extents(
				layout,
				&ink_rect,
				&log_rect
			);
			req_width = log_rect.width/PANGO_SCALE;
			req_height = log_rect.height/PANGO_SCALE;
			_curr_width = req_width;
		}

		// resize if necessary
		if((_width < req_width) || (_height < req_height))
		{
			_width = req_width;
			_height = req_height;
			_surface.replace(
				::cairo_image_surface_create(
					CAIRO_FORMAT_A8,
					_width,
					_height
				)
			);
			cairo.replace(::cairo_create(_surface));
			layout.replace(::pango_cairo_create_layout(cairo));
			PangoCairoDeallocateLayoutData(_parent, _data);
			PangoCairoAllocateLayoutData(
				_parent,
				_data,
				_width,
				_height
			);
		}
		int baseline = pango_layout_get_baseline(layout)/PANGO_SCALE;
		_log_coords = Vec4f(
			0.0f, // left bearing
			GLfloat(_curr_width)/GLfloat(_height), // right bearing
			GLfloat(baseline)/GLfloat(_height), // ascent
			GLfloat(_height-baseline)/GLfloat(_height) // descent
		);
		_tex_coords = Vec4f(
			0.0f, // origin x
			0.0f, // origin y
			_curr_width, // width
			_height // height
		);

		// render the text
		::cairo_new_path(cairo);
		::cairo_move_to(cairo, 0, 0);
		::cairo_set_line_width(cairo, 0.5);
		::pango_cairo_update_layout(cairo, layout);
		::pango_cairo_layout_path(cairo, layout);
		::cairo_fill(cairo);
		::cairo_surface_flush(_surface);

		// update the data
		PangoCairoInitializeLayoutData(
			_parent,
			_data,
			::cairo_image_surface_get_width(_surface),
			::cairo_image_surface_get_height(_surface),
			::cairo_image_surface_get_data(_surface)
		);
	}

	void Set(const StrLit& string_literal)
	{
		Set(string_literal.c_str(), string_literal.size());
	}

	void Set(const String& string)
	{
		Set(string.c_str(), string.size());
	}

	void Set(const CodePoint* code_points, const GLsizei length)
	{
		std::vector<char> str;
		CodePointsToUTF8(code_points, length, str);
		Set(str.data(), str.size());
	}
};

} // namespace text
} // namespace oglplus

#endif // include guard
