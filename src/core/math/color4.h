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
	const Color4 ALICEBLUE            = from_rgba(0xf0f8ffff);
	const Color4 ANTIQUEWHITE         = from_rgba(0xfaebd7ff);
	const Color4 AQUA                 = from_rgba(0x00ffffff);
	const Color4 AQUAMARINE           = from_rgba(0x7fffd4ff);
	const Color4 AZURE                = from_rgba(0xf0ffffff);
	const Color4 BEIGE                = from_rgba(0xf5f5dcff);
	const Color4 BISQUE               = from_rgba(0xffe4c4ff);
	const Color4 BLACK                = from_rgba(0x000000ff);
	const Color4 BLANCHEDALMOND       = from_rgba(0xffebcdff);
	const Color4 BLUE                 = from_rgba(0x0000ffff);
	const Color4 BLUEVIOLET           = from_rgba(0x8a2be2ff);
	const Color4 BROWN                = from_rgba(0xa52a2aff);
	const Color4 BURLYWOOD            = from_rgba(0xdeb887ff);
	const Color4 CADETBLUE            = from_rgba(0x5f9ea0ff);
	const Color4 CHARTREUSE           = from_rgba(0x7fff00ff);
	const Color4 CHOCOLATE            = from_rgba(0xd2691eff);
	const Color4 CORAL                = from_rgba(0xff7f50ff);
	const Color4 CORNFLOWERBLUE       = from_rgba(0x6495edff);
	const Color4 CORNSILK             = from_rgba(0xfff8dcff);
	const Color4 CRIMSON              = from_rgba(0xdc143cff);
	const Color4 CYAN                 = from_rgba(0x00ffffff);
	const Color4 DARKBLUE             = from_rgba(0x00008bff);
	const Color4 DARKCYAN             = from_rgba(0x008b8bff);
	const Color4 DARKGOLDENROD        = from_rgba(0xb8860bff);
	const Color4 DARKGRAY             = from_rgba(0xa9a9a9ff);
	const Color4 DARKGREEN            = from_rgba(0x006400ff);
	const Color4 DARKGREY             = from_rgba(0xa9a9a9ff);
	const Color4 DARKKHAKI            = from_rgba(0xbdb76bff);
	const Color4 DARKMAGENTA          = from_rgba(0x8b008bff);
	const Color4 DARKOLIVEGREEN       = from_rgba(0x556b2fff);
	const Color4 DARKORANGE           = from_rgba(0xff8c00ff);
	const Color4 DARKORCHID           = from_rgba(0x9932ccff);
	const Color4 DARKRED              = from_rgba(0x8b0000ff);
	const Color4 DARKSALMON           = from_rgba(0xe9967aff);
	const Color4 DARKSEAGREEN         = from_rgba(0x8fbc8fff);
	const Color4 DARKSLATEBLUE        = from_rgba(0x483d8bff);
	const Color4 DARKSLATEGRAY        = from_rgba(0x2f4f4fff);
	const Color4 DARKSLATEGREY        = from_rgba(0x2f4f4fff);
	const Color4 DARKTURQUOISE        = from_rgba(0x00ced1ff);
	const Color4 DARKVIOLET           = from_rgba(0x9400d3ff);
	const Color4 DEEPPINK             = from_rgba(0xff1493ff);
	const Color4 DEEPSKYBLUE          = from_rgba(0x00bfffff);
	const Color4 DIMGRAY              = from_rgba(0x696969ff);
	const Color4 DIMGREY              = from_rgba(0x696969ff);
	const Color4 DODGERBLUE           = from_rgba(0x1e90ffff);
	const Color4 FIREBRICK            = from_rgba(0xb22222ff);
	const Color4 FLORALWHITE          = from_rgba(0xfffaf0ff);
	const Color4 FORESTGREEN          = from_rgba(0x228b22ff);
	const Color4 FUCHSIA              = from_rgba(0xff00ffff);
	const Color4 GAINSBORO            = from_rgba(0xdcdcdcff);
	const Color4 GHOSTWHITE           = from_rgba(0xf8f8ffff);
	const Color4 GOLD                 = from_rgba(0xffd700ff);
	const Color4 GOLDENROD            = from_rgba(0xdaa520ff);
	const Color4 GRAY                 = from_rgba(0x808080ff);
	const Color4 GREEN                = from_rgba(0x008000ff);
	const Color4 GREENYELLOW          = from_rgba(0xadff2fff);
	const Color4 GREY                 = from_rgba(0x808080ff);
	const Color4 HONEYDEW             = from_rgba(0xf0fff0ff);
	const Color4 HOTPINK              = from_rgba(0xff69b4ff);
	const Color4 INDIANRED            = from_rgba(0xcd5c5cff);
	const Color4 INDIGO               = from_rgba(0x4b0082ff);
	const Color4 IVORY                = from_rgba(0xfffff0ff);
	const Color4 KHAKI                = from_rgba(0xf0e68cff);
	const Color4 LAVENDER             = from_rgba(0xe6e6faff);
	const Color4 LAVENDERBLUSH        = from_rgba(0xfff0f5ff);
	const Color4 LAWNGREEN            = from_rgba(0x7cfc00ff);
	const Color4 LEMONCHIFFON         = from_rgba(0xfffacdff);
	const Color4 LIGHTBLUE            = from_rgba(0xadd8e6ff);
	const Color4 LIGHTCORAL           = from_rgba(0xf08080ff);
	const Color4 LIGHTCYAN            = from_rgba(0xe0ffffff);
	const Color4 LIGHTGOLDENRODYELLOW = from_rgba(0xfafad2ff);
	const Color4 LIGHTGRAY            = from_rgba(0xd3d3d3ff);
	const Color4 LIGHTGREEN           = from_rgba(0x90ee90ff);
	const Color4 LIGHTGREY            = from_rgba(0xd3d3d3ff);
	const Color4 LIGHTPINK            = from_rgba(0xffb6c1ff);
	const Color4 LIGHTSALMON          = from_rgba(0xffa07aff);
	const Color4 LIGHTSEAGREEN        = from_rgba(0x20b2aaff);
	const Color4 LIGHTSKYBLUE         = from_rgba(0x87cefaff);
	const Color4 LIGHTSLATEGRAY       = from_rgba(0x778899ff);
	const Color4 LIGHTSLATEGREY       = from_rgba(0x778899ff);
	const Color4 LIGHTSTEELBLUE       = from_rgba(0xb0c4deff);
	const Color4 LIGHTYELLOW          = from_rgba(0xffffe0ff);
	const Color4 LIME                 = from_rgba(0x00ff00ff);
	const Color4 LIMEGREEN            = from_rgba(0x32cd32ff);
	const Color4 LINEN                = from_rgba(0xfaf0e6ff);
	const Color4 MAGENTA              = from_rgba(0xff00ffff);
	const Color4 MAROON               = from_rgba(0x800000ff);
	const Color4 MEDIUMAQUAMARINE     = from_rgba(0x66cdaaff);
	const Color4 MEDIUMBLUE           = from_rgba(0x0000cdff);
	const Color4 MEDIUMORCHID         = from_rgba(0xba55d3ff);
	const Color4 MEDIUMPURPLE         = from_rgba(0x9370dbff);
	const Color4 MEDIUMSEAGREEN       = from_rgba(0x3cb371ff);
	const Color4 MEDIUMSLATEBLUE      = from_rgba(0x7b68eeff);
	const Color4 MEDIUMSPRINGGREEN    = from_rgba(0x00fa9aff);
	const Color4 MEDIUMTURQUOISE      = from_rgba(0x48d1ccff);
	const Color4 MEDIUMVIOLETRED      = from_rgba(0xc71585ff);
	const Color4 MIDNIGHTBLUE         = from_rgba(0x191970ff);
	const Color4 MINTCREAM            = from_rgba(0xf5fffaff);
	const Color4 MISTYROSE            = from_rgba(0xffe4e1ff);
	const Color4 MOCCASIN             = from_rgba(0xffe4b5ff);
	const Color4 NAVAJOWHITE          = from_rgba(0xffdeadff);
	const Color4 NAVY                 = from_rgba(0x000080ff);
	const Color4 OLDLACE              = from_rgba(0xfdf5e6ff);
	const Color4 OLIVE                = from_rgba(0x808000ff);
	const Color4 OLIVEDRAB            = from_rgba(0x6b8e23ff);
	const Color4 ORANGE               = from_rgba(0xffa500ff);
	const Color4 ORANGERED            = from_rgba(0xff4500ff);
	const Color4 ORCHID               = from_rgba(0xda70d6ff);
	const Color4 PALEGOLDENROD        = from_rgba(0xeee8aaff);
	const Color4 PALEGREEN            = from_rgba(0x98fb98ff);
	const Color4 PALETURQUOISE        = from_rgba(0xafeeeeff);
	const Color4 PALEVIOLETRED        = from_rgba(0xdb7093ff);
	const Color4 PAPAYAWHIP           = from_rgba(0xffefd5ff);
	const Color4 PEACHPUFF            = from_rgba(0xffdab9ff);
	const Color4 PERU                 = from_rgba(0xcd853fff);
	const Color4 PINK                 = from_rgba(0xffc0cbff);
	const Color4 PLUM                 = from_rgba(0xdda0ddff);
	const Color4 POWDERBLUE           = from_rgba(0xb0e0e6ff);
	const Color4 PURPLE               = from_rgba(0x800080ff);
	const Color4 RED                  = from_rgba(0xff0000ff);
	const Color4 ROSYBROWN            = from_rgba(0xbc8f8fff);
	const Color4 ROYALBLUE            = from_rgba(0x4169e1ff);
	const Color4 SADDLEBROWN          = from_rgba(0x8b4513ff);
	const Color4 SALMON               = from_rgba(0xfa8072ff);
	const Color4 SANDYBROWN           = from_rgba(0xf4a460ff);
	const Color4 SEAGREEN             = from_rgba(0x2e8b57ff);
	const Color4 SEASHELL             = from_rgba(0xfff5eeff);
	const Color4 SIENNA               = from_rgba(0xa0522dff);
	const Color4 SILVER               = from_rgba(0xc0c0c0ff);
	const Color4 SKYBLUE              = from_rgba(0x87ceebff);
	const Color4 SLATEBLUE            = from_rgba(0x6a5acdff);
	const Color4 SLATEGRAY            = from_rgba(0x708090ff);
	const Color4 SLATEGREY            = from_rgba(0x708090ff);
	const Color4 SNOW                 = from_rgba(0xfffafaff);
	const Color4 SPRINGGREEN          = from_rgba(0x00ff7fff);
	const Color4 STEELBLUE            = from_rgba(0x4682b4ff);
	const Color4 TAN                  = from_rgba(0xd2b48cff);
	const Color4 TEAL                 = from_rgba(0x008080ff);
	const Color4 THISTLE              = from_rgba(0xd8bfd8ff);
	const Color4 TOMATO               = from_rgba(0xff6347ff);
	const Color4 TURQUOISE            = from_rgba(0x40e0d0ff);
	const Color4 VIOLET               = from_rgba(0xee82eeff);
	const Color4 WHEAT                = from_rgba(0xf5deb3ff);
	const Color4 WHITE                = from_rgba(0xffffffff);
	const Color4 WHITESMOKE           = from_rgba(0xf5f5f5ff);
	const Color4 YELLOW               = from_rgba(0xffff00ff);
	const Color4 YELLOWGREEN          = from_rgba(0x9acd32ff);
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
		return Color4(1.0f/255.0f * ((rgba & 0xff000000) >> 24)
			, 1.0f/255.0f * ((rgba & 0x00ff0000) >> 16)
			, 1.0f/255.0f * ((rgba & 0x0000ff00) >> 8)
			, 1.0f/255.0f * ((rgba & 0x000000ff) >> 0)
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
