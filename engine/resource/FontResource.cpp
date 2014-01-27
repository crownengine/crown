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

#include "FontResource.h"
#include "JSONParser.h"
#include "Allocator.h"
#include "Filesystem.h"
#include "Hash.h"
#include "Bundle.h"
#include "Types.h"


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
	JSONElement advance = e.key("advance");

	glyph.id = id.int_value();
	glyph.x = x.int_value();
	glyph.y = y.int_value();
	glyph.width = width.int_value();
	glyph.height = height.int_value();
	glyph.x_offset = x_offset.float_value();
	glyph.y_offset = y_offset.float_value();
	glyph.x_advance = advance.float_value();
}

//-----------------------------------------------------------------------------
void compile(Filesystem& fs, const char* resource_path, File* out_file)
{
	File* file = fs.open(resource_path, FOM_READ);
	char* buf = (char*)default_allocator().allocate(file->size());
	file->read(buf, file->size());

	// Out buffer
	FontHeader h;
	List<FontGlyphData> m_glyphs(default_allocator());

	JSONParser json(buf);
	JSONElement root = json.root();

	JSONElement mat = root.key("material");
	JSONElement count = root.key("count");
	JSONElement glyphs = root.key("glyphs");

	DynamicString material_name;
	mat.string_value(material_name);
	material_name += ".material";
	
	for (uint32_t i = 0; i < count.int_value(); i++)
	{
		FontGlyphData data;
		parse_glyph(glyphs[i], data);
		m_glyphs.push_back(data);
	}

	fs.close(file);
	default_allocator().deallocate(buf);

	h.material.id = hash::murmur2_64(material_name.c_str(), string::strlen(material_name.c_str()), 0);
	h.num_glyphs = m_glyphs.size();

	out_file->write((char*) &h, sizeof(FontHeader));

	if (m_glyphs.size() > 0)
	{
		out_file->write((char*) m_glyphs.begin(), sizeof(FontGlyphData) * h.num_glyphs);
	}
}

} // namespace font_resource
} // namespace crown