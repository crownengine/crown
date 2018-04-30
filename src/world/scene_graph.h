/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/types.h"
#include "core/math/types.h"
#include "core/memory/types.h"
#include "core/types.h"
#include "world/types.h"

namespace crown
{
/// Represents a collection of nodes, possibly linked together to form a tree.
///
/// @ingroup World
struct SceneGraph
{
	struct Pose
	{
		Vector3 position;
		Matrix3x3 rotation;
		Vector3 scale;

		Pose& operator=(const Matrix4x4& m);
	};

	struct InstanceData
	{
		InstanceData()
			: size(0)
			, capacity(0)
			, buffer(NULL)
			, unit(NULL)
			, world(NULL)
			, local(NULL)
			, parent(NULL)
			, first_child(NULL)
			, next_sibling(NULL)
			, prev_sibling(NULL)
			, changed(NULL)
		{
		}

		u32 size;
		u32 capacity;
		void* buffer;

		UnitId* unit;
		Matrix4x4* world;
		Pose* local;
		TransformInstance* parent;
		TransformInstance* first_child;
		TransformInstance* next_sibling;
		TransformInstance* prev_sibling;
		bool* changed;
	};

	u32 _marker;
	Allocator* _allocator;
	UnitManager* _unit_manager;
	InstanceData _data;
	HashMap<UnitId, u32> _map;

	///
	SceneGraph(Allocator& a, UnitManager& um);

	///
	~SceneGraph();

	/// Creates a new transform instance for unit @a id.
	TransformInstance create(UnitId id, const Matrix4x4& pose);

	/// Creates a new transform instance for unit @a id.
	TransformInstance create(UnitId id, const Vector3& pos, const Quaternion& rot, const Vector3& scale);

	/// Destroys the transform for the @a unit. The transform is ignored.
	void destroy(UnitId unit, TransformInstance id);

	/// Returns the transform instance of unit @a id.
	TransformInstance instances(UnitId id);

	/// Returns whether the @a unit has a transform.
	bool has(UnitId unit);

	/// Sets the local position, rotation, scale or pose of the given @a unit.
	void set_local_position(UnitId unit, const Vector3& pos);

	/// @copydoc SceneGraph::set_local_position()
	void set_local_rotation(UnitId unit, const Quaternion& rot);

	/// @copydoc SceneGraph::set_local_position()
	void set_local_scale(UnitId unit, const Vector3& scale);

	/// @copydoc SceneGraph::set_local_position()
	void set_local_pose(UnitId unit, const Matrix4x4& pose);

	/// Returns the local position, rotation or pose of the given @a unit.
	Vector3 local_position(UnitId unit);

	/// @copydoc SceneGraph::local_position()
	Quaternion local_rotation(UnitId unit);

	/// @copydoc SceneGraph::local_position()
	Vector3 local_scale(UnitId unit);

	/// @copydoc SceneGraph::local_position()
	Matrix4x4 local_pose(UnitId unit);

	/// Returns the world position, rotation or pose of the given @a unit.
	Vector3 world_position(UnitId unit);

	/// @copydoc SceneGraph::world_position()
	Quaternion world_rotation(UnitId unit);

	/// @copydoc SceneGraph::world_position()
	Matrix4x4 world_pose(UnitId unit);

	void set_world_pose(TransformInstance i, const Matrix4x4& pose);

	/// Returns the number of nodes in the graph.
	u32 num_nodes() const;

	/// Links the unit @a child to the unit @a parent.
	void link(UnitId child, UnitId parent);

	/// Unlinks the @a unit from its parent if it has any.
	/// After unlinking, the @a unit's local pose is set to its previous world pose.
	void unlink(UnitId unit);

	void clear_changed();
	void get_changed(Array<UnitId>& units, Array<Matrix4x4>& world_poses);
	void set_local(TransformInstance i);
	void transform(const Matrix4x4& parent, TransformInstance i);
	void grow();
	void allocate(u32 num);
	TransformInstance make_instance(u32 i);
	void unit_destroyed_callback(UnitId id);
};

} // namespace crown
