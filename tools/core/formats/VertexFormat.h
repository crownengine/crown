/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
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

namespace crown
{

/// Enumerates vertex formats.
enum VertexFormat
{
	VF_XY_FLOAT_32					= 0,	///< XY coordinates, 32-bit floating point each
	VF_XYZ_FLOAT_32					= 1,	///< XYZ coordinates, 32-bit floating point each

	VF_UV_FLOAT_32					= 2,	///< UV coordinates, 32-bit floating point each
	VF_UVT_FLOAT_32					= 3,	///< UVT coordinates, 32-bit floating point each

	VF_XYZ_NORMAL_FLOAT_32			= 4, 	///< XYZ normal coordinates, 32-bit floating point each

	VF_XYZ_UV_XYZ_NORMAL_FLOAT_32	= 5		///< XYZ coordinates, UV coordinates, XYZ normal coordinates, 32-bit floating point each
};

} // namespace crown
