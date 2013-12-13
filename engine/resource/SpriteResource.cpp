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
#include "Hash.h"
#include "JSONParser.h"
#include "SpriteResource.h"
#include "StringUtils.h"

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
void parse_frame(JSONElement frame, List<StringId32>& names, List<FrameData>& regions)
{
	JSONElement name = frame.key("name");
	JSONElement region = frame.key("region");
	JSONElement offset = frame.key("offset");
	JSONElement scale = frame.key("scale");

	StringId32 name_hash = hash::murmur2_32(name.string_value(), name.size(), 0);
	FrameData fd;
	fd.x0 = region[0].float_value();
	fd.y0 = region[1].float_value();
	fd.x1 = region[2].float_value();
	fd.y1 = region[3].float_value();
	fd.offset_x = offset[0].float_value();
	fd.offset_y = offset[1].float_value();
	fd.scale_x = scale[0].float_value();
	fd.scale_y = scale[1].float_value();

	names.push_back(name_hash);
	regions.push_back(fd);
}

//-----------------------------------------------------------------------------
void compile(Filesystem& fs, const char* resource_path, File* out_file)
{
	File* file = fs.open(resource_path, FOM_READ);
	char* buf = (char*)default_allocator().allocate(file->size());
	file->read(buf, file->size());

	JSONParser json(buf);
	JSONElement root = json.root();

	List<StringId32>		m_names(default_allocator());
	List<FrameData> 		m_regions(default_allocator());
	List<float>				m_vertices(default_allocator());

	// Read frames
	JSONElement frames = root.key("frames");
	uint32_t num_frames = frames.size();
	for (uint32_t i = 0; i < num_frames; i++)
	{
		parse_frame(frames[i], m_names, m_regions);
	}

	for (uint32_t i = 0; i < num_frames; i++)
	{
		const FrameData& fd = m_regions[i];

		// Compute uv coords
		float u0 = fd.x0;
		float v0 = fd.y0;
		float u1 = fd.x0 + fd.x1;
		float v1 = fd.y0 + fd.y1;

		// Compute positions
		float w = fd.x1;
		float h = fd.y1;

		float x0 = fd.scale_x * (-w * 0.5) + fd.offset_x;
		float y0 = fd.scale_y * (-h * 0.5) + fd.offset_y;
		float x1 = fd.scale_x * ( w * 0.5) + fd.offset_x;
		float y1 = fd.scale_y * ( h * 0.5) + fd.offset_y;

		m_vertices.push_back(x0); m_vertices.push_back(y0); // position
		m_vertices.push_back(u0); m_vertices.push_back(v0); // uv

		m_vertices.push_back(x1); m_vertices.push_back(y0); // position
		m_vertices.push_back(u1); m_vertices.push_back(v0); // uv

		m_vertices.push_back(x1); m_vertices.push_back(y1); // position
		m_vertices.push_back(u1); m_vertices.push_back(v1); // uv

		m_vertices.push_back(x0); m_vertices.push_back(y1); // position
		m_vertices.push_back(u0); m_vertices.push_back(v1); // uv
	}

	fs.close(file);
	default_allocator().deallocate(buf);

	SpriteHeader h;
	h.texture.id = hash::murmur2_64("textures/circle.texture", string::strlen("textures/circle.texture"), 0);
	h.num_frames = m_names.size();

	uint32_t offt = sizeof(SpriteHeader);
	h.frame_names_offset    = offt; offt += sizeof(StringId32) * h.num_frames;
	h.frame_vertices_offset = offt; // offt += sizeof(float) * 16 * h.num_frames; <- not necessary, just for future reference

	out_file->write((char*) &h, sizeof(SpriteHeader));
	out_file->write((char*) m_names.begin(), sizeof(StringId32) * m_names.size());
	out_file->write((char*) m_vertices.begin(), sizeof(float) * 16 * m_vertices.size());
}

} // namespace sprite_resource
} // namespace crown