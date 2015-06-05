/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "math_types.h"

namespace crown
{

/// Holds RGBA color as four floats.
///
/// @ingroup Math
typedef Vector4 Color4;

/// Functions to mamipulate Color4
///
/// @ingroup Math
namespace color4
{
	Color4 from_rgb(int r, int g, int b);
	Color4 from_rgba(int r, int g, int b, int a);
	Color4 from_rgba(uint32_t rgba);

	/// Returns the color as a packed 32-bit integer. (RGBA order, alpha assumed = 255)
	uint32_t to_rgb(const Color4& c);

	/// Returns the color as a packed 32-bit integer. (ABGR order, alpha assumed = 255)
	uint32_t to_bgr(const Color4& c);

	/// Returns the color as a packed 32-bit integer. (RGBA order)
	uint32_t to_rgba(const Color4& c);

	/// Returns the color as a packed 32-bit integer. (ABGR order)
	uint32_t to_abgr(const Color4& c);
} // namespace color4

namespace color4
{
	// SVG 1.0 color names
	const Color4 ALICEBLUE            = from_rgba(0XF0F8FFFF);
	const Color4 ANTIQUEWHITE         = from_rgba(0XFAEBD7FF);
	const Color4 AQUA                 = from_rgba(0X00FFFFFF);
	const Color4 AQUAMARINE           = from_rgba(0X7FFFD4FF);
	const Color4 AZURE                = from_rgba(0XF0FFFFFF);
	const Color4 BEIGE                = from_rgba(0XF5F5DCFF);
	const Color4 BISQUE               = from_rgba(0XFFE4C4FF);
	const Color4 BLACK                = from_rgba(0X000000FF);
	const Color4 BLANCHEDALMOND       = from_rgba(0XFFEBCDFF);
	const Color4 BLUE                 = from_rgba(0X0000FFFF);
	const Color4 BLUEVIOLET           = from_rgba(0X8A2BE2FF);
	const Color4 BROWN                = from_rgba(0XA52A2AFF);
	const Color4 BURLYWOOD            = from_rgba(0XDEB887FF);
	const Color4 CADETBLUE            = from_rgba(0X5F9EA0FF);
	const Color4 CHARTREUSE           = from_rgba(0X7FFF00FF);
	const Color4 CHOCOLATE            = from_rgba(0XD2691EFF);
	const Color4 CORAL                = from_rgba(0XFF7F50FF);
	const Color4 CORNFLOWERBLUE       = from_rgba(0X6495EDFF);
	const Color4 CORNSILK             = from_rgba(0XFFF8DCFF);
	const Color4 CRIMSON              = from_rgba(0XDC143CFF);
	const Color4 CYAN                 = from_rgba(0X00FFFFFF);
	const Color4 DARKBLUE             = from_rgba(0X00008BFF);
	const Color4 DARKCYAN             = from_rgba(0X008B8BFF);
	const Color4 DARKGOLDENROD        = from_rgba(0XB8860BFF);
	const Color4 DARKGRAY             = from_rgba(0XA9A9A9FF);
	const Color4 DARKGREEN            = from_rgba(0X006400FF);
	const Color4 DARKGREY             = from_rgba(0XA9A9A9FF);
	const Color4 DARKKHAKI            = from_rgba(0XBDB76BFF);
	const Color4 DARKMAGENTA          = from_rgba(0X8B008BFF);
	const Color4 DARKOLIVEGREEN       = from_rgba(0X556B2FFF);
	const Color4 DARKORANGE           = from_rgba(0XFF8C00FF);
	const Color4 DARKORCHID           = from_rgba(0X9932CCFF);
	const Color4 DARKRED              = from_rgba(0X8B0000FF);
	const Color4 DARKSALMON           = from_rgba(0XE9967AFF);
	const Color4 DARKSEAGREEN         = from_rgba(0X8FBC8FFF);
	const Color4 DARKSLATEBLUE        = from_rgba(0X483D8BFF);
	const Color4 DARKSLATEGRAY        = from_rgba(0X2F4F4FFF);
	const Color4 DARKSLATEGREY        = from_rgba(0X2F4F4FFF);
	const Color4 DARKTURQUOISE        = from_rgba(0X00CED1FF);
	const Color4 DARKVIOLET           = from_rgba(0X9400D3FF);
	const Color4 DEEPPINK             = from_rgba(0XFF1493FF);
	const Color4 DEEPSKYBLUE          = from_rgba(0X00BFFFFF);
	const Color4 DIMGRAY              = from_rgba(0X696969FF);
	const Color4 DIMGREY              = from_rgba(0X696969FF);
	const Color4 DODGERBLUE           = from_rgba(0X1E90FFFF);
	const Color4 FIREBRICK            = from_rgba(0XB22222FF);
	const Color4 FLORALWHITE          = from_rgba(0XFFFAF0FF);
	const Color4 FORESTGREEN          = from_rgba(0X228B22FF);
	const Color4 FUCHSIA              = from_rgba(0XFF00FFFF);
	const Color4 GAINSBORO            = from_rgba(0XDCDCDCFF);
	const Color4 GHOSTWHITE           = from_rgba(0XF8F8FFFF);
	const Color4 GOLD                 = from_rgba(0XFFD700FF);
	const Color4 GOLDENROD            = from_rgba(0XDAA520FF);
	const Color4 GRAY                 = from_rgba(0X808080FF);
	const Color4 GREEN                = from_rgba(0X008000FF);
	const Color4 GREENYELLOW          = from_rgba(0XADFF2FFF);
	const Color4 GREY                 = from_rgba(0X808080FF);
	const Color4 HONEYDEW             = from_rgba(0XF0FFF0FF);
	const Color4 HOTPINK              = from_rgba(0XFF69B4FF);
	const Color4 INDIANRED            = from_rgba(0XCD5C5CFF);
	const Color4 INDIGO               = from_rgba(0X4B0082FF);
	const Color4 IVORY                = from_rgba(0XFFFFF0FF);
	const Color4 KHAKI                = from_rgba(0XF0E68CFF);
	const Color4 LAVENDER             = from_rgba(0XE6E6FAFF);
	const Color4 LAVENDERBLUSH        = from_rgba(0XFFF0F5FF);
	const Color4 LAWNGREEN            = from_rgba(0X7CFC00FF);
	const Color4 LEMONCHIFFON         = from_rgba(0XFFFACDFF);
	const Color4 LIGHTBLUE            = from_rgba(0XADD8E6FF);
	const Color4 LIGHTCORAL           = from_rgba(0XF08080FF);
	const Color4 LIGHTCYAN            = from_rgba(0XE0FFFFFF);
	const Color4 LIGHTGOLDENRODYELLOW = from_rgba(0XFAFAD2FF);
	const Color4 LIGHTGRAY            = from_rgba(0XD3D3D3FF);
	const Color4 LIGHTGREEN           = from_rgba(0X90EE90FF);
	const Color4 LIGHTGREY            = from_rgba(0XD3D3D3FF);
	const Color4 LIGHTPINK            = from_rgba(0XFFB6C1FF);
	const Color4 LIGHTSALMON          = from_rgba(0XFFA07AFF);
	const Color4 LIGHTSEAGREEN        = from_rgba(0X20B2AAFF);
	const Color4 LIGHTSKYBLUE         = from_rgba(0X87CEFAFF);
	const Color4 LIGHTSLATEGRAY       = from_rgba(0X778899FF);
	const Color4 LIGHTSLATEGREY       = from_rgba(0X778899FF);
	const Color4 LIGHTSTEELBLUE       = from_rgba(0XB0C4DEFF);
	const Color4 LIGHTYELLOW          = from_rgba(0XFFFFE0FF);
	const Color4 LIME                 = from_rgba(0X00FF00FF);
	const Color4 LIMEGREEN            = from_rgba(0X32CD32FF);
	const Color4 LINEN                = from_rgba(0XFAF0E6FF);
	const Color4 MAGENTA              = from_rgba(0XFF00FFFF);
	const Color4 MAROON               = from_rgba(0X800000FF);
	const Color4 MEDIUMAQUAMARINE     = from_rgba(0X66CDAAFF);
	const Color4 MEDIUMBLUE           = from_rgba(0X0000CDFF);
	const Color4 MEDIUMORCHID         = from_rgba(0XBA55D3FF);
	const Color4 MEDIUMPURPLE         = from_rgba(0X9370DBFF);
	const Color4 MEDIUMSEAGREEN       = from_rgba(0X3CB371FF);
	const Color4 MEDIUMSLATEBLUE      = from_rgba(0X7B68EEFF);
	const Color4 MEDIUMSPRINGGREEN    = from_rgba(0X00FA9AFF);
	const Color4 MEDIUMTURQUOISE      = from_rgba(0X48D1CCFF);
	const Color4 MEDIUMVIOLETRED      = from_rgba(0XC71585FF);
	const Color4 MIDNIGHTBLUE         = from_rgba(0X191970FF);
	const Color4 MINTCREAM            = from_rgba(0XF5FFFAFF);
	const Color4 MISTYROSE            = from_rgba(0XFFE4E1FF);
	const Color4 MOCCASIN             = from_rgba(0XFFE4B5FF);
	const Color4 NAVAJOWHITE          = from_rgba(0XFFDEADFF);
	const Color4 NAVY                 = from_rgba(0X000080FF);
	const Color4 OLDLACE              = from_rgba(0XFDF5E6FF);
	const Color4 OLIVE                = from_rgba(0X808000FF);
	const Color4 OLIVEDRAB            = from_rgba(0X6B8E23FF);
	const Color4 ORANGE               = from_rgba(0XFFA500FF);
	const Color4 ORANGERED            = from_rgba(0XFF4500FF);
	const Color4 ORCHID               = from_rgba(0XDA70D6FF);
	const Color4 PALEGOLDENROD        = from_rgba(0XEEE8AAFF);
	const Color4 PALEGREEN            = from_rgba(0X98FB98FF);
	const Color4 PALETURQUOISE        = from_rgba(0XAFEEEEFF);
	const Color4 PALEVIOLETRED        = from_rgba(0XDB7093FF);
	const Color4 PAPAYAWHIP           = from_rgba(0XFFEFD5FF);
	const Color4 PEACHPUFF            = from_rgba(0XFFDAB9FF);
	const Color4 PERU                 = from_rgba(0XCD853FFF);
	const Color4 PINK                 = from_rgba(0XFFC0CBFF);
	const Color4 PLUM                 = from_rgba(0XDDA0DDFF);
	const Color4 POWDERBLUE           = from_rgba(0XB0E0E6FF);
	const Color4 PURPLE               = from_rgba(0X800080FF);
	const Color4 RED                  = from_rgba(0XFF0000FF);
	const Color4 ROSYBROWN            = from_rgba(0XBC8F8FFF);
	const Color4 ROYALBLUE            = from_rgba(0X4169E1FF);
	const Color4 SADDLEBROWN          = from_rgba(0X8B4513FF);
	const Color4 SALMON               = from_rgba(0XFA8072FF);
	const Color4 SANDYBROWN           = from_rgba(0XF4A460FF);
	const Color4 SEAGREEN             = from_rgba(0X2E8B57FF);
	const Color4 SEASHELL             = from_rgba(0XFFF5EEFF);
	const Color4 SIENNA               = from_rgba(0XA0522DFF);
	const Color4 SILVER               = from_rgba(0XC0C0C0FF);
	const Color4 SKYBLUE              = from_rgba(0X87CEEBFF);
	const Color4 SLATEBLUE            = from_rgba(0X6A5ACDFF);
	const Color4 SLATEGRAY            = from_rgba(0X708090FF);
	const Color4 SLATEGREY            = from_rgba(0X708090FF);
	const Color4 SNOW                 = from_rgba(0XFFFAFAFF);
	const Color4 SPRINGGREEN          = from_rgba(0X00FF7FFF);
	const Color4 STEELBLUE            = from_rgba(0X4682B4FF);
	const Color4 TAN                  = from_rgba(0XD2B48CFF);
	const Color4 TEAL                 = from_rgba(0X008080FF);
	const Color4 THISTLE              = from_rgba(0XD8BFD8FF);
	const Color4 TOMATO               = from_rgba(0XFF6347FF);
	const Color4 TURQUOISE            = from_rgba(0X40E0D0FF);
	const Color4 VIOLET               = from_rgba(0XEE82EEFF);
	const Color4 WHEAT                = from_rgba(0XF5DEB3FF);
	const Color4 WHITE                = from_rgba(0XFFFFFFFF);
	const Color4 WHITESMOKE           = from_rgba(0XF5F5F5FF);
	const Color4 YELLOW               = from_rgba(0XFFFF00FF);
	const Color4 YELLOWGREEN          = from_rgba(0X9ACD32FF);
} // namespace color4

namespace color4
{
	inline Color4 from_rgb(int r, int g, int b)
	{
		return Color4(r * 1.0f/255.0f
			, g * 1.0f/255.0f
			, b * 1.0f/255.0f
			, 1.0f
		);
	}

