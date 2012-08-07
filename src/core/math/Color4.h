/*
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "Types.h"
#include "MathUtils.h"
#include "Str.h"

namespace Crown
{

/**
	RGBA color.

	Maintains 3 float values each representing a primary
	color intensity. An extra component is used as transparency
	coefficient. Each component ranges from 0.0 to 1.0; the higher
	the value the brighter the color. An alpha value of 0.0 means
	totally transparent while a value of 1.0 means totally opaque.

	It also offers a set of color name constants compliant with SVG 1.0.
*/
class Color4
{

public:

	float					r, g, b, a;

							Color4();									//!< Constructor, does nothing for efficiency
							~Color4();									//!< Destructor
							Color4(const Color4& c);					//!< Copy constructor
							Color4(float r, float g, float b);			//!< Construct from three values
							Color4(int r, int g, int b);				//!< Construct from three values
							Color4(float r, float g, float b, float a); //!< Construct from four values
							Color4(int r, int g, int b, int a);			//!< Construct from four values
	explicit				Color4(float c[4]);							//!< Construct from four values
	explicit				Color4(uint rgba);							//!< Construct from 32-bit integer (red at MSB, alpha at LSB)

	uint					GetAsRGB() const;	//!< Returns the color as a single 32-bit packed value. (RGBA order, alpha assumed = 255)
	uint					GetAsBGR() const;	//!< Returns the color as a single 32-bit packed value. (ABGR order, alpha assumed = 255)
	uint					GetAsRGBA() const;	//!< Returns the color as a single 32-bit packed value. (RGBA order)
	uint					GetAsABGR() const;	//!< Returns the color as a single 32-bit packed value. (ABGR order)

	float					operator[](uint i) const;					//!< Random access by index
	float&					operator[](uint i);							//!< Random access by index

	Color4					operator+(const Color4& c) const;			//!< Addition
	Color4&					operator+=(const Color4& c);				//!< Addition
	Color4					operator-(const Color4& c) const;			//!< Subtraction
	Color4&					operator-=(const Color4& c);				//!< Subtraction
	Color4					operator*(const Color4& c) const;			//!< Multiplication
	Color4&					operator*=(const Color4& c);				//!< Multiplication
	Color4					operator*(float scalar) const;				//!< Scalar multiplication
	Color4&					operator*=(float scalar);					//!< Scalar multiplication

	bool					operator==(const Color4& other) const;		//!< Equality operator
	bool					operator!=(const Color4& other) const;		//!< Disequality operator

	float*					ToFloatPtr();								//!< Returns the pointer to the color's data.
	const float*			ToFloatPtr() const;							//!< Returns the pointer to the color's data.
	Str						ToStr() const;								//!< Returns a Str containing the colors' components.

