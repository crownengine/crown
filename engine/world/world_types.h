/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"

namespace crown
{

typedef Id UnitId;
typedef Id WorldId;
typedef Id CameraId;

struct Unit;
struct SceneGraph;
struct Camera;
class WorldManager;
class World;

struct EventType
{
	enum Enum
	{
		UNIT_SPAWNED,
		UNIT_DESTROYED,

		LEVEL_LOADED
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
