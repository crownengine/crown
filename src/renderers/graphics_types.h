/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "string_id.h"
#include "math_types.h"

namespace crown
{

class ShaderManager;
class MaterialManager;
class RenderWorld;
struct DebugLine;
struct Gui;
struct Material;

struct MeshInstance
{
	uint32_t i;
};

struct SpriteInstance
{
	uint32_t i;
};

struct LightInstance
{
	uint32_t i;
};

/// Enumerates light types.
///
/// @ingroup Graphics
struct LightType
{
	enum Enum
	{
		DIRECTIONAL,
		OMNI,
		SPOT,

		COUNT
	};
};

struct MeshRendererDesc
{
	StringId64 mesh_resource;
	StringId32 mesh_name;
	StringId64 material_resource; // FIXME
	bool visible;
	char _pad[3];
};

struct SpriteRendererDesc
{
	StringId64 sprite_resource;
	StringId64 material_resource; // FIXME
	bool visible;
	char _pad[3];
	char _pad1[4];
};

struct LightDesc
{
	uint32_t type;    // LightType::Enum
	float range;      // In meters
	float intensity;
	float spot_angle; // In radians
	Vector3 color;
};

} // namespace crown
