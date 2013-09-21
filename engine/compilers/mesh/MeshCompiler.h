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

#include "Compiler.h"
#include "MeshResource.h"
#include "Vec3.h"
#include "Vec2.h"
#include "List.h"

namespace crown
{

struct MeshVertex
{
	Vec3 position;
	Vec3 normal;
	Vec2 texcoord;

	bool operator==(const MeshVertex& other)
	{
		return position == other.position &&
				normal == other.normal &&
				texcoord == other.texcoord;
	}
};

class MeshCompiler : public Compiler
{
public:

						MeshCompiler();
						~MeshCompiler();

	size_t				compile_impl(Filesystem& fs, const char* resource_path);
	void				write_impl(File* out_file);

private:

	MeshHeader			m_mesh_header;
	bool				m_has_normal;
	bool				m_has_texcoord;

	List<MeshVertex>	m_vertices;
	List<uint16_t>		m_indices;
};

} // namespace crown
