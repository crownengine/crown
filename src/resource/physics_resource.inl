/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "resource/physics_resource.h"

namespace crown
{
namespace physics_config_resource
{
	const PhysicsMaterial *materials_array(const PhysicsConfigResource *pcr)
	{
		return (const PhysicsMaterial *)((char *)pcr + pcr->materials_offset);
	}

	u32 material_index(const PhysicsMaterial *materials, u32 num, StringId32 name)
	{
		for (u32 i = 0; i < num; ++i) {
			if (materials[i].name == name)
				return i;
		}

		CE_FATAL("Material not found");
		return UINT32_MAX;
	}

	const PhysicsActor *actors_array(const PhysicsConfigResource *pcr)
	{
		return (const PhysicsActor *)((char *)pcr + pcr->actors_offset);
	}

	u32 actor_index(const PhysicsActor *actors, u32 num, StringId32 name)
	{
		for (u32 i = 0; i < num; ++i) {
			if (actors[i].name == name)
				return i;
		}

		CE_FATAL("Actor not found");
		return UINT32_MAX;
	}

	const PhysicsCollisionFilter *filters_array(const PhysicsConfigResource *pcr)
	{
		return (const PhysicsCollisionFilter *)((char *)pcr + pcr->filters_offset);
	}

	u32 filter_index(const PhysicsCollisionFilter *filters, u32 num, StringId32 name)
	{
		for (u32 i = 0; i < num; ++i) {
			if (filters[i].name == name)
				return i;
		}

		CE_FATAL("Filter not found");
		return UINT32_MAX;
	}

} // namespace physics_config_resource

} // namespace crown
