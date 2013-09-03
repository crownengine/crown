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

namespace crown
{

class ResourcePackage
{
public:

	//-----------------------------------------------------------------------------
	ResourcePackage(ResourceManager& resman, const PackageResource* package)
		: m_resource_manager(&resman), m_package(package), m_has_loaded(false)
	{
		CE_ASSERT_NOT_NULL(package);
	}

	//-----------------------------------------------------------------------------
	void load()
	{
		Log::i("ResourcePackage: loading %d textures", m_package->num_textures());
		Log::i("ResourcePackage: loading %d scripts", m_package->num_scripts());
		for (uint32_t i = 0; i < m_package->num_textures(); i++)
		{
			m_resource_manager->load(TEXTURE_TYPE, m_package->get_texture_id(i));
		}

		for (uint32_t i = 0; i < m_package->num_scripts(); i++)
		{
			m_resource_manager->load(LUA_TYPE, m_package->get_script_id(i));
		}
	}

	//-----------------------------------------------------------------------------
	void unload()
	{
		for (uint32_t i = 0; i < m_package->num_textures(); i++)
		{
			m_resource_manager->unload(m_package->get_texture_id(i));
		}

		for (uint32_t i = 0; i < m_package->num_scripts(); i++)
		{
			m_resource_manager->unload(m_package->get_script_id(i));
		}		
	}

	//-----------------------------------------------------------------------------
	void flush()
	{
		m_resource_manager->flush();
		m_has_loaded = true;
	}

	//-----------------------------------------------------------------------------
	bool has_loaded() const
	{
		return m_has_loaded;
	}

private:

	ResourceManager* m_resource_manager;
	const PackageResource* m_package;
	bool m_has_loaded;
};

} // namespace crown
