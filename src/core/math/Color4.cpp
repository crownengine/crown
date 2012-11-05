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

#include "Color4.h"

namespace crown
{

const Color4 Color4::ALICEBLUE				= Color4(0XF0F8FFFF);
const Color4 Color4::ANTIQUEWHITE			= Color4(0XFAEBD7FF);
const Color4 Color4::AQUA					= Color4(0X00FFFFFF);
const Color4 Color4::AQUAMARINE				= Color4(0X7FFFD4FF);
const Color4 Color4::AZURE					= Color4(0XF0FFFFFF);
const Color4 Color4::BEIGE					= Color4(0XF5F5DCFF);
const Color4 Color4::BISQUE					= Color4(0XFFE4C4FF);
const Color4 Color4::BLACK					= Color4(0X000000FF);
const Color4 Color4::BLANCHEDALMOND			= Color4(0XFFEBCDFF);
const Color4 Color4::BLUE					= Color4(0X0000FFFF);
const Color4 Color4::BLUEVIOLET				= Color4(0X8A2BE2FF);
const Color4 Color4::BROWN					= Color4(0XA52A2AFF);
const Color4 Color4::BURLYWOOD				= Color4(0XDEB887FF);
const Color4 Color4::CADETBLUE				= Color4(0X5F9EA0FF);
const Color4 Color4::CHARTREUSE				= Color4(0X7FFF00FF);
const Color4 Color4::CHOCOLATE				= Color4(0XD2691EFF);
const Color4 Color4::CORAL					= Color4(0XFF7F50FF);
const Color4 Color4::CORNFLOWERBLUE			= Color4(0X6495EDFF);
const Color4 Color4::CORNSILK				= Color4(0XFFF8DCFF);
const Color4 Color4::CRIMSON				= Color4(0XDC143CFF);
const Color4 Color4::CYAN					= Color4(0X00FFFFFF);
const Color4 Color4::DARKBLUE				= Color4(0X00008BFF);
const Color4 Color4::DARKCYAN				= Color4(0X008B8BFF);
const Color4 Color4::DARKGOLDENROD			= Color4(0XB8860BFF);
const Color4 Color4::DARKGRAY				= Color4(0XA9A9A9FF);
const Color4 Color4::DARKGREEN				= Color4(0X006400FF);
const Color4 Color4::DARKGREY				= Color4(0XA9A9A9FF);
const Color4 Color4::DARKKHAKI				= Color4(0XBDB76BFF);
const Color4 Color4::DARKMAGENTA			= Color4(0X8B008BFF);
const Color4 Color4::DARKOLIVEGREEN			= Color4(0X556B2FFF);
const Color4 Color4::DARKORANGE				= Color4(0XFF8C00FF);
const Color4 Color4::DARKORCHID				= Color4(0X9932CCFF);
const Color4 Color4::DARKRED				= Color4(0X8B0000FF);
const Color4 Color4::DARKSALMON				= Color4(0XE9967AFF);
const Color4 Color4::DARKSEAGREEN			= Color4(0X8FBC8FFF);
const Color4 Color4::DARKSLATEBLUE			= Color4(0X483D8BFF);
const Color4 Color4::DARKSLATEGRAY			= Color4(0X2F4F4FFF);
const Color4 Color4::DARKSLATEGREY			= Color4(0X2F4F4FFF);
const Color4 Color4::DARKTURQUOISE			= Color4(0X00CED1FF);
const Color4 Color4::DARKVIOLET				= Color4(0X9400D3FF);
const Color4 Color4::DEEPPINK				= Color4(0XFF1493FF);
const Color4 Color4::DEEPSKYBLUE			= Color4(0X00BFFFFF);
const Color4 Color4::DIMGRAY				= Color4(0X696969FF);
const Color4 Color4::DIMGREY				= Color4(0X696969FF);
const Color4 Color4::DODGERBLUE				= Color4(0X1E90FFFF);
const Color4 Color4::FIREBRICK				= Color4(0XB22222FF);
const Color4 Color4::FLORALWHITE			= Color4(0XFFFAF0FF);
const Color4 Color4::FORESTGREEN			= Color4(0X228B22FF);
const Color4 Color4::FUCHSIA				= Color4(0XFF00FFFF);
const Color4 Color4::GAINSBORO				= Color4(0XDCDCDCFF);
const Color4 Color4::GHOSTWHITE				= Color4(0XF8F8FFFF);
const Color4 Color4::GOLD					= Color4(0XFFD700FF);
const Color4 Color4::GOLDENROD				= Color4(0XDAA520FF);
const Color4 Color4::GRAY					= Color4(0X808080FF);
const Color4 Color4::GREEN					= Color4(0X008000FF);
const Color4 Color4::GREENYELLOW			= Color4(0XADFF2FFF);
const Color4 Color4::GREY					= Color4(0X808080FF);
const Color4 Color4::HONEYDEW				= Color4(0XF0FFF0FF);
const Color4 Color4::HOTPINK				= Color4(0XFF69B4FF);
const Color4 Color4::INDIANRED				= Color4(0XCD5C5CFF);
const Color4 Color4::INDIGO					= Color4(0X4B0082FF);
const Color4 Color4::IVORY					= Color4(0XFFFFF0FF);
const Color4 Color4::KHAKI					= Color4(0XF0E68CFF);
const Color4 Color4::LAVENDER				= Color4(0XE6E6FAFF);
const Color4 Color4::LAVENDERBLUSH			= Color4(0XFFF0F5FF);
const Color4 Color4::LAWNGREEN				= Color4(0X7CFC00FF);
const Color4 Color4::LEMONCHIFFON			= Color4(0XFFFACDFF);
const Color4 Color4::LIGHTBLUE				= Color4(0XADD8E6FF);
const Color4 Color4::LIGHTCORAL				= Color4(0XF08080FF);
const Color4 Color4::LIGHTCYAN				= Color4(0XE0FFFFFF);
const Color4 Color4::LIGHTGOLDENRODYELLOW	= Color4(0XFAFAD2FF);
const Color4 Color4::LIGHTGRAY				= Color4(0XD3D3D3FF);
const Color4 Color4::LIGHTGREEN				= Color4(0X90EE90FF);
const Color4 Color4::LIGHTGREY				= Color4(0XD3D3D3FF);
const Color4 Color4::LIGHTPINK				= Color4(0XFFB6C1FF);
const Color4 Color4::LIGHTSALMON			= Color4(0XFFA07AFF);
const Color4 Color4::LIGHTSEAGREEN			= Color4(0X20B2AAFF);
const Color4 Color4::LIGHTSKYBLUE			= Color4(0X87CEFAFF);
const Color4 Color4::LIGHTSLATEGRAY			= Color4(0X778899FF);
const Color4 Color4::LIGHTSLATEGREY			= Color4(0X778899FF);
const Color4 Color4::LIGHTSTEELBLUE			= Color4(0XB0C4DEFF);
const Color4 Color4::LIGHTYELLOW			= Color4(0XFFFFE0FF);
const Color4 Color4::LIME					= Color4(0X00FF00FF);
const Color4 Color4::LIMEGREEN				= Color4(0X32CD32FF);
const Color4 Color4::LINEN					= Color4(0XFAF0E6FF);
const Color4 Color4::MAGENTA				= Color4(0XFF00FFFF);
const Color4 Color4::MAROON					= Color4(0X800000FF);
const Color4 Color4::MEDIUMAQUAMARINE		= Color4(0X66CDAAFF);
const Color4 Color4::MEDIUMBLUE				= Color4(0X0000CDFF);
const Color4 Color4::MEDIUMORCHID			= Color4(0XBA55D3FF);
const Color4 Color4::MEDIUMPURPLE			= Color4(0X9370DBFF);
const Color4 Color4::MEDIUMSEAGREEN			= Color4(0X3CB371FF);
const Color4 Color4::MEDIUMSLATEBLUE		= Color4(0X7B68EEFF);
const Color4 Color4::MEDIUMSPRINGGREEN		= Color4(0X00FA9AFF);
const Color4 Color4::MEDIUMTURQUOISE		= Color4(0X48D1CCFF);
const Color4 Color4::MEDIUMVIOLETRED		= Color4(0XC71585FF);
const Color4 Color4::MIDNIGHTBLUE			= Color4(0X191970FF);
const Color4 Color4::MINTCREAM				= Color4(0XF5FFFAFF);
const Color4 Color4::MISTYROSE				= Color4(0XFFE4E1FF);
const Color4 Color4::MOCCASIN				= Color4(0XFFE4B5FF);
const Color4 Color4::NAVAJOWHITE			= Color4(0XFFDEADFF);
const Color4 Color4::NAVY					= Color4(0X000080FF);
const Color4 Color4::OLDLACE				= Color4(0XFDF5E6FF);
const Color4 Color4::OLIVE					= Color4(0X808000FF);
const Color4 Color4::OLIVEDRAB				= Color4(0X6B8E23FF);
const Color4 Color4::ORANGE					= Color4(0XFFA500FF);
const Color4 Color4::ORANGERED				= Color4(0XFF4500FF);
const Color4 Color4::ORCHID					= Color4(0XDA70D6FF);
const Color4 Color4::PALEGOLDENROD			= Color4(0XEEE8AAFF);
const Color4 Color4::PALEGREEN				= Color4(0X98FB98FF);
const Color4 Color4::PALETURQUOISE			= Color4(0XAFEEEEFF);
const Color4 Color4::PALEVIOLETRED			= Color4(0XDB7093FF);
const Color4 Color4::PAPAYAWHIP				= Color4(0XFFEFD5FF);
const Color4 Color4::PEACHPUFF				= Color4(0XFFDAB9FF);
const Color4 Color4::PERU					= Color4(0XCD853FFF);
const Color4 Color4::PINK					= Color4(0XFFC0CBFF);
const Color4 Color4::PLUM					= Color4(0XDDA0DDFF);
const Color4 Color4::POWDERBLUE				= Color4(0XB0E0E6FF);
const Color4 Color4::PURPLE					= Color4(0X800080FF);
const Color4 Color4::RED					= Color4(0XFF0000FF);
const Color4 Color4::ROSYBROWN				= Color4(0XBC8F8FFF);
const Color4 Color4::ROYALBLUE				= Color4(0X4169E1FF);
const Color4 Color4::SADDLEBROWN			= Color4(0X8B4513FF);
const Color4 Color4::SALMON					= Color4(0XFA8072FF);
const Color4 Color4::SANDYBROWN				= Color4(0XF4A460FF);
const Color4 Color4::SEAGREEN				= Color4(0X2E8B57FF);
const Color4 Color4::SEASHELL				= Color4(0XFFF5EEFF);
const Color4 Color4::SIENNA					= Color4(0XA0522DFF);
const Color4 Color4::SILVER					= Color4(0XC0C0C0FF);
const Color4 Color4::SKYBLUE				= Color4(0X87CEEBFF);
const Color4 Color4::SLATEBLUE				= Color4(0X6A5ACDFF);
const Color4 Color4::SLATEGRAY				= Color4(0X708090FF);
const Color4 Color4::SLATEGREY				= Color4(0X708090FF);
const Color4 Color4::SNOW					= Color4(0XFFFAFAFF);
const Color4 Color4::SPRINGGREEN			= Color4(0X00FF7FFF);
const Color4 Color4::STEELBLUE				= Color4(0X4682B4FF);
const Color4 Color4::TAN					= Color4(0XD2B48CFF);
const Color4 Color4::TEAL					= Color4(0X008080FF);
const Color4 Color4::THISTLE				= Color4(0XD8BFD8FF);
const Color4 Color4::TOMATO					= Color4(0XFF6347FF);
const Color4 Color4::TURQUOISE				= Color4(0X40E0D0FF);
const Color4 Color4::VIOLET					= Color4(0XEE82EEFF);
const Color4 Color4::WHEAT					= Color4(0XF5DEB3FF);
const Color4 Color4::WHITE					= Color4(0XFFFFFFFF);
const Color4 Color4::WHITESMOKE				= Color4(0XF5F5F5FF);
const Color4 Color4::YELLOW					= Color4(0XFFFF00FF);
const Color4 Color4::YELLOWGREEN			= Color4(0X9ACD32FF);

} //namespace crown