	// SVG 1.0 color names
	static const Color4		ALICEBLUE;
	static const Color4		ANTIQUEWHITE;
	static const Color4		AQUA;
	static const Color4		AQUAMARINE;
	static const Color4		AZURE;
	static const Color4		BEIGE;
	static const Color4		BISQUE;
	static const Color4		BLACK;
	static const Color4		BLANCHEDALMOND;
	static const Color4		BLUE;
	static const Color4		BLUEVIOLET;
	static const Color4		BROWN;
	static const Color4		BURLYWOOD;
	static const Color4		CADETBLUE;
	static const Color4		CHARTREUSE;
	static const Color4		CHOCOLATE;
	static const Color4		CORAL;
	static const Color4		CORNFLOWERBLUE;
	static const Color4		CORNSILK;
	static const Color4		CRIMSON;
	static const Color4		CYAN;
	static const Color4		DARKBLUE;
	static const Color4		DARKCYAN;
	static const Color4		DARKGOLDENROD;
	static const Color4		DARKGRAY;
	static const Color4		DARKGREEN;
	static const Color4		DARKGREY;
	static const Color4		DARKKHAKI;
	static const Color4		DARKMAGENTA;
	static const Color4		DARKOLIVEGREEN;
	static const Color4		DARKORANGE;
	static const Color4		DARKORCHID;
	static const Color4		DARKRED;
	static const Color4		DARKSALMON;
	static const Color4		DARKSEAGREEN;
	static const Color4		DARKSLATEBLUE;
	static const Color4		DARKSLATEGRAY;
	static const Color4		DARKSLATEGREY;
	static const Color4		DARKTURQUOISE;
	static const Color4		DARKVIOLET;
	static const Color4		DEEPPINK;
	static const Color4		DEEPSKYBLUE;
	static const Color4		DIMGRAY;
	static const Color4		DIMGREY;
	static const Color4		DODGERBLUE;
	static const Color4		FIREBRICK;
	static const Color4		FLORALWHITE;
	static const Color4		FORESTGREEN;
	static const Color4		FUCHSIA;
	static const Color4		GAINSBORO;
	static const Color4		GHOSTWHITE;
	static const Color4		GOLD;
	static const Color4		GOLDENROD;
	static const Color4		GRAY;
	static const Color4		GREEN;
	static const Color4		GREENYELLOW;
	static const Color4		GREY;
	static const Color4		HONEYDEW;
	static const Color4		HOTPINK;
	static const Color4		INDIANRED;
	static const Color4		INDIGO;
	static const Color4		IVORY;
	static const Color4		KHAKI;
	static const Color4		LAVENDER;
	static const Color4		LAVENDERBLUSH;
	static const Color4		LAWNGREEN;
	static const Color4		LEMONCHIFFON;
	static const Color4		LIGHTBLUE;
	static const Color4		LIGHTCORAL;
	static const Color4		LIGHTCYAN;
	static const Color4		LIGHTGOLDENRODYELLOW;
	static const Color4		LIGHTGRAY;
	static const Color4		LIGHTGREEN;
	static const Color4		LIGHTGREY;
	static const Color4		LIGHTPINK;
	static const Color4		LIGHTSALMON;
	static const Color4		LIGHTSEAGREEN;
	static const Color4		LIGHTSKYBLUE;
	static const Color4		LIGHTSLATEGRAY;
	static const Color4		LIGHTSLATEGREY;
	static const Color4		LIGHTSTEELBLUE;
	static const Color4		LIGHTYELLOW;
	static const Color4		LIME;
	static const Color4		LIMEGREEN;
	static const Color4		LINEN;
	static const Color4		MAGENTA;
	static const Color4		MAROON;
	static const Color4		MEDIUMAQUAMARINE;
	static const Color4		MEDIUMBLUE;
	static const Color4		MEDIUMORCHID;
	static const Color4		MEDIUMPURPLE;
	static const Color4		MEDIUMSEAGREEN;
	static const Color4		MEDIUMSLATEBLUE;
	static const Color4		MEDIUMSPRINGGREEN;
	static const Color4		MEDIUMTURQUOISE;
	static const Color4		MEDIUMVIOLETRED;
	static const Color4		MIDNIGHTBLUE;
	static const Color4		MINTCREAM;
	static const Color4		MISTYROSE;
	static const Color4		MOCCASIN;
	static const Color4		NAVAJOWHITE;
	static const Color4		NAVY;
	static const Color4		OLDLACE;
	static const Color4		OLIVE;
	static const Color4		OLIVEDRAB;
	static const Color4		ORANGE;
	static const Color4		ORANGERED;
	static const Color4		ORCHID;
	static const Color4		PALEGOLDENROD;
	static const Color4		PALEGREEN;
	static const Color4		PALETURQUOISE;
	static const Color4		PALEVIOLETRED;
	static const Color4		PAPAYAWHIP;
	static const Color4		PEACHPUFF;
	static const Color4		PERU;
	static const Color4		PINK;
	static const Color4		PLUM;
	static const Color4		POWDERBLUE;
	static const Color4		PURPLE;
	static const Color4		RED;
	static const Color4		ROSYBROWN;
	static const Color4		ROYALBLUE;
	static const Color4		SADDLEBROWN;
	static const Color4		SALMON;
	static const Color4		SANDYBROWN;
	static const Color4		SEAGREEN;
	static const Color4		SEASHELL;
	static const Color4		SIENNA;
	static const Color4		SILVER;
	static const Color4		SKYBLUE;
	static const Color4		SLATEBLUE;
	static const Color4		SLATEGRAY;
	static const Color4		SLATEGREY;
	static const Color4		SNOW;
	static const Color4		SPRINGGREEN;
	static const Color4		STEELBLUE;
	static const Color4		TAN;
	static const Color4		TEAL;
	static const Color4		THISTLE;
	static const Color4		TOMATO;
	static const Color4		TURQUOISE;
	static const Color4		VIOLET;
	static const Color4		WHEAT;
	static const Color4		WHITE;
	static const Color4		WHITESMOKE;
	static const Color4		YELLOW;
	static const Color4		YELLOWGREEN;
};

//-----------------------------------------------------------------------------
inline Color4::Color4()
{
}

//-----------------------------------------------------------------------------
inline Color4::~Color4()
{
}

//-----------------------------------------------------------------------------
inline Color4::Color4(const Color4& c) : r(c.r), g(c.g), b(c.b), a(c.a)
{
}

//-----------------------------------------------------------------------------
inline Color4::Color4(float r, float g, float b)
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = 1.0f;
}

//-----------------------------------------------------------------------------
inline Color4::Color4(int r, int g, int b)
{
	this->r = r * Math::ONE_OVER_255;
	this->g = g * Math::ONE_OVER_255;
	this->b = b * Math::ONE_OVER_255;
	this->a = 1.0f;
}

//-----------------------------------------------------------------------------
inline Color4::Color4(float r, float g, float b, float a)
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}

//-----------------------------------------------------------------------------
inline Color4::Color4(int r, int g, int b, int a)
{
	this->r = r * Math::ONE_OVER_255;
	this->g = g * Math::ONE_OVER_255;
	this->b = b * Math::ONE_OVER_255;
	this->a = a * Math::ONE_OVER_255;
}

