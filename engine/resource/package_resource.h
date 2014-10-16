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

#pragma once

#include "types.h"
#include "resource.h"
#include "bundle.h"
#include "allocator.h"
#include "file.h"

namespace crown
{

struct PackageResource
{
	uint32_t version;
	uint32_t num_textures;
	uint32_t textures_offset;
	uint32_t num_scripts;
	uint32_t scripts_offset;
	uint32_t num_sounds;
	uint32_t sounds_offset;
	uint32_t num_meshes;
	uint32_t meshes_offset;
	uint32_t num_units;
	uint32_t units_offset;
	uint32_t num_sprites;
	uint32_t sprites_offset;
	uint32_t num_physics;
	uint32_t physics_offset;
	uint32_t num_materials;
	uint32_t materials_offset;
	uint32_t num_fonts;
	uint32_t fonts_offset;
	uint32_t num_levels;
	uint32_t levels_offset;
	uint32_t num_physics_configs;
	uint32_t physics_configs_offset;
	uint32_t num_shaders;
	uint32_t shaders_offset;
	uint32_t num_sprite_animations;
	uint32_t sprite_animations_offset;
};

namespace package_resource
{
	void compile(const char* path, CompileOptions& opts);
	void* load(Allocator& allocator, Bundle& bundle, ResourceId id);
	void online(StringId64 /*id*/, ResourceManager& /*rm*/);
	void offline(StringId64 /*id*/, ResourceManager& /*rm*/);
	void unload(Allocator& allocator, void* resource);
	
	uint32_t num_textures(const PackageResource* pr);
	uint32_t num_scripts(const PackageResource* pr);
	uint32_t num_sounds(const PackageResource* pr);
	uint32_t num_meshes(const PackageResource* pr);
	uint32_t num_units(const PackageResource* pr);
	uint32_t num_sprites(const PackageResource* pr);
	uint32_t num_physics(const PackageResource* pr);
	uint32_t num_materials(const PackageResource* pr);
	uint32_t num_fonts(const PackageResource* pr);
	uint32_t num_levels(const PackageResource* pr);
	uint32_t num_physics_configs(const PackageResource* pr);
	uint32_t num_shaders(const PackageResource* pr);
	uint32_t num_sprite_animations(const PackageResource* pr);
	StringId64 get_texture_id(const PackageResource* pr, uint32_t i);
	StringId64 get_script_id(const PackageResource* pr, uint32_t i);
	StringId64 get_sound_id(const PackageResource* pr, uint32_t i);
	StringId64 get_mesh_id(const PackageResource* pr, uint32_t i);
	StringId64 get_unit_id(const PackageResource* pr, uint32_t i);
	StringId64 get_sprite_id(const PackageResource* pr, uint32_t i);
	StringId64 get_physics_id(const PackageResource* pr, uint32_t i);
	StringId64 get_material_id(const PackageResource* pr, uint32_t i);
	StringId64 get_font_id(const PackageResource* pr, uint32_t i);
	StringId64 get_level_id(const PackageResource* pr, uint32_t i);
	StringId64 get_physics_config_id(const PackageResource* pr, uint32_t i);
	StringId64 get_shader_id(const PackageResource* pr, uint32_t i);
	StringId64 get_sprite_animation_id(const PackageResource* pr, uint32_t i);
} // namespace package_resource
} // namespace crown
