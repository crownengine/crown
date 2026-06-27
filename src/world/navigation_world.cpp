/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/hash_map.inl"
#include "core/containers/array.inl"
#include "core/memory/memory.inl"
#include "core/memory/temp_allocator.inl"
#include "core/math/vector3.inl"
#include "core/math/matrix4x4.inl"
#include "world/debug_line.h"
#include "world/navigation_world.h"
#include "resource/navmesh_resource.h"
#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>
#include <DetourCrowd.h>
#include <DetourCommon.h>
#include <DetourDebugDraw.h>

namespace crown
{
static const int NAVMESH_MAX_POLYS = 256;
static const int CROWD_MAX_AGENTS = 128;

static float frand()
{
	return (float)rand() / (float)RAND_MAX;
}

NavigationWorld::NavigationWorld(Allocator &a)
	: _marker(NAVIGATION_WORLD_MARKER)
	, _allocator(&a)
	, _nav_mesh(NULL)
	, _nav_query(NULL)
	, _crowd(NULL)
	, _agent_map(a)
	, _agent_units(a)
{
}

NavigationWorld::~NavigationWorld()
{
	unload_navmesh();
	_marker = 0;
}

void NavigationWorld::load_navmesh(const void *data, u32 size)
{
	unload_navmesh();

	NavmeshResource *resource = (NavmeshResource *)data;

	u8 *nav_data = (u8 *)data + resource->data_offset;

	_nav_mesh = dtAllocNavMesh();
	if (!_nav_mesh)
		return;

	dtStatus status = _nav_mesh->init(nav_data, resource->size, DT_TILE_FREE_DATA);
	if (dtStatusFailed(status)) {
		dtFreeNavMesh(_nav_mesh);
		_nav_mesh = NULL;
		return;
	}

	_nav_query = dtAllocNavMeshQuery();
	if (_nav_query) {
		_nav_query->init(_nav_mesh, 2048);
	}

	_crowd = dtAllocCrowd();
	if (_crowd) {
		_crowd->init(CROWD_MAX_AGENTS, 0.6f, _nav_mesh);
	}
}

void NavigationWorld::unload_navmesh()
{
	if (_crowd) {
		_crowd->destroy();
		dtFreeCrowd(_crowd);
		_crowd = NULL;
	}

	if (_nav_query) {
		dtFreeNavMeshQuery(_nav_query);
		_nav_query = NULL;
	}

	if (_nav_mesh) {
		dtFreeNavMesh(_nav_mesh);
		_nav_mesh = NULL;
	}

	hash_map::clear(_agent_map);
	array::clear(_agent_units);
}

bool NavigationWorld::has_navmesh() const
{
	return _nav_mesh != NULL;
}

u32 NavigationWorld::find_path(Vector3 *path, u32 max_path, const Vector3 &start, const Vector3 &end) const
{
	if (!_nav_query)
		return 0;

	dtPolyRef start_poly;
	f32 start_nearest[3];
	dtPolyRef end_poly;
	f32 end_nearest[3];

	dtQueryFilter filter;
	filter.setIncludeFlags(0xffff);
	filter.setExcludeFlags(0);

	f32 extents[3] = { 2.0f, 4.0f, 2.0f };

	_nav_query->findNearestPoly(&start.x, extents, &filter, &start_poly, start_nearest);
	_nav_query->findNearestPoly(&end.x, extents, &filter, &end_poly, end_nearest);

	if (!start_poly || !end_poly)
		return 0;

	dtPolyRef polys[NAVMESH_MAX_POLYS];
	s32 num_polys;
	_nav_query->findPath(start_poly, end_poly, start_nearest, end_nearest, &filter, polys, &num_polys, NAVMESH_MAX_POLYS);

	if (!num_polys)
		return 0;

	f32 straight_path[NAVMESH_MAX_POLYS * 3];
	s32 num_verts;
	u8 straight_path_flags[NAVMESH_MAX_POLYS];
	dtPolyRef straight_path_polys[NAVMESH_MAX_POLYS];

	_nav_query->findStraightPath(start_nearest, end_nearest, polys, num_polys
		, straight_path, straight_path_flags, straight_path_polys, &num_verts, NAVMESH_MAX_POLYS);

	u32 count = (u32)num_verts < max_path ? (u32)num_verts : max_path;
	for (u32 i = 0; i < count; ++i) {
		path[i].x = straight_path[i * 3 + 0];
		path[i].y = straight_path[i * 3 + 1];
		path[i].z = straight_path[i * 3 + 2];
	}

	return count;
}

Vector3 NavigationWorld::nearest_point(const Vector3 &pos, f32 extents) const
{
	if (!_nav_query)
		return pos;

	dtPolyRef poly;
	f32 nearest[3];
	dtQueryFilter filter;
	filter.setIncludeFlags(0xffff);
	filter.setExcludeFlags(0);

	f32 ext[3] = { extents, extents, extents };
	_nav_query->findNearestPoly(&pos.x, ext, &filter, &poly, nearest);

	if (!poly)
		return pos;

	return vector3(nearest[0], nearest[1], nearest[2]);
}

Vector3 NavigationWorld::random_point() const
{
	if (!_nav_query)
		return VECTOR3_ZERO;

	dtQueryFilter filter;
	filter.setIncludeFlags(0xffff);
	filter.setExcludeFlags(0);

	dtPolyRef poly;
	f32 point[3];
	_nav_query->findRandomPoint(&filter, dtRand, &poly, point);

	return vector3(point[0], point[1], point[2]);
}

bool NavigationWorld::ray_cast(Vector3 &hit, const Vector3 &from, const Vector3 &to) const
{
	if (!_nav_query)
		return false;

	dtPolyRef start_poly;
	f32 start_nearest[3];
	dtQueryFilter filter;
	filter.setIncludeFlags(0xffff);
	filter.setExcludeFlags(0);

	f32 extents[3] = { 2.0f, 4.0f, 2.0f };
	_nav_query->findNearestPoly(&from.x, extents, &filter, &start_poly, start_nearest);

	if (!start_poly)
		return false;

	f32 t = 0;
	f32 hit_normal[3];
	dtPolyRef hit_polys[NAVMESH_MAX_POLYS];
	s32 num_hit_polys;
	f32 result[3];

	_nav_query->rayCast(start_poly, start_nearest, &to.x, &filter, &t, hit_normal, hit_polys, &num_hit_polys, NAVMESH_MAX_POLYS);

	if (t < 1.0f) {
		dtVlerp(result, &from.x, &to.x, t);
		hit = vector3(result[0], result[1], result[2]);
		return true;
	}

	hit = to;
	return false;
}

u32 NavigationWorld::agent_create(UnitId unit, f32 radius, f32 height, f32 max_speed, f32 acceleration)
{
	if (!_crowd)
		return UINT32_MAX;

	CE_ASSERT(!hash_map::has(_agent_map, unit), "Unit already has a navigation agent");

	dtCrowdAgentParams params;
	memset(&params, 0, sizeof(params));
	params.radius = radius;
	params.height = height;
	params.maxAcceleration = acceleration;
	params.maxSpeed = max_speed;
	params.collisionQueryRange = radius * 12.0f;
	params.pathOptimizationRange = radius * 30.0f;
	params.separationWeight = 2.0f;
	params.updateFlags = DT_CROWD_ANTICIPATE_TURNS
		| DT_CROWD_OBSTACLE_AVOIDANCE
		| DT_CROWD_SEPARATION
		| DT_CROWD_OPTIMIZE_VIS
		| DT_CROWD_OPTIMIZE_TOPO
	;

	s32 idx = _crowd->addAgent(&unit._idx, &params);
	if (idx < 0)
		return UINT32_MAX;

	u32 agent_id = (u32)idx;
	hash_map::set(_agent_map, unit, agent_id);
	array::push_back(_agent_units, unit);

	return agent_id;
}

void NavigationWorld::agent_destroy(u32 agent_id)
{
	if (!_crowd)
		return;

	if (agent_id >= (u32)_crowd->getAgentCount())
		return;

	_crowd->removeAgent((s32)agent_id);

	for (u32 i = 0; i < array::size(_agent_units); ++i) {
		u32 id = hash_map::get(_agent_map, _agent_units[i], UINT32_MAX);
		if (id == agent_id) {
			hash_map::remove(_agent_map, _agent_units[i]);
			_agent_units[i] = array::back(_agent_units);
			array::pop_back(_agent_units);
			break;
		}
	}
}

u32 NavigationWorld::agent(UnitId unit) const
{
	return hash_map::get(_agent_map, unit, UINT32_MAX);
}

void NavigationWorld::agent_set_target(u32 agent_id, const Vector3 &target)
{
	if (!_crowd)
		return;

	if (agent_id >= (u32)_crowd->getAgentCount())
		return;

	_crowd->requestMoveTarget((s32)agent_id, &target.x);
}

Vector3 NavigationWorld::agent_position(u32 agent_id) const
{
	if (!_crowd)
		return VECTOR3_ZERO;

	if (agent_id >= (u32)_crowd->getAgentCount())
		return VECTOR3_ZERO;

	const dtCrowdAgent *agent = _crowd->getAgent((s32)agent_id);
	if (!agent || !agent->active)
		return VECTOR3_ZERO;

	return vector3(agent->npos[0], agent->npos[1], agent->npos[2]);
}

Vector3 NavigationWorld::agent_velocity(u32 agent_id) const
{
	if (!_crowd)
		return VECTOR3_ZERO;

	if (agent_id >= (u32)_crowd->getAgentCount())
		return VECTOR3_ZERO;

	const dtCrowdAgent *agent = _crowd->getAgent((s32)agent_id);
	if (!agent || !agent->active)
		return VECTOR3_ZERO;

	return vector3(agent->vel[0], agent->vel[1], agent->vel[2]);
}

bool NavigationWorld::agent_is_at_target(u32 agent_id) const
{
	if (!_crowd)
		return false;

	if (agent_id >= (u32)_crowd->getAgentCount())
		return false;

	const dtCrowdAgent *agent = _crowd->getAgent((s32)agent_id);
	if (!agent || !agent->active)
		return false;

	return (agent->state == DT_CROWDAGENT_STATE_WALKING) ? false : true;
}

void NavigationWorld::update(f32 dt)
{
	if (_crowd) {
		_crowd->update(dt, NULL);
	}
}

void NavigationWorld::debug_draw(DebugLine *lines)
{
	if (!lines)
		return;

	if (_nav_mesh) {
		duDebugDrawNavMesh(/* TODO: wrap DebugLine as duDebugDraw */);
	}

	if (_crowd) {
		for (s32 i = 0; i < _crowd->getAgentCount(); ++i) {
			const dtCrowdAgent *agent = _crowd->getAgent(i);
			if (!agent || !agent->active)
				continue;

			Vector3 pos = vector3(agent->npos[0], agent->npos[1], agent->npos[2]);
			lines->add_sphere(pos, agent->params.radius, COLOR4_CYAN);
		}
	}
}

namespace navigation_world
{
	void init()
	{
	}

	void shutdown()
	{
	}

	NavigationWorld *create(Allocator &a)
	{
		return CE_NEW(a, NavigationWorld)(a);
	}

	void destroy(Allocator &a, NavigationWorld &nw)
	{
		CE_DELETE(a, &nw);
	}

} // namespace navigation_world

} // namespace crown
