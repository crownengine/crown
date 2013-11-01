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

#include "Allocator.h"
#include "Filesystem.h"
#include "StringUtils.h"
#include "SpriteCompiler.h"

namespace crown
{

//-----------------------------------------------------------------------------
SpriteCompiler::SpriteCompiler()
	: m_anim_data(default_allocator())
{
}

//-----------------------------------------------------------------------------
SpriteCompiler::~SpriteCompiler()
{
}

//-----------------------------------------------------------------------------
size_t SpriteCompiler::compile_impl(Filesystem& fs, const char* resource_path)
{
	File* file = fs.open(resource_path, FOM_READ);
	char* buf = (char*)default_allocator().allocate(file->size());
	file->read(buf, file->size());

	JSONParser json(buf);
	JSONElement root = json.root();

	JSONElement anim_name = root.key("name");
	string::strncpy(m_anim_header.name, anim_name.string_value(), string::strlen(anim_name.string_value()) + 1);

	JSONElement anim_texture = root.key("texture");
	string::strncpy(m_anim_header.texture, anim_texture.string_value(), string::strlen(anim_texture.string_value()) + 1);

	JSONElement anim_length = root.key("length");
	m_anim_header.length = anim_length.int_value();

	JSONElement anim_frame_rate = root.key("frame_rate");
	m_anim_header.frame_rate = anim_frame_rate.int_value();

	JSONElement anim_playback_mode = root.key("playback_mode");
	m_anim_header.playback_mode = anim_playback_mode.int_value();

	Log::i("Playback mode: %d", m_anim_header.playback_mode);

	List<float> t_positions(default_allocator());
	JSONElement anim_vertices = root.key("positions");
	anim_vertices.array_value(t_positions);

	List<float> t_texcoords(default_allocator());
	JSONElement anim_texcoords = root.key("texcoords");
	anim_texcoords.array_value(t_texcoords);

	for (uint32_t i = 0; i < t_texcoords.size(); i+=8)
	{
		for (uint32_t j = 0; j < t_positions.size(); j+=2)
		{
			SpriteAnimationData t_animation_data;

			t_animation_data.position.x = t_positions[j];
			t_animation_data.position.y = t_positions[j+1];

			t_animation_data.texcoords.x = t_texcoords[j+i];
			t_animation_data.texcoords.y = t_texcoords[j+i+1];

			m_anim_data.push_back(t_animation_data);
		}
	}

	fs.close(file);

	return sizeof(SpriteHeader) + sizeof(SpriteAnimationData) * m_anim_data.size();
}

//-----------------------------------------------------------------------------
void SpriteCompiler::write_impl(File* out_file)
{
	out_file->write((char*)&m_anim_header, sizeof(SpriteHeader));

	for (uint32_t j = 0; j < m_anim_data.size(); j++)
	{
		out_file->write((char*)&m_anim_data[j], sizeof(SpriteAnimationData));
	}
}


} // namespace crown