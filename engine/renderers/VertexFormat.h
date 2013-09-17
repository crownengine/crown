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

#include "RendererTypes.h"

namespace crown
{

struct VertexFormatInfo
{
	bool has_attrib(ShaderAttrib attrib) const
	{
		return sizes[attrib] != 0;
	}

	/// Returns the number of components per @a attrib
	size_t num_components(ShaderAttrib attrib) const
	{
		return (size_t) sizes[attrib];
	}

	/// Returns the byte offset between consecutive vertex @a attrib
	size_t attrib_stride(ShaderAttrib /*attrib*/) const
	{
		size_t stride = 0;
		for (uint8_t i = 0; i < ATTRIB_COUNT; i++)
		{
			stride += sizes[i];
		}

		return stride * sizeof(float);
	}

	/// Returns the byte offset of the first @a attrib in the format
	size_t attrib_offset(ShaderAttrib attrib) const
	{
		size_t offset = 0;
		for (uint8_t i = 0; i < attrib; i++)
		{
			offset += sizes[i];
		}

		return offset * sizeof(float);
	}

public:

	uint8_t sizes[ATTRIB_COUNT];
};

// VertexFormat to VertexFormatInfo
const VertexFormatInfo VERTEX_FORMAT_INFO[VERTEX_COUNT] =
{
	{ 2, 0, 0, 0, 0, 0, 0 },
	{ 2, 3, 0, 0, 0, 0, 0 },
	{ 2, 0, 4, 0, 0, 0, 0 },
	{ 2, 0, 0, 2, 0, 0, 0 },
	{ 2, 3, 4, 0, 0, 0, 0 },
	{ 2, 3, 4, 2, 0, 0, 0 },

	{ 3, 0, 0, 0, 0, 0, 0 },
	{ 3, 3, 0, 0, 0, 0, 0 },
	{ 3, 0, 4, 0, 0, 0, 0 },
	{ 3, 0, 0, 2, 0, 0, 0 },
	{ 3, 3, 4, 0, 0, 0, 0 },
	{ 3, 3, 0, 2, 0, 0, 0 },
	{ 3, 3, 4, 2, 0, 0, 0 }
};

class Vertex
{
public:

	/// Returns the bytes occupied by @a format
	static size_t bytes_per_vertex(VertexFormat format)
	{
		const VertexFormatInfo& info = VERTEX_FORMAT_INFO[format];
		
		size_t size = 0;
		for (uint8_t i = 0; i < ATTRIB_COUNT; i++)
		{
			size += info.sizes[i];
		}

		// Components are always float
		return size * sizeof(float);
	}

	/// Returns the bits occupied by @a format
	static size_t bits_per_vertex(VertexFormat format)
	{
		return bytes_per_vertex(format) * 8;
	}

	static const VertexFormatInfo& info(VertexFormat format)
	{
		return VERTEX_FORMAT_INFO[format];
	}

private:

	// Disable construction
	Vertex();
};

} // namespace crown
