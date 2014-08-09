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

#include "Types.h"
#include "ResourceManager.h"
#include "PackageResource.h"
#include "Resource.h"
#include "Log.h"

namespace crown
{

/// Collection of resources to load in a batch.
struct ResourcePackage
{
public:

	//-----------------------------------------------------------------------------
	ResourcePackage(const char* name, ResourceManager& resman)
		: m_resource_manager(&resman)
		, m_id("package", name)
		, m_package(NULL)
		, m_has_loaded(false)
	{
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
		for (uint32_t i = 0; i < m_package->num_textures(); i++)
		{
			m_resource_manager->load(m_package->get_texture_id(i));
		}

		for (uint32_t i = 0; i < m_package->num_scripts(); i++)
		{
			m_resource_manager->load(m_package->get_script_id(i));
		}

		for (uint32_t i = 0; i < m_package->num_sounds(); i++)
		{
			m_resource_manager->load(m_package->get_sound_id(i));
		}

		for (uint32_t i = 0; i < m_package->num_meshes(); i++)
		{
			m_resource_manager->load(m_package->get_mesh_id(i));
		}

		for (uint32_t i = 0; i < m_package->num_units(); i++)
		{
			m_resource_manager->load(m_package->get_unit_id(i));
		}

		for (uint32_t i = 0; i < m_package->num_sprites(); i++)
		{
			m_resource_manager->load(m_package->get_sprite_id(i));
		}

		for (uint32_t i = 0; i < m_package->num_physics(); i++)
		{
			m_resource_manager->load(m_package->get_physics_id(i));
		}

		for (uint32_t i = 0; i < m_package->num_materials(); i++)
		{
			m_resource_manager->load(m_package->get_material_id(i));
		}

		for (uint32_t i = 0; i < m_package->num_fonts(); i++)
		{
			m_resource_manager->load(m_package->get_font_id(i));
		}

		for (uint32_t i = 0; i < m_package->num_levels(); i++)
		{
			m_resource_manager->load(m_package->get_level_id(i));
		}

		for (uint32_t i = 0; i < m_package->num_physics_configs(); i++)
		{
			m_resource_manager->load(m_package->get_physics_config_id(i));
		}
	}

	/// Unloads all the resources in the package.
	void unload()
	{
		for (uint32_t i = 0; i < m_package->num_physics_configs(); i++)
		{
			m_resource_manager->unload(m_package->get_physics_config_id(i));
		}

		for (uint32_t i = 0; i < m_package->num_levels(); i++)
		{
			m_resource_manager->unload(m_package->get_level_id(i));
		}

		for (uint32_t i = 0; i < m_package->num_fonts(); i++)
		{
			m_resource_manager->unload(m_package->get_font_id(i));
		}

		for (uint32_t i = 0; i < m_package->num_materials(); i++)
		{
			m_resource_manager->unload(m_package->get_material_id(i));
		}

		for (uint32_t i = 0; i < m_package->num_physics(); i++)
		{
			m_resource_manager->unload(m_package->get_physics_id(i));
		}

		for (uint32_t i = 0; i < m_package->num_sprites(); i++)
		{
			m_resource_manager->unload(m_package->get_sprite_id(i));
		}

		for (uint32_t i = 0; i < m_package->num_units(); i++)
		{
			m_resource_manager->unload(m_package->get_unit_id(i));
		}

		for (uint32_t i = 0; i < m_package->num_meshes(); i++)
		{
			m_resource_manager->unload(m_package->get_mesh_id(i));
		}

		for (uint32_t i = 0; i < m_package->num_sounds(); i++)
		{
			m_resource_manager->unload(m_package->get_sound_id(i));
		}

		for (uint32_t i = 0; i < m_package->num_scripts(); i++)
		{
			m_resource_manager->unload(m_package->get_script_id(i));
		}

		for (uint32_t i = 0; i < m_package->num_textures(); i++)
		{
			m_resource_manager->unload(m_package->get_texture_id(i));
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
	const ResourceId m_id;
	const PackageResource* m_package;
	bool m_has_loaded;
};

} // namespace crown
