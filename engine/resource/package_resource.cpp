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

#include "file.h"
#include "filesystem.h"
#include "json_parser.h"
#include "package_resource.h"
#include "temp_allocator.h"
#include "reader_writer.h"

namespace crown
{
namespace package_resource
{

//-----------------------------------------------------------------------------
void compile(Filesystem& fs, const char* resource_path, File* out_file)
{
	File* file = fs.open(resource_path, FOM_READ);
	JSONParser json(*file);
	fs.close(file);

	JSONElement root = json.root();
	BinaryWriter bw(*out_file);

	const uint32_t num_textures  = root.key("texture").size();
	const uint32_t num_scripts   = root.key("lua").size();
	const uint32_t num_sounds    = root.key("sound").size();
	const uint32_t num_meshes    = root.key("mesh").size();
	const uint32_t num_units     = root.key("unit").size();
	const uint32_t num_sprites   = root.key("sprite").size();
	const uint32_t num_physics   = root.key("physics").size();
	const uint32_t num_materials = root.key("material").size();
	const uint32_t num_fonts     = root.key("font").size();
	const uint32_t num_levels    = root.key("level").size();
	const uint32_t num_phyconfs  = root.key("physics_config").size();

	// Write header
	bw.write(num_textures);
	uint32_t offt = sizeof(PackageHeader);
	bw.write(offt);

	bw.write(num_scripts);
	offt += sizeof(ResourceId) * num_textures;
	bw.write(offt);

	bw.write(num_sounds);
	offt += sizeof(ResourceId) * num_scripts;
	bw.write(offt);

	bw.write(num_meshes);
	offt += sizeof(ResourceId) * num_sounds;
	bw.write(offt);

	bw.write(num_units);
	offt += sizeof(ResourceId) * num_meshes;
	bw.write(offt);

	bw.write(num_sprites);
	offt += sizeof(ResourceId) * num_units;
	bw.write(offt);

	bw.write(num_physics);
	offt += sizeof(ResourceId) * num_sprites;
	bw.write(offt);

	bw.write(num_materials);
	offt += sizeof(ResourceId) * num_physics;
	bw.write(offt);

	bw.write(num_fonts);
	offt += sizeof(ResourceId) * num_materials;
	bw.write(offt);

	bw.write(num_levels);
	offt += sizeof(ResourceId) * num_fonts;
	bw.write(offt);

	bw.write(num_phyconfs);
	offt += sizeof(ResourceId) * num_levels;
	bw.write(offt);

	// Write resource ids
	for (uint32_t i = 0; i < num_textures; i++)
		bw.write(root.key("texture")[i].to_resource_id("texture"));

	for (uint32_t i = 0; i < num_scripts; i++)
		bw.write(root.key("lua")[i].to_resource_id("lua"));

	for (uint32_t i = 0; i < num_sounds; i++)
		bw.write(root.key("sound")[i].to_resource_id("sound"));

	for (uint32_t i = 0; i < num_meshes; i++)
		bw.write(root.key("mesh")[i].to_resource_id("mesh"));

	for (uint32_t i = 0; i < num_units; i++)
		bw.write(root.key("unit")[i].to_resource_id("unit"));

	for (uint32_t i = 0; i < num_sprites; i++)
		bw.write(root.key("sprite")[i].to_resource_id("sprite"));

	for (uint32_t i = 0; i < num_physics; i++)
		bw.write(root.key("physics")[i].to_resource_id("physics"));

	for (uint32_t i = 0; i < num_materials; i++)
		bw.write(root.key("material")[i].to_resource_id("material"));

	for (uint32_t i = 0; i < num_fonts; i++)
		bw.write(root.key("font")[i].to_resource_id("font"));

	for (uint32_t i = 0; i < num_levels; i++)
		bw.write(root.key("level")[i].to_resource_id("level"));

	for (uint32_t i = 0; i < num_phyconfs; i++)
		bw.write(root.key("physics_config")[i].to_resource_id("physics_config"));
}

} // namespace package_resource
} // namespace crown
