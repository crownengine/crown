/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "resource_manager.h"
#include "package_resource.h"

namespace crown
{

/// Collection of resources to load in a batch.
struct ResourcePackage
{
	ResourcePackage(StringId64 id, ResourceManager& resman)
		: _resman(&resman)
		, _id(id)
		, _package(NULL)
		, _has_loaded(false)
	{
		resman.load(PACKAGE_TYPE, _id);
		resman.flush();
		_package = (const PackageResource*) resman.get(PACKAGE_TYPE, _id);
	}

	~ResourcePackage()
	{
		_resman->unload(PACKAGE_TYPE, _id);
	}

	/// Loads all the resources in the package.
	/// @note
	/// The resources are not immediately available after the call is made,
	/// instead, you have to poll for completion with has_loaded()
	void load()
	{
		using namespace package_resource;

		for (uint32_t i = 0; i < num_textures(_package); i++)
		{
			_resman->load(TEXTURE_TYPE, get_texture_id(_package, i));
		}

		for (uint32_t i = 0; i < num_scripts(_package); i++)
		{
			_resman->load(LUA_TYPE, get_script_id(_package, i));
		}

		for (uint32_t i = 0; i < num_sounds(_package); i++)
		{
			_resman->load(SOUND_TYPE, get_sound_id(_package, i));
		}

		for (uint32_t i = 0; i < num_meshes(_package); i++)
		{
			_resman->load(MESH_TYPE, get_mesh_id(_package, i));
		}

		for (uint32_t i = 0; i < num_units(_package); i++)
		{
			_resman->load(UNIT_TYPE, get_unit_id(_package, i));
		}

		for (uint32_t i = 0; i < num_sprites(_package); i++)
		{
			_resman->load(SPRITE_TYPE, get_sprite_id(_package, i));
		}

		for (uint32_t i = 0; i < num_physics(_package); i++)
		{
			_resman->load(PHYSICS_TYPE, get_physics_id(_package, i));
		}

		for (uint32_t i = 0; i < num_materials(_package); i++)
		{
			_resman->load(MATERIAL_TYPE, get_material_id(_package, i));
		}

		for (uint32_t i = 0; i < num_fonts(_package); i++)
		{
			_resman->load(FONT_TYPE, get_font_id(_package, i));
		}

		for (uint32_t i = 0; i < num_levels(_package); i++)
		{
			_resman->load(LEVEL_TYPE, get_level_id(_package, i));
		}

		for (uint32_t i = 0; i < num_physics_configs(_package); i++)
		{
			_resman->load(PHYSICS_CONFIG_TYPE, get_physics_config_id(_package, i));
		}

		for (uint32_t i = 0; i < num_shaders(_package); i++)
		{
			_resman->load(SHADER_TYPE, get_shader_id(_package, i));
		}

		for (uint32_t i = 0; i < num_sprite_animations(_package); i++)
		{
			_resman->load(SPRITE_ANIMATION_TYPE, get_sprite_animation_id(_package, i));
		}
	}

	/// Unloads all the resources in the package.
	void unload()
	{
		using namespace package_resource;

		for (uint32_t i = 0; i < num_sprite_animations(_package); i++)
		{
			_resman->unload(SPRITE_ANIMATION_TYPE, get_sprite_animation_id(_package, i));
		}

		for (uint32_t i = 0; i < num_shaders(_package); i++)
		{
			_resman->unload(SHADER_TYPE, get_shader_id(_package, i));
		}

		for (uint32_t i = 0; i < num_physics_configs(_package); i++)
		{
			_resman->unload(PHYSICS_CONFIG_TYPE, get_physics_config_id(_package, i));
		}

		for (uint32_t i = 0; i < num_levels(_package); i++)
		{
			_resman->unload(LEVEL_TYPE, get_level_id(_package, i));
		}

		for (uint32_t i = 0; i < num_fonts(_package); i++)
		{
			_resman->unload(FONT_TYPE, get_font_id(_package, i));
		}

		for (uint32_t i = 0; i < num_materials(_package); i++)
		{
			_resman->unload(MATERIAL_TYPE, get_material_id(_package, i));
		}

		for (uint32_t i = 0; i < num_physics(_package); i++)
		{
			_resman->unload(PHYSICS_TYPE, get_physics_id(_package, i));
		}

		for (uint32_t i = 0; i < num_sprites(_package); i++)
		{
			_resman->unload(SPRITE_TYPE, get_sprite_id(_package, i));
		}

		for (uint32_t i = 0; i < num_units(_package); i++)
		{
			_resman->unload(UNIT_TYPE, get_unit_id(_package, i));
		}

		for (uint32_t i = 0; i < num_meshes(_package); i++)
		{
			_resman->unload(MESH_TYPE, get_mesh_id(_package, i));
		}

		for (uint32_t i = 0; i < num_sounds(_package); i++)
		{
			_resman->unload(SOUND_TYPE, get_sound_id(_package, i));
		}

		for (uint32_t i = 0; i < num_scripts(_package); i++)
		{
			_resman->unload(LUA_TYPE, get_script_id(_package, i));
		}

		for (uint32_t i = 0; i < num_textures(_package); i++)
		{
			_resman->unload(TEXTURE_TYPE, get_texture_id(_package, i));
		}
	}

	/// Waits until the package has been loaded.
	void flush()
	{
		_resman->flush();
		_has_loaded = true;
	}

	/// Returns whether the package has been loaded.
	bool has_loaded() const
	{
		return _has_loaded;
	}

private:

	ResourceManager* _resman;
	StringId64 _id;
	const PackageResource* _package;
	bool _has_loaded;
};

} // namespace crown
