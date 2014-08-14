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

#include "material_resource.h"
#include "dynamic_string.h"
#include "string_utils.h"
#include "string_utils.h"
#include "json_parser.h"
#include "filesystem.h"
#include "reader_writer.h"
#include "os.h"

namespace crown
{
namespace material_resource
{

struct Data
{
	Array<TextureData> textures;
	Array<UniformData> uniforms;
	Array<char> dynamic;

	Data()
		: textures(default_allocator())
		, uniforms(default_allocator())
		, dynamic(default_allocator())
	{}
};

// Returns offset to start of data
template <typename T>
static uint32_t reserve_dynamic_data(T data, Array<char>& dynamic)
{
	uint32_t offt = array::size(dynamic);
	array::push(dynamic, (char*) &data, sizeof(data));
	return offt;
}

static void parse_textures(JSONElement root, Array<TextureData>& textures, Array<char>& dynamic)
{
	using namespace vector;

	Vector<DynamicString> keys(default_allocator());
	root.key("textures").to_keys(keys);

	for (uint32_t i = 0; i < size(keys); i++)
	{
		TextureHandle th;
		th.sampler_handle = 0;
		th.texture_handle = 0;

		ResourceId texid = root.key("textures").key(keys[i].c_str()).to_resource_id("texture");

		TextureData td;
		td.id = texid.name;
		td.data_offset = reserve_dynamic_data(th, dynamic);
		strncpy(td.sampler_name, keys[i].c_str(), 256);

		array::push_back(textures, td);
	}
}

struct UniformTypeInfo
{
	const char* name;
	UniformType::Enum type;
	uint8_t size;
};

static const UniformTypeInfo s_uniform_type_info[UniformType::COUNT] =
{
	{ "integer", UniformType::INTEGER,  4 },
	{ "float",   UniformType::FLOAT,    4 },
	{ "vector2", UniformType::VECTOR3,  8 },
	{ "vector3", UniformType::VECTOR3, 12 },
	{ "vector4", UniformType::VECTOR4, 16 }
};

static UniformType::Enum string_to_uniform_type(const char* str)
{
	for (uint32_t i = 0; i < UniformType::COUNT; i++)
	{
		if (string::strcmp(str, s_uniform_type_info[i].name) == 0)
			return s_uniform_type_info[i].type;
	}

	CE_FATAL("Unknown uniform type");
}

static void parse_uniforms(JSONElement root, Array<UniformData>& uniforms, Array<char>& dynamic)
{
	using namespace vector;

	Vector<DynamicString> keys(default_allocator());
	root.key("uniforms").to_keys(keys);

	for (uint32_t i = 0; i < size(keys); i++)
	{
		UniformHandle uh;
		uh.uniform_handle = 0;

		DynamicString type;
		root.key("uniforms").key(keys[i].c_str()).key("type").to_string(type);

		UniformData ud;
		ud.type = string_to_uniform_type(type.c_str());
		ud.data_offset = reserve_dynamic_data(uh, dynamic);

		strncpy(ud.name, keys[i].c_str(), 256);

		switch (ud.type)
		{
			case UniformType::INTEGER:
			{
				int32_t data = root.key("uniforms").key(keys[i].c_str()).key("value").to_int();
				reserve_dynamic_data(data, dynamic);
				break;
			}
			case UniformType::FLOAT:
			{
				float data = root.key("uniforms").key(keys[i].c_str()).key("value").to_int();
				reserve_dynamic_data(data, dynamic);
				break;
			}
			case UniformType::VECTOR2:
			{
				Vector2 data = root.key("uniforms").key(keys[i].c_str()).key("value").to_vector2();
				reserve_dynamic_data(data, dynamic);
				break;
			}
			case UniformType::VECTOR3:
			{
				Vector3 data = root.key("uniforms").key(keys[i].c_str()).key("value").to_vector3();
				reserve_dynamic_data(data, dynamic);
				break;
			}
			case UniformType::VECTOR4:
			{
				Vector4 data = root.key("uniforms").key(keys[i].c_str()).key("value").to_vector4();
				reserve_dynamic_data(data, dynamic);
				break;
			}
			default: CE_FATAL("Oops"); break;
		}

		array::push_back(uniforms, ud);
	}
}

//-----------------------------------------------------------------------------
void compile(Filesystem& fs, const char* resource_path, File* out_file)
{
	File* file = fs.open(resource_path, FOM_READ);
	JSONParser json(*file);
	fs.close(file);

	JSONElement root = json.root();

	Array<TextureData> texdata(default_allocator());
	Array<UniformData> unidata(default_allocator());
	Array<char> dynblob(default_allocator());

	parse_textures(root, texdata, dynblob);
	parse_uniforms(root, unidata, dynblob);

	MaterialHeader mh;
	mh.version = MATERIAL_VERSION;
	mh.num_textures = array::size(texdata);
	mh.texture_data_offset = sizeof(mh);
	mh.num_uniforms = array::size(unidata);
	mh.uniform_data_offset = sizeof(mh) + sizeof(TextureData) * array::size(texdata);
	mh.dynamic_data_size = array::size(dynblob);
	mh.dynamic_data_offset = sizeof(mh) + sizeof(TextureData) * array::size(texdata) + sizeof(UniformData) * array::size(unidata);

	out_file->write((char*) &mh, sizeof(mh));
	out_file->write((char*) array::begin(texdata), sizeof(TextureData) * array::size(texdata));
	out_file->write((char*) array::begin(unidata), sizeof(UniformData) * array::size(unidata));
	out_file->write((char*) array::begin(dynblob), sizeof(char) * array::size(dynblob));
}

} // namespace material_resource
} // namespace crown
