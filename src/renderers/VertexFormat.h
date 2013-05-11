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

namespace crown
{

/// Enumerates vertex formats.
enum VertexFormat
{
	VF_XY_FLOAT_32,					///< XY coordinates, 32-bit floating point each
	VF_XYZ_FLOAT_32,				///< XYZ coordinates, 32-bit floating point each

	VF_UV_FLOAT_32,					///< UV coordinates, 32-bit floating point each
	VF_UVT_FLOAT_32,				///< UVT coordinates, 32-bit floating point each

	VF_XYZ_NORMAL_FLOAT_32,			///< XYZ normal coordinates, 32-bit floating point each

	VF_XYZ_UV_XYZ_NORMAL_FLOAT_32,	///< XYZ coordinates, UV coordinates, XYZ normal coordinates, 32-bit floating point each

	VF_UNKNOWN
};

class Vertex
{
public:

	/// Returns the bytes occupied by @format
	static size_t bytes_per_vertex(VertexFormat format);

	/// Returns the bits occupied by @format
	static size_t bits_per_vertex(VertexFormat format);

private:

	// Disable construction
	Vertex();
};

} // namespace crown
