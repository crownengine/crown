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

#include "font_resource.h"
#include "json_parser.h"
#include "allocator.h"
#include "filesystem.h"
#include "string_utils.h"
#include "bundle.h"
#include "types.h"


namespace crown
{
namespace font_resource
{

//-----------------------------------------------------------------------------
void parse_glyph(JSONElement e, FontGlyphData& glyph)
{
	JSONElement id = e.key("id");
	JSONElement x = e.key("x");
	JSONElement y = e.key("y");
	JSONElement width = e.key("width");
	JSONElement height = e.key("height");
	JSONElement x_offset = e.key("x_offset");
	JSONElement y_offset = e.key("y_offset");
	JSONElement x_advance = e.key("x_advance");

	glyph.id = id.to_int();
	glyph.x = x.to_int();
	glyph.y = y.to_int();
	glyph.width = width.to_int();
	glyph.height = height.to_int();
	glyph.x_offset = x_offset.to_float();
	glyph.y_offset = y_offset.to_float();
	glyph.x_advance = x_advance.to_float();
}

//-----------------------------------------------------------------------------
void compile(Filesystem& fs, const char* resource_path, File* out_file)
{
	File* file = fs.open(resource_path, FOM_READ);
	JSONParser json(*file);
	fs.close(file);

	// Out buffer
	FontHeader h;
	Array<FontGlyphData> m_glyphs(default_allocator());

	JSONElement root = json.root();

	JSONElement mat = root.key("material");
	JSONElement count = root.key("count");
	JSONElement size = root.key("size");
	JSONElement font_size = root.key("font_size");
	JSONElement glyphs = root.key("glyphs");

	uint32_t num_glyphs = count.to_int();

	for (uint32_t i = 0; i < num_glyphs; i++)
	{
		FontGlyphData data;
		parse_glyph(glyphs[i], data);
		array::push_back(m_glyphs, data);
	}

	h.material = mat.to_resource_id("material");
	h.num_glyphs = array::size(m_glyphs);
	h.texture_size = size.to_int();
	h.font_size = font_size.to_int();

	out_file->write((char*) &h, sizeof(FontHeader));

	if (array::size(m_glyphs) > 0)
	{
		out_file->write((char*) array::begin(m_glyphs), sizeof(FontGlyphData) * h.num_glyphs);
	}
}

} // namespace font_resource
} // namespace crown
