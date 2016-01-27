/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "math_types.h"

namespace crown
{
/// @addtogroup Math
/// @{

/// Returns a new color from individual elements.
inline Color4 color4(f32 r, f32 g, f32 b, f32 a)
{
	Color4 c;
	c.x = r;
	c.y = g;
	c.z = b;
	c.w = a;
	return c;
}

/// Returns a new color from individual elements. Alpha is set to 255.
inline Color4 from_rgb(u8 r, u8 g, u8 b)
{
	Color4 c;
	c.x = 1.0f/255.0f * r;
	c.y = 1.0f/255.0f * g;
	c.z = 1.0f/255.0f * b;
	c.w = 1.0f;
	return c;
}

/// Returns a new color from individual elements.
inline Color4 from_rgba(u8 r, u8 g, u8 b, u8 a)
{
	Color4 c;
	c.x = 1.0f/255.0f * r;
	c.y = 1.0f/255.0f * g;
	c.z = 1.0f/255.0f * b;
	c.w = 1.0f/255.0f * a;
	return c;
}

/// Returns a new color from packed RGBA integer.
inline Color4 from_rgba(u32 rgba)
{
	Color4 c;
	c.x = 1.0f/255.0f * ((rgba & 0xff000000) >> 24);
	c.y = 1.0f/255.0f * ((rgba & 0x00ff0000) >> 16);
	c.z = 1.0f/255.0f * ((rgba & 0x0000ff00) >> 8);
	c.w = 1.0f/255.0f * ((rgba & 0x000000ff) >> 0);
	return c;
}

/// Returns the color as a packed RGBA integer. Alpha is set to 255.
inline u32 to_rgb(const Color4& c)
{
	u32 rgba;
	rgba =	(u32)(255.0f * c.x) << 24;
	rgba |= (u32)(255.0f * c.y) << 16;
	rgba |= (u32)(255.0f * c.z) << 8;
	rgba |= 255;
	return rgba;
}

/// Returns the color as a packed ABGR integer. Alpha is set to 255.
inline u32 to_bgr(const Color4& c)
{
	u32 abgr;
	abgr =	255 << 24;
	abgr |= (u32)(255.0f * c.z) << 16;
	abgr |= (u32)(255.0f * c.y) << 8;
	abgr |= (u32)(255.0f * c.x);
	return abgr;
}

/// Returns the color as a packed 32-bit integer. (RGBA order)
inline u32 to_rgba(const Color4& c)
{
	u32 rgba;
	rgba =	(u32)(255.0f * c.x) << 24;
	rgba |= (u32)(255.0f * c.y) << 16;
	rgba |= (u32)(255.0f * c.z) << 8;
	rgba |= (u32)(255.0f * c.w);
	return rgba;
}

/// Returns the color as a packed 32-bit integer. (ABGR order)
inline u32 to_abgr(const Color4& c)
{
	u32 abgr;
	abgr =	(u32)(255.0f * c.w) << 24;
	abgr |= (u32)(255.0f * c.z) << 16;
	abgr |= (u32)(255.0f * c.y) << 8;
	abgr |= (u32)(255.0f * c.x);
	return abgr;
}

// SVG 1.0 color names
const Color4 COLOR4_ALICEBLUE            = from_rgba(0xf0f8ffff);
const Color4 COLOR4_ANTIQUEWHITE         = from_rgba(0xfaebd7ff);
const Color4 COLOR4_AQUA                 = from_rgba(0x00ffffff);
const Color4 COLOR4_AQUAMARINE           = from_rgba(0x7fffd4ff);
const Color4 COLOR4_AZURE                = from_rgba(0xf0ffffff);
const Color4 COLOR4_BEIGE                = from_rgba(0xf5f5dcff);
const Color4 COLOR4_BISQUE               = from_rgba(0xffe4c4ff);
const Color4 COLOR4_BLACK                = from_rgba(0x000000ff);
const Color4 COLOR4_BLANCHEDALMOND       = from_rgba(0xffebcdff);
const Color4 COLOR4_BLUE                 = from_rgba(0x0000ffff);
const Color4 COLOR4_BLUEVIOLET           = from_rgba(0x8a2be2ff);
const Color4 COLOR4_BROWN                = from_rgba(0xa52a2aff);
const Color4 COLOR4_BURLYWOOD            = from_rgba(0xdeb887ff);
const Color4 COLOR4_CADETBLUE            = from_rgba(0x5f9ea0ff);
const Color4 COLOR4_CHARTREUSE           = from_rgba(0x7fff00ff);
const Color4 COLOR4_CHOCOLATE            = from_rgba(0xd2691eff);
const Color4 COLOR4_CORAL                = from_rgba(0xff7f50ff);
const Color4 COLOR4_CORNFLOWERBLUE       = from_rgba(0x6495edff);
const Color4 COLOR4_CORNSILK             = from_rgba(0xfff8dcff);
const Color4 COLOR4_CRIMSON              = from_rgba(0xdc143cff);
const Color4 COLOR4_CYAN                 = from_rgba(0x00ffffff);
const Color4 COLOR4_DARKBLUE             = from_rgba(0x00008bff);
const Color4 COLOR4_DARKCYAN             = from_rgba(0x008b8bff);
const Color4 COLOR4_DARKGOLDENROD        = from_rgba(0xb8860bff);
const Color4 COLOR4_DARKGRAY             = from_rgba(0xa9a9a9ff);
const Color4 COLOR4_DARKGREEN            = from_rgba(0x006400ff);
const Color4 COLOR4_DARKGREY             = from_rgba(0xa9a9a9ff);
const Color4 COLOR4_DARKKHAKI            = from_rgba(0xbdb76bff);
const Color4 COLOR4_DARKMAGENTA          = from_rgba(0x8b008bff);
const Color4 COLOR4_DARKOLIVEGREEN       = from_rgba(0x556b2fff);
const Color4 COLOR4_DARKORANGE           = from_rgba(0xff8c00ff);
const Color4 COLOR4_DARKORCHID           = from_rgba(0x9932ccff);
const Color4 COLOR4_DARKRED              = from_rgba(0x8b0000ff);
const Color4 COLOR4_DARKSALMON           = from_rgba(0xe9967aff);
const Color4 COLOR4_DARKSEAGREEN         = from_rgba(0x8fbc8fff);
const Color4 COLOR4_DARKSLATEBLUE        = from_rgba(0x483d8bff);
const Color4 COLOR4_DARKSLATEGRAY        = from_rgba(0x2f4f4fff);
const Color4 COLOR4_DARKSLATEGREY        = from_rgba(0x2f4f4fff);
const Color4 COLOR4_DARKTURQUOISE        = from_rgba(0x00ced1ff);
const Color4 COLOR4_DARKVIOLET           = from_rgba(0x9400d3ff);
const Color4 COLOR4_DEEPPINK             = from_rgba(0xff1493ff);
const Color4 COLOR4_DEEPSKYBLUE          = from_rgba(0x00bfffff);
const Color4 COLOR4_DIMGRAY              = from_rgba(0x696969ff);
const Color4 COLOR4_DIMGREY              = from_rgba(0x696969ff);
const Color4 COLOR4_DODGERBLUE           = from_rgba(0x1e90ffff);
const Color4 COLOR4_FIREBRICK            = from_rgba(0xb22222ff);
const Color4 COLOR4_FLORALWHITE          = from_rgba(0xfffaf0ff);
const Color4 COLOR4_FORESTGREEN          = from_rgba(0x228b22ff);
const Color4 COLOR4_FUCHSIA              = from_rgba(0xff00ffff);
const Color4 COLOR4_GAINSBORO            = from_rgba(0xdcdcdcff);
const Color4 COLOR4_GHOSTWHITE           = from_rgba(0xf8f8ffff);
const Color4 COLOR4_GOLD                 = from_rgba(0xffd700ff);
const Color4 COLOR4_GOLDENROD            = from_rgba(0xdaa520ff);
const Color4 COLOR4_GRAY                 = from_rgba(0x808080ff);
const Color4 COLOR4_GREEN                = from_rgba(0x008000ff);
const Color4 COLOR4_GREENYELLOW          = from_rgba(0xadff2fff);
const Color4 COLOR4_GREY                 = from_rgba(0x808080ff);
const Color4 COLOR4_HONEYDEW             = from_rgba(0xf0fff0ff);
const Color4 COLOR4_HOTPINK              = from_rgba(0xff69b4ff);
const Color4 COLOR4_INDIANRED            = from_rgba(0xcd5c5cff);
const Color4 COLOR4_INDIGO               = from_rgba(0x4b0082ff);
const Color4 COLOR4_IVORY                = from_rgba(0xfffff0ff);
const Color4 COLOR4_KHAKI                = from_rgba(0xf0e68cff);
const Color4 COLOR4_LAVENDER             = from_rgba(0xe6e6faff);
const Color4 COLOR4_LAVENDERBLUSH        = from_rgba(0xfff0f5ff);
const Color4 COLOR4_LAWNGREEN            = from_rgba(0x7cfc00ff);
const Color4 COLOR4_LEMONCHIFFON         = from_rgba(0xfffacdff);
const Color4 COLOR4_LIGHTBLUE            = from_rgba(0xadd8e6ff);
const Color4 COLOR4_LIGHTCORAL           = from_rgba(0xf08080ff);
const Color4 COLOR4_LIGHTCYAN            = from_rgba(0xe0ffffff);
const Color4 COLOR4_LIGHTGOLDENRODYELLOW = from_rgba(0xfafad2ff);
const Color4 COLOR4_LIGHTGRAY            = from_rgba(0xd3d3d3ff);
const Color4 COLOR4_LIGHTGREEN           = from_rgba(0x90ee90ff);
const Color4 COLOR4_LIGHTGREY            = from_rgba(0xd3d3d3ff);
const Color4 COLOR4_LIGHTPINK            = from_rgba(0xffb6c1ff);
const Color4 COLOR4_LIGHTSALMON          = from_rgba(0xffa07aff);
const Color4 COLOR4_LIGHTSEAGREEN        = from_rgba(0x20b2aaff);
const Color4 COLOR4_LIGHTSKYBLUE         = from_rgba(0x87cefaff);
const Color4 COLOR4_LIGHTSLATEGRAY       = from_rgba(0x778899ff);
const Color4 COLOR4_LIGHTSLATEGREY       = from_rgba(0x778899ff);
const Color4 COLOR4_LIGHTSTEELBLUE       = from_rgba(0xb0c4deff);
const Color4 COLOR4_LIGHTYELLOW          = from_rgba(0xffffe0ff);
const Color4 COLOR4_LIME                 = from_rgba(0x00ff00ff);
const Color4 COLOR4_LIMEGREEN            = from_rgba(0x32cd32ff);
const Color4 COLOR4_LINEN                = from_rgba(0xfaf0e6ff);
const Color4 COLOR4_MAGENTA              = from_rgba(0xff00ffff);
const Color4 COLOR4_MAROON               = from_rgba(0x800000ff);
const Color4 COLOR4_MEDIUMAQUAMARINE     = from_rgba(0x66cdaaff);
const Color4 COLOR4_MEDIUMBLUE           = from_rgba(0x0000cdff);
const Color4 COLOR4_MEDIUMORCHID         = from_rgba(0xba55d3ff);
const Color4 COLOR4_MEDIUMPURPLE         = from_rgba(0x9370dbff);
const Color4 COLOR4_MEDIUMSEAGREEN       = from_rgba(0x3cb371ff);
const Color4 COLOR4_MEDIUMSLATEBLUE      = from_rgba(0x7b68eeff);
const Color4 COLOR4_MEDIUMSPRINGGREEN    = from_rgba(0x00fa9aff);
const Color4 COLOR4_MEDIUMTURQUOISE      = from_rgba(0x48d1ccff);
const Color4 COLOR4_MEDIUMVIOLETRED      = from_rgba(0xc71585ff);
const Color4 COLOR4_MIDNIGHTBLUE         = from_rgba(0x191970ff);
const Color4 COLOR4_MINTCREAM            = from_rgba(0xf5fffaff);
const Color4 COLOR4_MISTYROSE            = from_rgba(0xffe4e1ff);
const Color4 COLOR4_MOCCASIN             = from_rgba(0xffe4b5ff);
const Color4 COLOR4_NAVAJOWHITE          = from_rgba(0xffdeadff);
const Color4 COLOR4_NAVY                 = from_rgba(0x000080ff);
const Color4 COLOR4_OLDLACE              = from_rgba(0xfdf5e6ff);
const Color4 COLOR4_OLIVE                = from_rgba(0x808000ff);
const Color4 COLOR4_OLIVEDRAB            = from_rgba(0x6b8e23ff);
const Color4 COLOR4_ORANGE               = from_rgba(0xffa500ff);
const Color4 COLOR4_ORANGERED            = from_rgba(0xff4500ff);
const Color4 COLOR4_ORCHID               = from_rgba(0xda70d6ff);
const Color4 COLOR4_PALEGOLDENROD        = from_rgba(0xeee8aaff);
const Color4 COLOR4_PALEGREEN            = from_rgba(0x98fb98ff);
const Color4 COLOR4_PALETURQUOISE        = from_rgba(0xafeeeeff);
const Color4 COLOR4_PALEVIOLETRED        = from_rgba(0xdb7093ff);
const Color4 COLOR4_PAPAYAWHIP           = from_rgba(0xffefd5ff);
const Color4 COLOR4_PEACHPUFF            = from_rgba(0xffdab9ff);
const Color4 COLOR4_PERU                 = from_rgba(0xcd853fff);
const Color4 COLOR4_PINK                 = from_rgba(0xffc0cbff);
const Color4 COLOR4_PLUM                 = from_rgba(0xdda0ddff);
const Color4 COLOR4_POWDERBLUE           = from_rgba(0xb0e0e6ff);
const Color4 COLOR4_PURPLE               = from_rgba(0x800080ff);
const Color4 COLOR4_RED                  = from_rgba(0xff0000ff);
const Color4 COLOR4_ROSYBROWN            = from_rgba(0xbc8f8fff);
const Color4 COLOR4_ROYALBLUE            = from_rgba(0x4169e1ff);
const Color4 COLOR4_SADDLEBROWN          = from_rgba(0x8b4513ff);
const Color4 COLOR4_SALMON               = from_rgba(0xfa8072ff);
const Color4 COLOR4_SANDYBROWN           = from_rgba(0xf4a460ff);
const Color4 COLOR4_SEAGREEN             = from_rgba(0x2e8b57ff);
const Color4 COLOR4_SEASHELL             = from_rgba(0xfff5eeff);
const Color4 COLOR4_SIENNA               = from_rgba(0xa0522dff);
const Color4 COLOR4_SILVER               = from_rgba(0xc0c0c0ff);
const Color4 COLOR4_SKYBLUE              = from_rgba(0x87ceebff);
const Color4 COLOR4_SLATEBLUE            = from_rgba(0x6a5acdff);
const Color4 COLOR4_SLATEGRAY            = from_rgba(0x708090ff);
const Color4 COLOR4_SLATEGREY            = from_rgba(0x708090ff);
const Color4 COLOR4_SNOW                 = from_rgba(0xfffafaff);
const Color4 COLOR4_SPRINGGREEN          = from_rgba(0x00ff7fff);
const Color4 COLOR4_STEELBLUE            = from_rgba(0x4682b4ff);
const Color4 COLOR4_TAN                  = from_rgba(0xd2b48cff);
const Color4 COLOR4_TEAL                 = from_rgba(0x008080ff);
const Color4 COLOR4_THISTLE              = from_rgba(0xd8bfd8ff);
const Color4 COLOR4_TOMATO               = from_rgba(0xff6347ff);
const Color4 COLOR4_TURQUOISE            = from_rgba(0x40e0d0ff);
const Color4 COLOR4_VIOLET               = from_rgba(0xee82eeff);
const Color4 COLOR4_WHEAT                = from_rgba(0xf5deb3ff);
const Color4 COLOR4_WHITE                = from_rgba(0xffffffff);
const Color4 COLOR4_WHITESMOKE           = from_rgba(0xf5f5f5ff);
const Color4 COLOR4_YELLOW               = from_rgba(0xffff00ff);
const Color4 COLOR4_YELLOWGREEN          = from_rgba(0x9acd32ff);

/// @}
} // namespace crown
