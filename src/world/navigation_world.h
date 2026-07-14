/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/math/types.h"
#include "core/memory/types.h"
#include "core/types.h"
#include "resource/types.h"
#include "world/types.h"

struct dtNavMesh;
struct dtNavMeshQuery;
struct dtCrowd;

namespace crown
{
struct DebugLine;

/// Manages navigation meshes and pathfinding in a World.
///
/// @ingroup World
struct NavigationWorld
{
	u32 _marker;
	Allocator *_allocator;

	dtNavMesh *_nav_mesh;
	dtNavMeshQuery *_nav_query;
	dtCrowd *_crowd;
	HashMap<UnitId, u32> _agent_map;
	Array<UnitId> _agent_units;

	NavigationWorld(Allocator &a);
	~NavigationWorld();

	NavigationWorld(const NavigationWorld &) = delete;
	NavigationWorld &operator=(const NavigationWorld &) = delete;

	/// Loads a navmesh from the given data pointer.
	void load_navmesh(const void *data, u32 size);

	/// Unloads the current navmesh.
	void unload_navmesh();

	/// Returns whether a navmesh is loaded.
	bool has_navmesh() const;

	/// Finds a path from @a start to @a end.
	/// Returns the number of path points found.
	u32 find_path(Vector3 *path, u32 max_path, const Vector3 &start, const Vector3 &end) const;

	/// Returns the nearest point on the navmesh to @a pos.
	Vector3 nearest_point(const Vector3 &pos, f32 extents) const;

	/// Returns a random point on the navmesh.
	Vector3 random_point() const;

	/// Casts a ray on the navmesh and returns the hit point if any.
	bool ray_cast(Vector3 &hit, const Vector3 &from, const Vector3 &to) const;

	/// Creates a navigation agent for the @a unit with the given @a radius and @a height.
	u32 agent_create(UnitId unit, f32 radius, f32 height, f32 max_speed, f32 acceleration);

	/// Destroys the agent with the given @a agent_id.
	void agent_destroy(u32 agent_id);

	/// Returns the agent id for the @a unit, or UINT32_MAX.
	u32 agent(UnitId unit) const;

	/// Sets the move target of the @a agent.
	void agent_set_target(u32 agent_id, const Vector3 &target);

	/// Returns the current position of the @a agent.
	Vector3 agent_position(u32 agent_id) const;

	/// Returns the current velocity of the @a agent.
	Vector3 agent_velocity(u32 agent_id) const;

	/// Returns whether the @a agent has reached its target.
	bool agent_is_at_target(u32 agent_id) const;

	/// Updates the crowd simulation.
	void update(f32 dt);

	/// Draws debug lines for the navmesh and agents.
	void debug_draw(DebugLine *lines);
};

namespace navigation_world
{
	void init();
	void shutdown();
	NavigationWorld *create(Allocator &a);
	void destroy(Allocator &a, NavigationWorld &nw);

} // namespace navigation_world

} // namespace crown
