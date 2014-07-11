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
#include "ReaderWriter.h"
#include <cfloat>

namespace crown
{
namespace sprite_resource
{

struct SpriteFrame
{
	StringId32 name;
	Vector4 region;		// [x0, y0, x1, y1]
	Vector2 scale;		// [Sx, Sy]
	Vector2 offset;		// [Ox, Oy]
};

//-----------------------------------------------------------------------------
void parse_frame(JSONElement e, SpriteFrame& frame)
{
	frame.name   = e.key("name"  ).to_string_id();
	frame.region = e.key("region").to_vector4();
	frame.offset = e.key("offset").to_vector2();
	frame.scale  = e.key("scale" ).to_vector2();
}

//-----------------------------------------------------------------------------
void parse_animation(JSONElement e, SpriteAnimation& anim)
{
	anim.name = e.key("name").to_string_id();
	anim.time = e.key("time").to_float();
	anim.num_frames = 0;
	anim.start_frame = 0;
}

//-----------------------------------------------------------------------------
void compile(Filesystem& fs, const char* resource_path, File* out_file)
{
	File* file = fs.open(resource_path, FOM_READ);
	JSONParser json(*file);
	fs.close(file);

	JSONElement root = json.root();

	// Read width/height
	const float width  = root.key("width" ).to_float();
	const float height = root.key("height").to_float();
	const uint32_t num_frames = root.key("frames").size();
	const uint32_t num_animations = root.key("animations").size();

	BinaryWriter bw(*out_file);

	Array<float> vertices(default_allocator());
	Array<uint16_t> indices(default_allocator());
	uint32_t num_idx = 0;
	for (uint32_t i = 0; i < num_frames; i++)
	{
		JSONElement e(root.key("frames")[i]);

		SpriteFrame frame;
		parse_frame(e, frame);

		const SpriteFrame& fd = frame;

		// Compute uv coords
		const float u0 = fd.region.x / width;
		const float v0 = fd.region.y / height;
		const float u1 = (fd.region.x + fd.region.z) / width;
		const float v1 = (fd.region.y + fd.region.w) / height;

		// Compute positions
		const float w = fd.region.z / CE_PIXELS_PER_METER;
		const float h = fd.region.w / CE_PIXELS_PER_METER;

		const float x0 = fd.scale.x * (-w * 0.5) + fd.offset.x;
		const float y0 = fd.scale.y * (-h * 0.5) + fd.offset.y;
		const float x1 = fd.scale.x * ( w * 0.5) + fd.offset.x;
		const float y1 = fd.scale.y * ( h * 0.5) + fd.offset.y;

		array::push_back(vertices, x0); array::push_back(vertices, y0); // position
		array::push_back(vertices, u0); array::push_back(vertices, v0); // uv

		array::push_back(vertices, x1); array::push_back(vertices, y0); // position
		array::push_back(vertices, u1); array::push_back(vertices, v0); // uv

		array::push_back(vertices, x1); array::push_back(vertices, y1); // position
		array::push_back(vertices, u1); array::push_back(vertices, v1); // uv

		array::push_back(vertices, x0); array::push_back(vertices, y1); // position
		array::push_back(vertices, u0); array::push_back(vertices, v1); // uv

		array::push_back(indices, uint16_t(num_idx)); array::push_back(indices, uint16_t(num_idx + 1)); array::push_back(indices, uint16_t(num_idx + 2));
		array::push_back(indices, uint16_t(num_idx)); array::push_back(indices, uint16_t(num_idx + 2)); array::push_back(indices, uint16_t(num_idx + 3));
		num_idx += 4;
	}

	const uint32_t num_vertices = array::size(vertices) / 4; // 4 components per vertex
	const uint32_t num_indices = array::size(indices);

	// Write animations
	Array<SpriteAnimation> animations(default_allocator());
	Array<uint32_t> frames(default_allocator());

	for (uint32_t i = 0; i < num_animations; i++)
	{
		JSONElement e(root.key("animations")[i]);

		SpriteAnimation anim;
		parse_animation(e, anim);

		// Read frames
		const uint32_t num_frames = e.key("frames").size();

		anim.num_frames = num_frames;
		anim.start_frame = array::size(frames); // Relative offset

		for (uint32_t ff = 0; ff < num_frames; ff++)
			array::push_back(frames, (uint32_t) e.key("frames")[ff].to_int());

		array::push_back(animations, anim);
	}

	// Write header
	bw.write(uint32_t(0)); // vb
	bw.write(uint32_t(0)); // ib
	bw.write(num_animations); uint32_t offt = sizeof(SpriteHeader);
	bw.write(offt);
	bw.write(num_vertices); offt += sizeof(SpriteAnimation) * array::size(animations) + sizeof(uint32_t) * array::size(frames);
	bw.write(offt);
	bw.write(num_indices); offt += sizeof(float) * array::size(vertices);
	bw.write(offt);

	if (array::size(animations))
		bw.write(array::begin(animations), sizeof(SpriteAnimation) * array::size(animations));

	if (array::size(frames))
		bw.write(array::begin(frames), sizeof(uint32_t) * array::size(frames));

	if (array::size(vertices))
		bw.write(array::begin(vertices), sizeof(float) * array::size(vertices));

	if (array::size(indices))
		bw.write(array::begin(indices), sizeof(uint16_t) * array::size(indices));
}

} // namespace sprite_resource
} // namespace crown
