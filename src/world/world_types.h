/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "math_types.h"

namespace crown
{

class Level;
class UnitManager;
class World;
struct SceneGraph;

#define UNIT_INDEX_BITS 22
#define UNIT_INDEX_MASK 0x003fffff
#define UNIT_ID_BITS    8
#define UNIT_ID_MASK    0x3fc00000

struct UnitId
{
	uint32_t idx;

	uint32_t index() const
	{
		return idx & UNIT_INDEX_MASK;
	}

	uint32_t id() const
	{
		return (idx >> UNIT_INDEX_BITS) & UNIT_ID_MASK;
	}

	uint32_t encode() const
	{
		return idx;
	}

	void decode(uint32_t id)
	{
		idx = id;
	}

	bool is_valid()
	{
		return idx != UINT32_MAX;
	}
};

inline UnitId INVALID_UNIT() { UnitId id = { UINT32_MAX }; return id; }

struct TransformInstance
{
	uint32_t i;
};

struct CameraInstance
{
	uint32_t i;
};

struct ProjectionType
{
	enum Enum
	{
		ORTHOGRAPHIC,
		PERSPECTIVE,

		COUNT
	};
};

struct TransformDesc
{
	Vector3 position;
	Quaternion rotation;
	Vector3 scale;
};

struct CameraDesc
{
	uint32_t type; // ProjectionType::Enum
	float fov;
	float near_range;
	float far_range;
};

struct EventType
{
	enum Enum
	{
		UNIT_SPAWNED,
		UNIT_DESTROYED,

		LEVEL_LOADED,

		PHYSICS_COLLISION,
		PHYSICS_TRIGGER,
		PHYSICS_TRANSFORM,

		COUNT
	};
};

struct UnitSpawnedEvent
{
	/// The unit spawned
	UnitId unit;
};

struct UnitDestroyedEvent
{
	/// The unit destroyed
	UnitId unit;
};

struct LevelLoadedEvent
{
};

} // namespace crown
