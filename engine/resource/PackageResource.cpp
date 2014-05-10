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
#include "StringUtils.h"
#include "JSONParser.h"
#include "Log.h"
#include "PackageResource.h"
#include "TempAllocator.h"

namespace crown
{
namespace package_resource
{

//-----------------------------------------------------------------------------
void parse_resources(JSONElement arr, const char* type, Array<ResourceId>& out)
{
	const uint32_t num = arr.size();

	for (uint32_t i = 0; i < num; i++)
	{
		DynamicString name;
		arr[i].to_string(name);
		array::push_back(out, ResourceId(type, name.c_str()));
	}
}

//-----------------------------------------------------------------------------
void compile(Filesystem& fs, const char* resource_path, File* out_file)
{
	File* file = fs.open(resource_path, FOM_READ);
	JSONParser json(*file);
	fs.close(file);

	JSONElement root = json.root();

	Array<ResourceId> textures(default_allocator());
	Array<ResourceId> scripts(default_allocator());
	Array<ResourceId> sounds(default_allocator());
	Array<ResourceId> meshes(default_allocator());
	Array<ResourceId> units(default_allocator());
	Array<ResourceId> sprites(default_allocator());
	Array<ResourceId> physics(default_allocator());
	Array<ResourceId> materials(default_allocator());
	Array<ResourceId> guis(default_allocator());
	Array<ResourceId> fonts(default_allocator());
	Array<ResourceId> levels(default_allocator());

	if (root.has_key("texture"))
		parse_resources(root.key("texture"), "texture", textures);

	if (root.has_key("lua"))
		parse_resources(root.key("lua"), "lua", scripts);

	if (root.has_key("sound"))
		parse_resources(root.key("sound"), "sound", sounds);

	if (root.has_key("mesh"))
		parse_resources(root.key("mesh"), "mesh", meshes);

	if (root.has_key("unit"))
		parse_resources(root.key("unit"), "unit", units);

	if (root.has_key("sprite"))
		parse_resources(root.key("sprite"), "sprite", sprites);

	if (root.has_key("physics"))
		parse_resources(root.key("physics"), "physics", physics);

	if (root.has_key("material"))
		parse_resources(root.key("material"), "material", materials);

	if (root.has_key("gui"))
		parse_resources(root.key("gui"), "gui", guis);

	if (root.has_key("font"))
		parse_resources(root.key("font"), "font", fonts);

	if (root.has_key("level"))
		parse_resources(root.key("level"), "level", levels);

	PackageHeader header;
	header.num_textures = array::size(textures);
	header.num_scripts = array::size(scripts);
	header.num_sounds = array::size(sounds);
	header.num_meshes = array::size(meshes);
	header.num_units = array::size(units);
	header.num_sprites = array::size(sprites);
	header.num_physics = array::size(physics);
	header.num_materials = array::size(materials);
	header.num_guis = array::size(guis);
	header.num_fonts = array::size(fonts);
	header.num_levels = array::size(levels);

	header.textures_offset = sizeof(PackageHeader);
	header.scripts_offset  = header.textures_offset + sizeof(ResourceId) * header.num_textures;
	header.sounds_offset = header.scripts_offset + sizeof(ResourceId) * header.num_scripts;
	header.meshes_offset = header.sounds_offset + sizeof(ResourceId) * header.num_sounds;
	header.units_offset = header.meshes_offset + sizeof(ResourceId) * header.num_meshes;
	header.sprites_offset = header.units_offset + sizeof(ResourceId) * header.num_units;
	header.physics_offset = header.sprites_offset + sizeof(ResourceId) * header.num_sprites;
	header.materials_offset = header.physics_offset + sizeof(ResourceId) * header.num_physics;
	header.guis_offset = header.materials_offset + sizeof(ResourceId) * header.num_materials;
	header.fonts_offset = header.guis_offset + sizeof(ResourceId) * header.num_guis;
	header.levels_offset = header.fonts_offset + sizeof(ResourceId) * header.num_fonts;

	out_file->write((char*) &header, sizeof(PackageHeader));

	if (array::size(textures) > 0)
	{
		out_file->write((char*) array::begin(textures), sizeof(ResourceId) * header.num_textures);		
	}
	if (array::size(scripts) > 0)
	{
		out_file->write((char*) array::begin(scripts), sizeof(ResourceId) * header.num_scripts);
	}
	if (array::size(sounds) > 0)
	{
		out_file->write((char*) array::begin(sounds), sizeof(ResourceId) * header.num_sounds);
	}
	if (array::size(meshes) > 0)
	{
		out_file->write((char*) array::begin(meshes), sizeof(ResourceId) * header.num_meshes);
	}
	if (array::size(units) > 0)
	{
		out_file->write((char*) array::begin(units), sizeof(ResourceId) * header.num_units);	
	}
	if (array::size(sprites) > 0)
	{
		out_file->write((char*) array::begin(sprites), sizeof(ResourceId) * header.num_sprites);
	}
	if (array::size(physics) > 0)
	{
		out_file->write((char*) array::begin(physics), sizeof(ResourceId) * header.num_physics);
	}
	if (array::size(materials) > 0)
	{
		out_file->write((char*) array::begin(materials), sizeof(ResourceId) * header.num_materials);
	}
	if (array::size(guis) > 0)
	{
		out_file->write((char*) array::begin(guis), sizeof(ResourceId) * header.num_guis);
	}
	if (array::size(fonts) > 0)
	{
		out_file->write((char*) array::begin(fonts), sizeof(ResourceId) * header.num_fonts);
	}
	if (array::size(levels) > 0)
	{
		out_file->write((char*) array::begin(levels), sizeof(ResourceId) * header.num_levels);
	}
}

} // namespace package_resource
} // namespace crown
