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

#include "MaterialResource.h"
#include "DynamicString.h"
#include "StringUtils.h"
#include "StringUtils.h"
#include "JSONParser.h"
#include "Filesystem.h"

namespace crown
{
namespace material_resource
{

//-----------------------------------------------------------------------------
void compile(Filesystem& fs, const char* resource_path, File* out_file)
{
	File* file = fs.open(resource_path, FOM_READ);
	char* buf = (char*)default_allocator().allocate(file->size());
	file->read(buf, file->size());

	JSONParser json(buf);
	JSONElement root = json.root();

	// Read texture layers
	Array<ResourceId> texture_layers(default_allocator());

	JSONElement tl = root.key("texture_layers");
	for (uint32_t i = 0; i < tl.size(); i++)
	{
		DynamicString tex;
		tl[i].to_string(tex); tex += ".texture";

		array::push_back(texture_layers, ResourceId(tex.c_str()));
	}

	fs.close(file);
	default_allocator().deallocate(buf);

	// Write resource
	MaterialHeader mh;
	mh.num_texture_layers = array::size(texture_layers);

	uint32_t offt = sizeof(MaterialHeader);
	mh.texture_layers_offset = offt;

	out_file->write((char*) &mh, sizeof(MaterialHeader));

	if (mh.num_texture_layers)
	{
		out_file->write((char*) array::begin(texture_layers), sizeof(ResourceId) * array::size(texture_layers));
	}
}

} // namespace material_resource
} // namespace crown
