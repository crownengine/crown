/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/types.h"
#include "core/math/constants.h"
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
	UnitDestroyCallback _unit_destroy_callback;

	///
	SceneGraph(Allocator& a, UnitManager& um);

	///
	~SceneGraph();

	/// Creates a new transform instance for the @a unit.
	TransformInstance create(UnitId unit, const Matrix4x4& pose);

	/// Creates a new transform instance for the @a unit.
	TransformInstance create(UnitId unit, const Vector3& pos, const Quaternion& rot, const Vector3& scale);

	/// Destroys the @a transform.
	void destroy(TransformInstance transform);

	/// Returns the ID of the transform owned by the *unit*.
	TransformInstance instance(UnitId unit);

	/// Returns whether the @a unit has a transform.
	bool has(UnitId unit);

	/// Sets the local position, rotation, scale or pose of the @a transform.
	void set_local_position(TransformInstance transform, const Vector3& pos);

	/// @copydoc SceneGraph::set_local_position()
	void set_local_rotation(TransformInstance transform, const Quaternion& rot);

	/// @copydoc SceneGraph::set_local_position()
	void set_local_scale(TransformInstance transform, const Vector3& scale);

	/// @copydoc SceneGraph::set_local_position()
	void set_local_pose(TransformInstance transform, const Matrix4x4& pose);

	/// Returns the local position, rotation or pose of the @a transform.
	Vector3 local_position(TransformInstance transform);

	/// @copydoc SceneGraph::local_position()
	Quaternion local_rotation(TransformInstance transform);

	/// @copydoc SceneGraph::local_position()
	Vector3 local_scale(TransformInstance transform);

	/// @copydoc SceneGraph::local_position()
	Matrix4x4 local_pose(TransformInstance transform);

	/// Returns the world position, rotation or pose of the @a transform.
	Vector3 world_position(TransformInstance transform);

	/// @copydoc SceneGraph::world_position()
	Quaternion world_rotation(TransformInstance transform);

	/// @copydoc SceneGraph::world_position()
	Matrix4x4 world_pose(TransformInstance transform);

	///
	void set_world_pose(TransformInstance transform, const Matrix4x4& pose);

	///
	void set_world_pose_and_rescale(TransformInstance transform, const Matrix4x4& pose);

	/// Returns the number of nodes in the graph.
	u32 num_nodes() const;

	/// Links @a child to @a parent. After linking the child will follow its
	/// parent. Set child_local_* to modify the child position after it has been
	/// linked to the parent, otherwise che child will be positioned at the
	/// location of its parent.
	void link(TransformInstance parent
		, TransformInstance child
		, const Vector3& child_local_position = VECTOR3_ZERO
		, const Quaternion& child_local_rotation = QUATERNION_IDENTITY
		, const Vector3& child_local_scale = VECTOR3_ONE
		);

	/// Unlinks @a child from its parent if it has any. After unlinking, the local
	/// pose of the @a child is set to its previous world pose.
	void unlink(TransformInstance child);

	void clear_changed();
	void get_changed(Array<UnitId>& units, Array<Matrix4x4>& world_poses);
	void set_local(TransformInstance transform);
	void transform(const Matrix4x4& parent, TransformInstance transform);
	void grow();
	void allocate(u32 num);
	TransformInstance make_instance(u32 i);
	void unit_destroyed_callback(UnitId unit);
};

} // namespace crown