//-----------------------------------------------------------------------------
inline Color4::Color4(float c[4])
{
	r = c[0];
	g = c[1];
	b = c[2];
	a = c[3];
}

//-----------------------------------------------------------------------------
inline Color4::Color4(uint rgba)
{
	r = Math::ONE_OVER_255 * ((rgba & 0xFF000000) >> 24);
	g = Math::ONE_OVER_255 * ((rgba & 0x00FF0000) >> 16);
	b = Math::ONE_OVER_255 * ((rgba & 0x0000FF00) >> 8);
	a = Math::ONE_OVER_255 * (rgba & 0x000000FF);
}

//-----------------------------------------------------------------------------
inline float* Color4::ToFloatPtr()
{
	return &r;
}

//-----------------------------------------------------------------------------
inline const float* Color4::ToFloatPtr() const
{
	return &r;
}

//-----------------------------------------------------------------------------
inline Str Color4::ToStr() const
{
	Str tmp;

	tmp = Str("[ r: ") + r + Str(" g: ") + g + Str(" b: ") + b + Str(" a: ") + a + Str(" ]\n");

	return tmp;
}

//-----------------------------------------------------------------------------
inline uint Color4::GetAsRGB() const
{
	uint rgba;

	rgba =	(uint)(255.0f * r) << 24;
	rgba |= (uint)(255.0f * g) << 16;
	rgba |= (uint)(255.0f * b) << 8;
	rgba |= 255;

	return rgba;
}

//-----------------------------------------------------------------------------
inline uint Color4::GetAsBGR() const
{
	uint abgr;

	abgr =	255 << 24;
	abgr |= (uint)(255.0f * b) << 16;
	abgr |= (uint)(255.0f * g) << 8;
	abgr |= (uint)(255.0f * r);

	return abgr;
}

//-----------------------------------------------------------------------------
inline uint Color4::GetAsRGBA() const
{
	uint rgba;

	rgba =	(uint)(255.0f * r) << 24;
	rgba |= (uint)(255.0f * g) << 16;
	rgba |= (uint)(255.0f * b) << 8;
	rgba |= (uint)(255.0f * a);

	return rgba;
}

//-----------------------------------------------------------------------------
inline uint Color4::GetAsABGR() const
{
	uint abgr;

	abgr =	(uint)(255.0f * a) << 24;
	abgr |= (uint)(255.0f * b) << 16;
	abgr |= (uint)(255.0f * g) << 8;
	abgr |= (uint)(255.0f * r);

	return abgr;
}

//-----------------------------------------------------------------------------
inline float Color4::operator[](uint i) const
{
	return (&r)[i];
}

//-----------------------------------------------------------------------------
inline float& Color4::operator[](uint i)
{
	return (&r)[i];
}

//-----------------------------------------------------------------------------
inline Color4 Color4::operator+(const Color4& c) const
{
	return Color4(r + c.r, g + c.g, b + c.b, a + c.a);
}

//-----------------------------------------------------------------------------
inline Color4& Color4::operator+=(const Color4& c)
{
	r += c.r;
	g += c.g;
	b += c.b;
	a += c.a;

	return *this;
}

//-----------------------------------------------------------------------------
inline Color4 Color4::operator-(const Color4& c) const
{
	Color4 tmp;

	tmp.r = r - c.r;
	tmp.g = g - c.g;
	tmp.b = b - c.b;

	return tmp;
}

//-----------------------------------------------------------------------------
inline Color4& Color4::operator-=(const Color4& c)
{
	r -= c.r;
	g -= c.g;
	b -= c.b;

	return *this;
}

//-----------------------------------------------------------------------------
inline Color4 Color4::operator*(const Color4& c) const
{
	return Color4(r * c.r, g * c.g, b * c.b, a * c.a);
}

//-----------------------------------------------------------------------------
inline Color4& Color4::operator*=(const Color4& c)
{
	r *= c.r;
	g *= c.g;
	b *= c.b;
	a *= c.a;

	return *this;
}

//-----------------------------------------------------------------------------
inline Color4 Color4::operator*(float scalar) const
{
	return Color4(r * scalar, g * scalar, b * scalar, a * scalar);
}

//-----------------------------------------------------------------------------
inline Color4& Color4::operator*=(float scalar)
{
	r *= scalar;
	g *= scalar;
	b *= scalar;
	a *= scalar;

	return *this;
}

//-----------------------------------------------------------------------------
inline bool Color4::operator==(const Color4& other) const
{
	return (r == other.r && g == other.g && b == other.b && a == other.a);
}

//-----------------------------------------------------------------------------
inline bool Color4::operator!=(const Color4& other) const
{
	return (r != other.r || g != other.g || b != other.b || a != other.a);
}

//-----------------------------------------------------------------------------
inline Color4 operator*(float scalar, const Color4& color)
{
	return Color4(color.r * scalar, color.g * scalar, color.b * scalar, color.a * scalar);
}

} // namespace Crown

