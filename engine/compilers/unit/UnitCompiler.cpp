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

#include "Allocator.h"
#include "File.h"
#include "Filesystem.h"
#include "Hash.h"
#include "JSONParser.h"
#include "UnitCompiler.h"
#include "TempAllocator.h"
#include "Log.h"

namespace crown
{

//-----------------------------------------------------------------------------
UnitCompiler::UnitCompiler()
	: m_renderable(default_allocator())
{
}

//-----------------------------------------------------------------------------
size_t UnitCompiler::compile_impl(Filesystem& fs, const char* resource_path)
{
	File* file = fs.open(resource_path, FOM_READ);

	char file_buf[4096];
	file->read(file_buf, file->size());
	fs.close(file);

	JSONParser json(file_buf);
	JSONElement root = json.root();

	// Check for renderable
	if (root.has_key("renderable"))
	{
		Log::d("Reading renderables");

		JSONElement renderable_array = root.key("renderable");
		uint32_t renderable_array_size = renderable_array.size();

		for (uint32_t i = 0; i < renderable_array_size; i++)
		{
			DynamicString mesh_resource(default_allocator());
			mesh_resource += renderable_array[i].key("resource").string_value();
			mesh_resource += ".mesh";

			DynamicString mesh_name(default_allocator());
			mesh_name = renderable_array[i].key("name").string_value();

			UnitRenderable ur;
			ur.resource.id = hash::murmur2_64(mesh_resource.c_str(), string::strlen(mesh_resource.c_str()), 0);
			ur.name = hash::murmur2_32(mesh_name.c_str(), string::strlen(mesh_name.c_str()), 0);
			ur.visible = renderable_array[i].key("visible").bool_value();

			m_renderable.push_back(ur);
		}
	}

	return sizeof(UnitHeader) + m_renderable.size() * sizeof(UnitRenderable);
}

//-----------------------------------------------------------------------------
void UnitCompiler::write_impl(File* out_file)
{
	UnitHeader header;
	header.num_renderables = m_renderable.size();

	header.renderables_offset = sizeof(UnitHeader);

	out_file->write((char*) &header, sizeof(UnitHeader));

	if (m_renderable.size() > 0)
	{
		out_file->write((char*) m_renderable.begin(), sizeof(UnitRenderable) * header.num_renderables);
	}

	// Cleanup
	m_renderable.clear();
}

} // namespace crown
