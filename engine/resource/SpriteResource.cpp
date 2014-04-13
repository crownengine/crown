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

#include <cstring>
#include <inttypes.h>

#include "Allocator.h"
#include "Filesystem.h"
#include "StringUtils.h"
#include "JSONParser.h"
#include "SpriteResource.h"
#include "StringUtils.h"
#include "Array.h"
#include "Config.h"

namespace crown
{
namespace sprite_resource
{

//-----------------------------------------------------------------------------
struct FrameData
{
	float x0, y0;
	float x1, y1;

	float scale_x, scale_y;
	float offset_x, offset_y;
};

//-----------------------------------------------------------------------------
void parse_frame(JSONElement frame, Array<StringId32>& names, Array<FrameData>& regions)
{
	JSONElement name = frame.key("name");
	JSONElement region = frame.key("region");
	JSONElement offset = frame.key("offset");
	JSONElement scale = frame.key("scale");

	DynamicString frame_name;
	name.to_string(frame_name);

	StringId32 name_hash = string::murmur2_32(frame_name.c_str(), frame_name.length(), 0);
	FrameData fd;
	fd.x0 = region[0].to_float();
	fd.y0 = region[1].to_float();
	fd.x1 = region[2].to_float();
	fd.y1 = region[3].to_float();
	fd.offset_x = offset[0].to_float();
	fd.offset_y = offset[1].to_float();
	fd.scale_x = scale[0].to_float();
	fd.scale_y = scale[1].to_float();

	array::push_back(names, name_hash);
	array::push_back(regions, fd);
}

//-----------------------------------------------------------------------------
void compile(Filesystem& fs, const char* resource_path, File* out_file)
{
	File* file = fs.open(resource_path, FOM_READ);
	char* buf = (char*)default_allocator().allocate(file->size());
	file->read(buf, file->size());

	JSONParser json(buf);
	JSONElement root = json.root();

	float width;
	float height;
	Array<StringId32>		m_names(default_allocator());
	Array<FrameData> 		m_regions(default_allocator());
	Array<float>			m_vertices(default_allocator());
	Array<uint16_t>			m_indices(default_allocator());

	// Read width/height
	width = root.key("width").to_float();
	height = root.key("height").to_float();

	// Read frames
	JSONElement frames = root.key("frames");
	uint32_t num_frames = frames.size();
	for (uint32_t i = 0; i < num_frames; i++)
	{
		parse_frame(frames[i], m_names, m_regions);
	}

	uint32_t num_idx = 0;
	for (uint32_t i = 0; i < num_frames; i++)
	{
		const FrameData& fd = m_regions[i];

		// Compute uv coords
		const float u0 = fd.x0;
		const float v0 = fd.y0;
		const float u1 = fd.x0 + fd.x1;
		const float v1 = fd.y0 + fd.y1;

		// Compute positions
		const float w = (fd.x1 - fd.x0) * (width / CE_PIXELS_PER_METER);
		const float h = (fd.y1 - fd.y0) * (height / CE_PIXELS_PER_METER);

		const float x0 = fd.scale_x * (-w * 0.5) + fd.offset_x;
		const float y0 = fd.scale_y * (-h * 0.5) + fd.offset_y;
		const float x1 = fd.scale_x * ( w * 0.5) + fd.offset_x;
		const float y1 = fd.scale_y * ( h * 0.5) + fd.offset_y;

		array::push_back(m_vertices, x0); array::push_back(m_vertices, y0); // position
		array::push_back(m_vertices, u0); array::push_back(m_vertices, v0); // uv

		array::push_back(m_vertices, x1); array::push_back(m_vertices, y0); // position
		array::push_back(m_vertices, u1); array::push_back(m_vertices, v0); // uv

		array::push_back(m_vertices, x1); array::push_back(m_vertices, y1); // position
		array::push_back(m_vertices, u1); array::push_back(m_vertices, v1); // uv

		array::push_back(m_vertices, x0); array::push_back(m_vertices, y1); // position
		array::push_back(m_vertices, u0); array::push_back(m_vertices, v1); // uv

		array::push_back(m_indices, uint16_t(num_idx)); array::push_back(m_indices, uint16_t(num_idx + 1)); array::push_back(m_indices, uint16_t(num_idx + 2));
		array::push_back(m_indices, uint16_t(num_idx)); array::push_back(m_indices, uint16_t(num_idx + 2)); array::push_back(m_indices, uint16_t(num_idx + 3));
		num_idx += 4;
	}

	fs.close(file);
	default_allocator().deallocate(buf);

	SpriteHeader h;
	h.num_frames = array::size(m_names);
	h.num_vertices = array::size(m_vertices) / 4; // 4 components per vertex
	h.num_indices = array::size(m_indices);

	uint32_t offt = sizeof(SpriteHeader);
	/*h.frame_names_offset    = offt*/; offt += sizeof(StringId32) * h.num_frames;
	h.vertices_offset = offt; offt += sizeof(float) * array::size(m_vertices);
	h.indices_offset = offt; offt += sizeof(uint16_t) * array::size(m_indices);

	out_file->write((char*) &h, sizeof(SpriteHeader));
	out_file->write((char*) array::begin(m_names), sizeof(StringId32) * array::size(m_names));
	out_file->write((char*) array::begin(m_vertices), sizeof(float) * array::size(m_vertices));
	out_file->write((char*) array::begin(m_indices), sizeof(uint16_t) * array::size(m_indices));
}

} // namespace sprite_resource
} // namespace crown