/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/math/constants.h"
#include "core/math/matrix3x3.inl"
#include "core/math/matrix4x4.inl"
#include "core/math/quaternion.inl"
#include "core/math/vector3.inl"
#include "core/memory/allocator.h"
#include "world/scene_graph.h"
#include "world/unit_manager.h"
#include <stdint.h> // UINT_MAX
#include <string.h> // memcpy

namespace crown
{
static void unit_destroyed_callback_bridge(UnitId unit, void* user_ptr)
{
	((SceneGraph*)user_ptr)->unit_destroyed_callback(unit);
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
	_unit_destroy_callback.destroy = unit_destroyed_callback_bridge;
	_unit_destroy_callback.user_data = this;
	_unit_destroy_callback.node.next = NULL;
	_unit_destroy_callback.node.prev = NULL;
	um.register_destroy_callback(&_unit_destroy_callback);
}

SceneGraph::~SceneGraph()
{
	_unit_manager->unregister_destroy_callback(&_unit_destroy_callback);

	_allocator->deallocate(_data.buffer);

	_marker = 0;
}

TransformInstance SceneGraph::make_instance(u32 i)
{
	TransformInstance inst = { i }; return inst;
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

void SceneGraph::unit_destroyed_callback(UnitId unit)
{
	TransformInstance inst = instance(unit);
	if (is_valid(inst))
		destroy(inst);
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
	CE_ASSERT(!hash_map::has(_map, unit), "Unit already has a transform component");

	if (_data.capacity == 0 || _data.capacity-1 == _data.size)
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

/// Moves the node data from index @a src to index @a dst, while preserving
/// internal links between nodes.
static void scene_graph_move_data(const SceneGraph& sg, u32 dst, u32 src)
{
	// Any node can be referenced by its children.
	TransformInstance cur = sg._data.first_child[src];
	while (is_valid(cur))
	{
		sg._data.parent[cur.i].i = dst;
		cur = sg._data.next_sibling[cur.i];
	}

	if (sg._data.parent[src].i != UINT32_MAX)
	{
		// Child node can also be referenced by its parent (if it is the parent's
		// first child)...
		if (sg._data.first_child[sg._data.parent[src].i].i == src)
			sg._data.first_child[sg._data.parent[src].i].i = dst;

		// ... and (at most) by its two closest siblings.
		if (sg._data.next_sibling[src].i != UINT32_MAX)
			sg._data.prev_sibling[sg._data.next_sibling[src].i].i = dst;
		if (sg._data.prev_sibling[src].i != UINT32_MAX)
			sg._data.next_sibling[sg._data.prev_sibling[src].i].i = dst;
	}

	sg._data.unit[dst]         = sg._data.unit[src];
	sg._data.world[dst]        = sg._data.world[src];
	sg._data.local[dst]        = sg._data.local[src];
	sg._data.parent[dst]       = sg._data.parent[src];
	sg._data.first_child[dst]  = sg._data.first_child[src];
	sg._data.next_sibling[dst] = sg._data.next_sibling[src];
	sg._data.prev_sibling[dst] = sg._data.prev_sibling[src];
	sg._data.changed[dst]      = sg._data.changed[src];
}

/// Swaps the transforms @a aa and @a bb.
static void scene_graph_swap(const SceneGraph& sg, TransformInstance aa, TransformInstance bb)
{
	// Index of the temporary storage slot. Memory access past size-1 is allowed
	// because we allocate one extra slot in SceneGraph::grow().
	const u32 tt = sg._data.size;

	scene_graph_move_data(sg, tt, aa.i);
	scene_graph_move_data(sg, aa.i, bb.i);
	scene_graph_move_data(sg, bb.i, tt);
}

void SceneGraph::destroy(TransformInstance transform)
{
	CE_ASSERT(transform.i < _data.size, "Index out of bounds");

	// Unlink all children.
	TransformInstance cur = _data.first_child[transform.i];
	while (is_valid(cur))
	{
		TransformInstance next_sibling = _data.next_sibling[cur.i];
		unlink(cur);
		cur = next_sibling;
	}
	unlink(transform);

	const u32 last = _data.size - 1;
	const UnitId u = _data.unit[transform.i];
	const UnitId last_u = _data.unit[last];

	if (last != transform.i)
	{
		scene_graph_swap(*this, transform, make_instance(last));
		hash_map::set(_map, last_u, transform.i);
	}
	hash_map::remove(_map, u);

	--_data.size;
}

TransformInstance SceneGraph::instance(UnitId unit)
{
	return make_instance(hash_map::get(_map, unit, UINT32_MAX));
}

bool SceneGraph::has(UnitId unit)
{
	return hash_map::has(_map, unit);
}

void SceneGraph::set_local_position(TransformInstance transform, const Vector3& pos)
{
	CE_ASSERT(transform.i < _data.size, "Index out of bounds");
	_data.local[transform.i].position = pos;
	set_local(transform);
}

void SceneGraph::set_local_rotation(TransformInstance transform, const Quaternion& rot)
{
	CE_ASSERT(transform.i < _data.size, "Index out of bounds");
	_data.local[transform.i].rotation = from_quaternion(rot);
	set_local(transform);
}

void SceneGraph::set_local_scale(TransformInstance transform, const Vector3& scale)
{
	CE_ASSERT(transform.i < _data.size, "Index out of bounds");
	_data.local[transform.i].scale = scale;
	set_local(transform);
}

void SceneGraph::set_local_pose(TransformInstance transform, const Matrix4x4& pose)
{
	CE_ASSERT(transform.i < _data.size, "Index out of bounds");
	_data.local[transform.i] = pose;
	set_local(transform);
}

Vector3 SceneGraph::local_position(TransformInstance transform)
{
	CE_ASSERT(transform.i < _data.size, "Index out of bounds");
	return _data.local[transform.i].position;
}

Quaternion SceneGraph::local_rotation(TransformInstance transform)
{
	CE_ASSERT(transform.i < _data.size, "Index out of bounds");
	return rotation(_data.local[transform.i].rotation);
}

Vector3 SceneGraph::local_scale(TransformInstance transform)
{
	CE_ASSERT(transform.i < _data.size, "Index out of bounds");
	return _data.local[transform.i].scale;
}

Matrix4x4 SceneGraph::local_pose(TransformInstance transform)
{
	CE_ASSERT(transform.i < _data.size, "Index out of bounds");
	Matrix4x4 tr = from_quaternion_translation(quaternion(_data.local[transform.i].rotation), _data.local[transform.i].position);
	set_scale(tr, _data.local[transform.i].scale);
	return tr;
}

Vector3 SceneGraph::world_position(TransformInstance transform)
{
	CE_ASSERT(transform.i < _data.size, "Index out of bounds");
	return translation(_data.world[transform.i]);
}

Quaternion SceneGraph::world_rotation(TransformInstance transform)
{
	CE_ASSERT(transform.i < _data.size, "Index out of bounds");
	return rotation(_data.world[transform.i]);
}

Matrix4x4 SceneGraph::world_pose(TransformInstance transform)
{
	CE_ASSERT(transform.i < _data.size, "Index out of bounds");
	return _data.world[transform.i];
}

void SceneGraph::set_world_pose(TransformInstance transform, const Matrix4x4& pose)
{
	CE_ASSERT(transform.i < _data.size, "Index out of bounds");
	_data.world[transform.i] = pose;
	_data.changed[transform.i] = true;
}

void SceneGraph::set_world_pose_and_rescale(TransformInstance transform, const Matrix4x4& pose)
{
	CE_ASSERT(transform.i < _data.size, "Index out of bounds");
	_data.world[transform.i] = pose;
	set_scale(_data.world[transform.i], _data.local[transform.i].scale);
	_data.changed[transform.i] = true;
}

u32 SceneGraph::num_nodes() const
{
	return _data.size;
}

void SceneGraph::link(TransformInstance parent
	, TransformInstance child
	, const Vector3& child_local_position
	, const Quaternion& child_local_rotation
	, const Vector3& child_local_scale
	)
{
	CE_ASSERT(child.i < _data.size, "Index out of bounds");
	CE_ASSERT(parent.i < _data.size, "Index out of bounds");

	unlink(child);

	// Append transform to the list of parent's children
	if (!is_valid(_data.first_child[parent.i]))
	{
		_data.first_child[parent.i] = child;
		_data.parent[child.i] = parent;
	}
	else
	{
		TransformInstance prev = { UINT32_MAX };
		TransformInstance node = _data.first_child[parent.i];
		while (is_valid(node))
		{
			prev = node;
			node = _data.next_sibling[node.i];
		}

		_data.next_sibling[prev.i] = child;

		_data.first_child[child.i].i = UINT32_MAX;
		_data.next_sibling[child.i].i = UINT32_MAX;
		_data.prev_sibling[child.i] = prev;
	}

	_data.local[child.i].position = child_local_position;
	_data.local[child.i].rotation = from_quaternion(child_local_rotation);
	_data.local[child.i].scale    = child_local_scale;
	_data.parent[child.i] = parent;

	transform(_data.world[parent.i], child);
}

void SceneGraph::unlink(TransformInstance child)
{
	CE_ASSERT(child.i < _data.size, "Index out of bounds");

	if (!is_valid(_data.parent[child.i]))
		return;

	if (_data.first_child[_data.parent[child.i].i].i == child.i)
		_data.first_child[_data.parent[child.i].i] = _data.next_sibling[child.i];
	else
		_data.next_sibling[_data.prev_sibling[child.i].i] = _data.next_sibling[child.i];

	if (is_valid(_data.next_sibling[child.i]))
		_data.prev_sibling[_data.next_sibling[child.i].i] = _data.prev_sibling[child.i];

	_data.local[child.i].position = translation(_data.world[child.i]);
	_data.local[child.i].rotation = from_quaternion(rotation(_data.world[child.i]));
	_data.local[child.i].scale = scale(_data.world[child.i]);
	_data.parent[child.i].i = UINT32_MAX;
	_data.next_sibling[child.i].i = UINT32_MAX;
	_data.prev_sibling[child.i].i = UINT32_MAX;
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

void SceneGraph::set_local(TransformInstance transform)
{
	TransformInstance parent = _data.parent[transform.i];
	Matrix4x4 parent_tm = is_valid(parent) ? _data.world[parent.i] : MATRIX4X4_IDENTITY;
	SceneGraph::transform(parent_tm, transform);

	_data.changed[transform.i] = true;
}

void SceneGraph::transform(const Matrix4x4& parent, TransformInstance transform)
{
	_data.world[transform.i] = local_pose(transform) * parent;
	_data.changed[transform.i] = true;

	TransformInstance child = _data.first_child[transform.i];
	while (is_valid(child))
	{
		SceneGraph::transform(_data.world[transform.i], child);
		child = _data.next_sibling[child.i];
	}
}

void SceneGraph::grow()
{
	// Allocate one extra slot to be used as a temporary storage.
	allocate(2 + _data.capacity*2);
}

} // namespace crown
