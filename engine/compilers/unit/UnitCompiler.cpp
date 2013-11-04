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
	, m_camera(default_allocator())
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
		JSONElement renderable_array = root.key("renderable");
		uint32_t renderable_array_size = renderable_array.size();

		for (uint32_t i = 0; i < renderable_array_size; i++)
		{
			const char* type = renderable_array[i].key("type").string_value();
						
			UnitRenderable ur;
			DynamicString renderable;

			if (string::strcmp(type, "mesh") == 0)
			{
				ur.type = UnitRenderable::MESH;
				renderable += renderable_array[i].key("resource").string_value();
				renderable += ".mesh";
			}
			else if (string::strcmp(type, "sprite") == 0)
			{
				ur.type = UnitRenderable::SPRITE;
				renderable += renderable_array[i].key("resource").string_value();
				renderable += ".sprite";
			}
			else
			{
				CE_ASSERT(false, "Oops, unknown renderable type: '%s'", type);
			}

			DynamicString renderable_name;
			renderable_name = renderable_array[i].key("name").string_value();

			Log::d("Renderable %s", renderable.c_str());

			ur.resource.id = hash::murmur2_64(renderable.c_str(), string::strlen(renderable.c_str()), 0);
			ur.name = hash::murmur2_32(renderable_name.c_str(), string::strlen(renderable_name.c_str()), 0);
			ur.visible = renderable_array[i].key("visible").bool_value();

			m_renderable.push_back(ur);
		}
	}

	// Check for cameras
	if (root.has_key("camera"))
	{
		JSONElement camera = root.key("camera");
		uint32_t num_cameras = camera.size();

		for (uint32_t i = 0; i < num_cameras; i++)
		{
			JSONElement camera_name = camera[i].key("name");

			UnitCamera uc;
			uc.name = hash::murmur2_32(camera_name.string_value(), camera_name.size(), 0);

			m_camera.push_back(uc);
		}
	}

	return sizeof(UnitHeader) +
			m_renderable.size() * sizeof(UnitRenderable) +
			m_camera.size() * sizeof(UnitCamera);
}

//-----------------------------------------------------------------------------
void UnitCompiler::write_impl(File* out_file)
{
	UnitHeader header;
	header.num_renderables = m_renderable.size();
	header.num_cameras = m_camera.size();

	header.renderables_offset = sizeof(UnitHeader);
	header.cameras_offset = sizeof(UnitHeader) + sizeof(UnitRenderable) * header.num_renderables;

	out_file->write((char*) &header, sizeof(UnitHeader));

	if (m_renderable.size() > 0)
	{
		out_file->write((char*) m_renderable.begin(), sizeof(UnitRenderable) * header.num_renderables);
	}

	if (m_camera.size() > 0)
	{
		out_file->write((char*) m_camera.begin(), sizeof(UnitCamera) * header.num_cameras);
	}

	// Cleanup
	m_renderable.clear();
	m_camera.clear();
}

} // namespace crown