	inline Color4 from_rgba(int r, int g, int b, int a)
	{
		return Color4(r * 1.0f/255.0f
			, g * 1.0f/255.0f
			, b * 1.0f/255.0f
			, a * 1.0f/255.0f
		);
	}

	inline Color4 from_rgba(uint32_t rgba)
	{
		return Color4(1.0f/255.0f * ((rgba & 0xFF000000) >> 24)
			, 1.0f/255.0f * ((rgba & 0x00FF0000) >> 16)
			, 1.0f/255.0f * ((rgba & 0x0000FF00) >> 8)
			, 1.0f/255.0f * ((rgba & 0x000000FF) >> 0)
		);
	}

	inline uint32_t to_rgb(const Color4& c)
	{
		uint32_t rgba;

		rgba =	(uint32_t)(255.0f * c.x) << 24;
		rgba |= (uint32_t)(255.0f * c.y) << 16;
		rgba |= (uint32_t)(255.0f * c.z) << 8;
		rgba |= 255;

		return rgba;
	}

	inline uint32_t to_bgr(const Color4& c)
	{
		uint32_t abgr;

		abgr =	255 << 24;
		abgr |= (uint32_t)(255.0f * c.z) << 16;
		abgr |= (uint32_t)(255.0f * c.y) << 8;
		abgr |= (uint32_t)(255.0f * c.x);

		return abgr;
	}

	inline uint32_t to_rgba(const Color4& c)
	{
		uint32_t rgba;

		rgba =	(uint32_t)(255.0f * c.x) << 24;
		rgba |= (uint32_t)(255.0f * c.y) << 16;
		rgba |= (uint32_t)(255.0f * c.z) << 8;
		rgba |= (uint32_t)(255.0f * c.w);

		return rgba;
	}

	inline uint32_t to_abgr(const Color4& c)
	{
		uint32_t abgr;

		abgr =	(uint32_t)(255.0f * c.w) << 24;
		abgr |= (uint32_t)(255.0f * c.z) << 16;
		abgr |= (uint32_t)(255.0f * c.y) << 8;
		abgr |= (uint32_t)(255.0f * c.x);

		return abgr;
	}
} // namespace color4

} // namespace crown
