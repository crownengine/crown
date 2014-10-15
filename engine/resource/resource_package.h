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
#include "resource_manager.h"
#include "package_resource.h"
#include "resource.h"
#include "log.h"

namespace crown
{

/// Collection of resources to load in a batch.
struct ResourcePackage
{
	ResourcePackage(StringId64 id, ResourceManager& resman)
		: m_resource_manager(&resman)
		, m_package(NULL)
		, m_has_loaded(false)
	{
		m_id.type = PACKAGE_TYPE;
		m_id.name = id;
		resman.load(m_id);
		resman.flush();
		m_package = (const PackageResource*) resman.get(m_id);
	}

	~ResourcePackage()
	{
		m_resource_manager->unload(m_id);
	}

	/// Loads all the resources in the package.
	/// @note
	/// The resources are not immediately available after the call is made,
	/// instead, you have to poll for completion with has_loaded()
	void load()
	{
		using namespace package_resource;

		for (uint32_t i = 0; i < num_textures(m_package); i++)
		{
			m_resource_manager->load(get_texture_id(m_package, i));
		}

		for (uint32_t i = 0; i < num_scripts(m_package); i++)
		{
			m_resource_manager->load(get_script_id(m_package, i));
		}

		for (uint32_t i = 0; i < num_sounds(m_package); i++)
		{
			m_resource_manager->load(get_sound_id(m_package, i));
		}

		for (uint32_t i = 0; i < num_meshes(m_package); i++)
		{
			m_resource_manager->load(get_mesh_id(m_package, i));
		}

		for (uint32_t i = 0; i < num_units(m_package); i++)
		{
			m_resource_manager->load(get_unit_id(m_package, i));
		}

		for (uint32_t i = 0; i < num_sprites(m_package); i++)
		{
			m_resource_manager->load(get_sprite_id(m_package, i));
		}

		for (uint32_t i = 0; i < num_physics(m_package); i++)
		{
			m_resource_manager->load(get_physics_id(m_package, i));
		}

		for (uint32_t i = 0; i < num_materials(m_package); i++)
		{
			m_resource_manager->load(get_material_id(m_package, i));
		}

		for (uint32_t i = 0; i < num_fonts(m_package); i++)
		{
			m_resource_manager->load(get_font_id(m_package, i));
		}

		for (uint32_t i = 0; i < num_levels(m_package); i++)
		{
			m_resource_manager->load(get_level_id(m_package, i));
		}

		for (uint32_t i = 0; i < num_physics_configs(m_package); i++)
		{
			m_resource_manager->load(get_physics_config_id(m_package, i));
		}

		for (uint32_t i = 0; i < num_shaders(m_package); i++)
		{
			m_resource_manager->load(get_shader_id(m_package, i));
		}

		for (uint32_t i = 0; i < num_sprite_animations(m_package); i++)
		{
			m_resource_manager->load(get_sprite_animation_id(m_package, i));
		}
	}

	/// Unloads all the resources in the package.
	void unload()
	{
		using namespace package_resource;
		
		for (uint32_t i = 0; i < num_sprite_animations(m_package); i++)
		{
			m_resource_manager->unload(get_sprite_animation_id(m_package, i));
		}

		for (uint32_t i = 0; i < num_shaders(m_package); i++)
		{
			m_resource_manager->unload(get_shader_id(m_package, i));
		}

		for (uint32_t i = 0; i < num_physics_configs(m_package); i++)
		{
			m_resource_manager->unload(get_physics_config_id(m_package, i));
		}

		for (uint32_t i = 0; i < num_levels(m_package); i++)
		{
			m_resource_manager->unload(get_level_id(m_package, i));
		}

		for (uint32_t i = 0; i < num_fonts(m_package); i++)
		{
			m_resource_manager->unload(get_font_id(m_package, i));
		}

		for (uint32_t i = 0; i < num_materials(m_package); i++)
		{
			m_resource_manager->unload(get_material_id(m_package, i));
		}

		for (uint32_t i = 0; i < num_physics(m_package); i++)
		{
			m_resource_manager->unload(get_physics_id(m_package, i));
		}

		for (uint32_t i = 0; i < num_sprites(m_package); i++)
		{
			m_resource_manager->unload(get_sprite_id(m_package, i));
		}

		for (uint32_t i = 0; i < num_units(m_package); i++)
		{
			m_resource_manager->unload(get_unit_id(m_package, i));
		}

		for (uint32_t i = 0; i < num_meshes(m_package); i++)
		{
			m_resource_manager->unload(get_mesh_id(m_package, i));
		}

		for (uint32_t i = 0; i < num_sounds(m_package); i++)
		{
			m_resource_manager->unload(get_sound_id(m_package, i));
		}

		for (uint32_t i = 0; i < num_scripts(m_package); i++)
		{
			m_resource_manager->unload(get_script_id(m_package, i));
		}

		for (uint32_t i = 0; i < num_textures(m_package); i++)
		{
			m_resource_manager->unload(get_texture_id(m_package, i));
		}
	}

	/// Waits until the package has been loaded.
	void flush()
	{
		m_resource_manager->flush();
		m_has_loaded = true;
	}

	/// Returns whether the package has been loaded.
	bool has_loaded() const
	{
		return m_has_loaded;
	}

private:

	ResourceManager* m_resource_manager;
	ResourceId m_id;
	const PackageResource* m_package;
	bool m_has_loaded;
};

} // namespace crown
