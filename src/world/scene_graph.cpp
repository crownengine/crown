/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "scene_graph.h"
#include "quaternion.h"
#include "vector3.h"
#include "matrix3x3.h"
#include "matrix4x4.h"
#include "allocator.h"
#include "array.h"
#include <string.h> // memcpy
#include <stdint.h> // UINT_MAX

namespace crown
{

SceneGraph::SceneGraph(Allocator& a)
	: _allocator(a)
	, _map(a)
{
}

SceneGraph::~SceneGraph()
{
	_allocator.deallocate(_data.buffer);
}

TransformInstance SceneGraph::make_instance(uint32_t i)
{
	TransformInstance inst = { i };
	return inst;
}

void SceneGraph::allocate(uint32_t num)
{
	CE_ASSERT(num > _data.size, "num > _data.size");

	const uint32_t bytes = num * (0
		+ sizeof(UnitId)
		+ sizeof(Matrix4x4)
		+ sizeof(Pose)
		+ sizeof(TransformInstance) * 4);

	InstanceData new_data;
	new_data.size = _data.size;
	new_data.capacity = num;
	new_data.buffer = _allocator.allocate(bytes);

	new_data.unit = (UnitId*)(new_data.buffer);
	new_data.world = (Matrix4x4*)(new_data.unit + num);
	new_data.local = (Pose*)(new_data.world + num);
	new_data.parent = (TransformInstance*)(new_data.local + num);
	new_data.first_child = (TransformInstance*)(new_data.parent + num);
	new_data.next_sibling = (TransformInstance*)(new_data.first_child + num);
	new_data.prev_sibling = (TransformInstance*)(new_data.next_sibling + num);

	memcpy(new_data.unit, _data.unit, _data.size * sizeof(UnitId));
	memcpy(new_data.world, _data.world, _data.size * sizeof(Matrix4x4));
	memcpy(new_data.local, _data.local, _data.size * sizeof(Pose));
	memcpy(new_data.parent, _data.parent, _data.size * sizeof(TransformInstance));
	memcpy(new_data.first_child, _data.first_child, _data.size * sizeof(TransformInstance));
	memcpy(new_data.next_sibling, _data.next_sibling, _data.size * sizeof(TransformInstance));
	memcpy(new_data.prev_sibling, _data.prev_sibling, _data.size * sizeof(TransformInstance));

	_allocator.deallocate(_data.buffer);
	_data = new_data;
}

void SceneGraph::create(const Matrix4x4& m, UnitId id)
{
	if (_data.capacity == _data.size)
		grow();

	const uint32_t last = _data.size;

	_data.unit[last] = id;
	_data.world[last] = m;
	_data.local[last] = m;
	_data.parent[last].i = UINT32_MAX;
	_data.first_child[last].i = UINT32_MAX;
	_data.next_sibling[last].i = UINT32_MAX;
	_data.prev_sibling[last].i = UINT32_MAX;

	// FIXME
	if (array::size(_map) <= id.index)
		array::resize(_map, array::size(_map) + id.index + 1);

	_map[id.index] = last;

	++_data.size;
}

void SceneGraph::destroy(TransformInstance i)
{
	const uint32_t last = _data.size - 1;
	const UnitId u = _data.unit[i.i];
	const UnitId last_u = _data.unit[last];

	_data.unit[i.i] = _data.unit[last];
	_data.world[i.i] = _data.world[last];
	_data.local[i.i] = _data.local[last];
	_data.parent[i.i] = _data.parent[last];
	_data.first_child[i.i] = _data.first_child[last];
	_data.next_sibling[i.i] = _data.next_sibling[last];
	_data.prev_sibling[i.i] = _data.prev_sibling[last];

	_map[last_u.index] = i.i;
	_map[u.index] = UINT32_MAX;

	--_data.size;
}

TransformInstance SceneGraph::get(UnitId id)
{
	return make_instance(_map[id.index]);
}

void SceneGraph::set_local_position(TransformInstance i, const Vector3& pos)
{
	_data.local[i.i].position = pos;
	set_local(i);
}

void SceneGraph::set_local_rotation(TransformInstance i, const Quaternion& rot)
{
	_data.local[i.i].rotation = matrix3x3(rot);
	set_local(i);
}

void SceneGraph::set_local_scale(TransformInstance i, const Vector3& scale)
{
	_data.local[i.i].scale = scale;
	set_local(i);
}

void SceneGraph::set_local_pose(TransformInstance i, const Matrix4x4& pose)
{
	_data.local[i.i] = pose;
	set_local(i);
}

Vector3 SceneGraph::local_position(TransformInstance i) const
{
	return _data.local[i.i].position;
}

Quaternion SceneGraph::local_rotation(TransformInstance i) const
{
	return quaternion(_data.local[i.i].rotation);
}

Vector3 SceneGraph::local_scale(TransformInstance i) const
{
	return _data.local[i.i].scale;
}

Matrix4x4 SceneGraph::local_pose(TransformInstance i) const
{
	Matrix4x4 tr = matrix4x4(quaternion(_data.local[i.i].rotation), _data.local[i.i].position);
	set_scale(tr, _data.local[i.i].scale);
	return tr;
}

Vector3 SceneGraph::world_position(TransformInstance i) const
{
	return translation(_data.world[i.i]);
}

Quaternion SceneGraph::world_rotation(TransformInstance i) const
{
	return rotation(_data.world[i.i]);
}

Matrix4x4 SceneGraph::world_pose(TransformInstance i) const
{
	return _data.world[i.i];
}

uint32_t SceneGraph::num_nodes() const
{
	return _data.size;
}

void SceneGraph::link(TransformInstance child, TransformInstance parent)
{
	unlink(child);

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

	Matrix4x4 parent_tr = _data.world[parent.i];
	Matrix4x4 child_tr = _data.world[child.i];
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

	_data.local[child.i].position = translation(rel_tr);
	_data.local[child.i].rotation = to_matrix3x3(rel_tr);
	_data.local[child.i].scale = cs;
	_data.parent[child.i] = parent;

	transform(parent_tr, child);
}

void SceneGraph::unlink(TransformInstance child)
{
	if (!is_valid(_data.parent[child.i]))
		return;

	if (!is_valid(_data.prev_sibling[child.i]))
		_data.first_child[_data.parent[child.i].i] = _data.next_sibling[child.i];
	else
		_data.next_sibling[_data.prev_sibling[child.i].i] = _data.next_sibling[child.i];

	if (is_valid(_data.next_sibling[child.i]))
		_data.prev_sibling[_data.next_sibling[child.i].i] = _data.prev_sibling[child.i];

	_data.parent[child.i].i = UINT32_MAX;
	_data.next_sibling[child.i].i = UINT32_MAX;
	_data.prev_sibling[child.i].i = UINT32_MAX;
}

bool SceneGraph::is_valid(TransformInstance i)
{
	return i.i != UINT32_MAX;
}

void SceneGraph::set_local(TransformInstance i)
{
	TransformInstance parent = _data.parent[i.i];
	Matrix4x4 parent_tm = is_valid(parent) ? _data.world[parent.i] : MATRIX4X4_IDENTITY;
	transform(parent_tm, i);
}

void SceneGraph::transform(const Matrix4x4& parent, TransformInstance i)
{
	_data.world[i.i] = local_pose(i) * parent;

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

} // namespace crown
