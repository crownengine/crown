/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "memory_types.h"
#include "resource_types.h"
#include "filesystem_types.h"
#include "compiler_types.h"

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
	void* load(File& file, Allocator& a);
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
