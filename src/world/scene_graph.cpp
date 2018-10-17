/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/containers/array.h"
#include "core/containers/hash_map.h"
#include "core/math/matrix3x3.h"
#include "core/math/matrix4x4.h"
#include "core/math/quaternion.h"
#include "core/math/vector3.h"
#include "core/memory/allocator.h"
#include "world/scene_graph.h"
#include "world/unit_manager.h"
#include <stdint.h> // UINT_MAX
#include <string.h> // memcpy

namespace crown
{
static void unit_destroyed_callback_bridge(UnitId id, void* user_ptr)
{
	((SceneGraph*)user_ptr)->unit_destroyed_callback(id);
}

SceneGraph::Pose& SceneGraph::Pose::operator=(const Matrix4x4& m)
{
	Matrix3x3 rotm = to_matrix3x3(m);
	normalize(rotm.x);
	normalize(rotm.y);
	normalize(rotm.z);

	position = translation(m);
	rotation = rotm;
	scale = crown::scale(m);
	return *this;
}

SceneGraph::SceneGraph(Allocator& a, UnitManager& um)
	: _marker(SCENE_GRAPH_MARKER)
	, _allocator(&a)
	, _unit_manager(&um)
	, _map(a)
{
	um.register_destroy_function(unit_destroyed_callback_bridge, this);
}

SceneGraph::~SceneGraph()
{
	_unit_manager->unregister_destroy_function(this);

	_allocator->deallocate(_data.buffer);

	_marker = 0;
}

TransformInstance SceneGraph::make_instance(u32 i)
{
	TransformInstance inst = { i };
	return inst;
}

void SceneGraph::allocate(u32 num)
{
	CE_ASSERT(num > _data.size, "num > _data.size");

	const u32 bytes = 0
		+ num*sizeof(UnitId) + alignof(UnitId)
		+ num*sizeof(Matrix4x4) + alignof(Matrix4x4)
		+ num*sizeof(Pose) + alignof(Pose)
		+ num*sizeof(TransformInstance) * 4 + alignof(TransformInstance)
		+ num*sizeof(bool) + alignof(bool)
		;

	InstanceData new_data;
	new_data.size = _data.size;
	new_data.capacity = num;
	new_data.buffer = _allocator->allocate(bytes);

	new_data.unit         = (UnitId*           )memory::align_top(new_data.buffer,             alignof(UnitId           ));
	new_data.world        = (Matrix4x4*        )memory::align_top(new_data.unit + num,         alignof(Matrix4x4        ));
	new_data.local        = (Pose*             )memory::align_top(new_data.world + num,        alignof(Pose             ));
	new_data.parent       = (TransformInstance*)memory::align_top(new_data.local + num,        alignof(TransformInstance));
	new_data.first_child  = (TransformInstance*)memory::align_top(new_data.parent + num,       alignof(TransformInstance));
	new_data.next_sibling = (TransformInstance*)memory::align_top(new_data.first_child + num,  alignof(TransformInstance));
	new_data.prev_sibling = (TransformInstance*)memory::align_top(new_data.next_sibling + num, alignof(TransformInstance));
	new_data.changed      = (bool*             )memory::align_top(new_data.prev_sibling + num, alignof(bool             ));

	memcpy(new_data.unit, _data.unit, _data.size * sizeof(UnitId));
	memcpy(new_data.world, _data.world, _data.size * sizeof(Matrix4x4));
	memcpy(new_data.local, _data.local, _data.size * sizeof(Pose));
	memcpy(new_data.parent, _data.parent, _data.size * sizeof(TransformInstance));
	memcpy(new_data.first_child, _data.first_child, _data.size * sizeof(TransformInstance));
	memcpy(new_data.next_sibling, _data.next_sibling, _data.size * sizeof(TransformInstance));
	memcpy(new_data.prev_sibling, _data.prev_sibling, _data.size * sizeof(TransformInstance));
	memcpy(new_data.changed, _data.changed, _data.size * sizeof(bool));

	_allocator->deallocate(_data.buffer);
	_data = new_data;
}

void SceneGraph::unit_destroyed_callback(UnitId id)
{
	if (has(id))
		destroy(id, make_instance(UINT32_MAX));
}

TransformInstance SceneGraph::create(UnitId unit, const Vector3& pos, const Quaternion& rot, const Vector3& scale)
{
	Matrix4x4 pose;
	set_identity(pose);
	set_translation(pose, pos);
	set_rotation(pose, rot);
	set_scale(pose, scale);

	return create(unit, pose);
}

TransformInstance SceneGraph::create(UnitId unit, const Matrix4x4& pose)
{
	CE_ASSERT(!hash_map::has(_map, unit), "Unit already has transform");

	if (_data.capacity == _data.size)
		grow();

	const u32 last = _data.size;

	_data.unit[last]           = unit;
	_data.world[last]          = pose;
	_data.local[last]          = pose;
	_data.parent[last].i       = UINT32_MAX;
	_data.first_child[last].i  = UINT32_MAX;
	_data.next_sibling[last].i = UINT32_MAX;
	_data.prev_sibling[last].i = UINT32_MAX;
	_data.changed[last]        = false;

	++_data.size;

	hash_map::set(_map, unit, last);

	return make_instance(last);
}

void SceneGraph::destroy(UnitId unit, TransformInstance /*id*/)
{
	TransformInstance i = make_instance(hash_map::get(_map, unit, UINT32_MAX));
	CE_ASSERT(i.i < _data.size, "Index out of bounds");

	const u32 last = _data.size - 1;
	const UnitId u = _data.unit[i.i];
	const UnitId last_u = _data.unit[last];

	_data.unit[i.i]         = _data.unit[last];
	_data.world[i.i]        = _data.world[last];
	_data.local[i.i]        = _data.local[last];
	_data.parent[i.i]       = _data.parent[last];
	_data.first_child[i.i]  = _data.first_child[last];
	_data.next_sibling[i.i] = _data.next_sibling[last];
	_data.prev_sibling[i.i] = _data.prev_sibling[last];
	_data.changed[i.i]      = _data.changed[last];

	hash_map::set(_map, last_u, i.i);
	hash_map::remove(_map, u);

	--_data.size;
}

TransformInstance SceneGraph::instances(UnitId unit)
{
	return make_instance(hash_map::get(_map, unit, UINT32_MAX));
}

bool SceneGraph::has(UnitId unit)
{
	return hash_map::has(_map, unit);
}

void SceneGraph::set_local_position(UnitId unit, const Vector3& pos)
{
	TransformInstance i = make_instance(hash_map::get(_map, unit, UINT32_MAX));
	CE_ASSERT(i.i < _data.size, "Index out of bounds");
	_data.local[i.i].position = pos;
	set_local(i);
}

void SceneGraph::set_local_rotation(UnitId unit, const Quaternion& rot)
{
	TransformInstance i = make_instance(hash_map::get(_map, unit, UINT32_MAX));
	CE_ASSERT(i.i < _data.size, "Index out of bounds");
	_data.local[i.i].rotation = matrix3x3(rot);
	set_local(i);
}

void SceneGraph::set_local_scale(UnitId unit, const Vector3& scale)
{
	TransformInstance i = make_instance(hash_map::get(_map, unit, UINT32_MAX));
	CE_ASSERT(i.i < _data.size, "Index out of bounds");
	_data.local[i.i].scale = scale;
	set_local(i);
}

void SceneGraph::set_local_pose(UnitId unit, const Matrix4x4& pose)
{
	TransformInstance i = make_instance(hash_map::get(_map, unit, UINT32_MAX));
	CE_ASSERT(i.i < _data.size, "Index out of bounds");
	_data.local[i.i] = pose;
	set_local(i);
}

Vector3 SceneGraph::local_position(UnitId unit)
{
	TransformInstance i = make_instance(hash_map::get(_map, unit, UINT32_MAX));
	CE_ASSERT(i.i < _data.size, "Index out of bounds");
	return _data.local[i.i].position;
}

Quaternion SceneGraph::local_rotation(UnitId unit)
{
	TransformInstance i = make_instance(hash_map::get(_map, unit, UINT32_MAX));
	CE_ASSERT(i.i < _data.size, "Index out of bounds");
	return quaternion(_data.local[i.i].rotation);
}

Vector3 SceneGraph::local_scale(UnitId unit)
{
	TransformInstance i = make_instance(hash_map::get(_map, unit, UINT32_MAX));
	CE_ASSERT(i.i < _data.size, "Index out of bounds");
	return _data.local[i.i].scale;
}

Matrix4x4 SceneGraph::local_pose(UnitId unit)
{
	TransformInstance i = make_instance(hash_map::get(_map, unit, UINT32_MAX));
	CE_ASSERT(i.i < _data.size, "Index out of bounds");
	Matrix4x4 tr = matrix4x4(quaternion(_data.local[i.i].rotation), _data.local[i.i].position);
	set_scale(tr, _data.local[i.i].scale);
	return tr;
}

Vector3 SceneGraph::world_position(UnitId unit)
{
	TransformInstance i = make_instance(hash_map::get(_map, unit, UINT32_MAX));
	CE_ASSERT(i.i < _data.size, "Index out of bounds");
	return translation(_data.world[i.i]);
}

Quaternion SceneGraph::world_rotation(UnitId unit)
{
	TransformInstance i = make_instance(hash_map::get(_map, unit, UINT32_MAX));
	CE_ASSERT(i.i < _data.size, "Index out of bounds");
	return rotation(_data.world[i.i]);
}

Matrix4x4 SceneGraph::world_pose(UnitId unit)
{
	TransformInstance i = make_instance(hash_map::get(_map, unit, UINT32_MAX));
	CE_ASSERT(i.i < _data.size, "Index out of bounds");
	return _data.world[i.i];
}

void SceneGraph::set_world_pose(TransformInstance i, const Matrix4x4& pose)
{
	CE_ASSERT(i.i < _data.size, "Index out of bounds");
	_data.world[i.i] = pose;
	_data.changed[i.i] = true;
}

u32 SceneGraph::num_nodes() const
{
	return _data.size;
}

void SceneGraph::link(UnitId child, UnitId parent)
{
	TransformInstance tc = make_instance(hash_map::get(_map, child, UINT32_MAX));
	TransformInstance tp = make_instance(hash_map::get(_map, parent, UINT32_MAX));
	CE_ASSERT(tc.i < _data.size, "Index out of bounds");
	CE_ASSERT(tp.i < _data.size, "Index out of bounds");

	unlink(child);

	if (!is_valid(_data.first_child[tp.i]))
	{
		_data.first_child[tp.i] = tc;
		_data.parent[tc.i] = tp;
	}
	else
	{
		TransformInstance prev = { UINT32_MAX };
		TransformInstance node = _data.first_child[tp.i];
		while (is_valid(node))
		{
			prev = node;
			node = _data.next_sibling[node.i];
		}

		_data.next_sibling[prev.i] = tc;

		_data.first_child[tc.i].i = UINT32_MAX;
		_data.next_sibling[tc.i].i = UINT32_MAX;
		_data.prev_sibling[tc.i] = prev;
	}

	Matrix4x4 parent_tr = _data.world[tp.i];
	Matrix4x4 child_tr = _data.world[tc.i];
	const Vector3 cs = scale(child_tr);

	Vector3 px = x(parent_tr);
	Vector3 py = y(parent_tr);
	Vector3 pz = z(parent_tr);
	Vector3 cx = x(child_tr);
	Vector3 cy = y(child_tr);
	Vector3 cz = z(child_tr);

	set_x(parent_tr, normalize(px));
	set_y(parent_tr, normalize(py));
	set_z(parent_tr, normalize(pz));
	set_x(child_tr, normalize(cx));
	set_y(child_tr, normalize(cy));
	set_z(child_tr, normalize(cz));

	const Matrix4x4 rel_tr = child_tr * get_inverted(parent_tr);

	_data.local[tc.i].position = translation(rel_tr);
	_data.local[tc.i].rotation = to_matrix3x3(rel_tr);
	_data.local[tc.i].scale = cs;
	_data.parent[tc.i] = tp;

	transform(parent_tr, tc);
}

void SceneGraph::unlink(UnitId unit)
{
	TransformInstance tc = make_instance(hash_map::get(_map, unit, UINT32_MAX));
	CE_ASSERT(tc.i < _data.size, "Index out of bounds");

	if (!is_valid(_data.parent[tc.i]))
		return;

	if (!is_valid(_data.prev_sibling[tc.i]))
		_data.first_child[_data.parent[tc.i].i] = _data.next_sibling[tc.i];
	else
		_data.next_sibling[_data.prev_sibling[tc.i].i] = _data.next_sibling[tc.i];

	if (is_valid(_data.next_sibling[tc.i]))
		_data.prev_sibling[_data.next_sibling[tc.i].i] = _data.prev_sibling[tc.i];

	_data.parent[tc.i].i = UINT32_MAX;
	_data.next_sibling[tc.i].i = UINT32_MAX;
	_data.prev_sibling[tc.i].i = UINT32_MAX;
}

void SceneGraph::clear_changed()
{
	for (u32 i = 0; i < _data.size; ++i)
	{
		_data.changed[i] = false;
	}
}

void SceneGraph::get_changed(Array<UnitId>& units, Array<Matrix4x4>& world_poses)
{
	for (u32 i = 0; i < _data.size; ++i)
	{
		if (_data.changed[i])
		{
			array::push_back(units, _data.unit[i]);
			array::push_back(world_poses, _data.world[i]);
		}
	}
}

void SceneGraph::set_local(TransformInstance i)
{
	TransformInstance parent = _data.parent[i.i];
	Matrix4x4 parent_tm = is_valid(parent) ? _data.world[parent.i] : MATRIX4X4_IDENTITY;
	transform(parent_tm, i);

	_data.changed[i.i] = true;
}

void SceneGraph::transform(const Matrix4x4& parent, TransformInstance i)
{
	_data.world[i.i] = local_pose(_data.unit[i.i]) * parent;

	TransformInstance child = _data.first_child[i.i];
	while (is_valid(child))
	{
		transform(_data.world[i.i], child);
		child = _data.next_sibling[child.i];
	}
}

void SceneGraph::grow()
{
	allocate(_data.capacity * 2 + 1);
}

} // namespace crown
