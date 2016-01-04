/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "math_types.h"
#include "matrix4x4.h"
#include "memory_types.h"
#include "world_types.h"
#include "container_types.h"
#include "matrix3x3.h"
#include "matrix4x4.h"
#include "vector3.h"

namespace crown
{

struct TransformInstance
{
	uint32_t i;
};

/// Represents a collection of nodes, possibly linked together to form a tree.
///
/// @ingroup World
struct SceneGraph
{
	SceneGraph(Allocator& a);
	~SceneGraph();

	/// Creates a new transform instance for unit @a id.
	TransformInstance create(UnitId id, const Matrix4x4& m);

	/// Destroys the transform @a i.
	void destroy(TransformInstance i);

	/// Returns the transform instance of unit @a id.
	TransformInstance get(UnitId id);

	/// Sets the local position, rotation, scale or pose of the given @a node.
	void set_local_position(TransformInstance i, const Vector3& pos);

	/// @copydoc SceneGraph::set_local_position()
	void set_local_rotation(TransformInstance i, const Quaternion& rot);

	/// @copydoc SceneGraph::set_local_position()
	void set_local_scale(TransformInstance i, const Vector3& scale);

	/// @copydoc SceneGraph::set_local_position()
	void set_local_pose(TransformInstance i, const Matrix4x4& pose);

	/// Returns the local position, rotation or pose of the given @a node.
	Vector3 local_position(TransformInstance i) const;

	/// @copydoc SceneGraph::local_position()
	Quaternion local_rotation(TransformInstance i) const;

	/// @copydoc SceneGraph::local_position()
	Vector3 local_scale(TransformInstance i) const;

	/// @copydoc SceneGraph::local_position()
	Matrix4x4 local_pose(TransformInstance i) const;

	/// Returns the world position, rotation or pose of the given @a node.
	Vector3 world_position(TransformInstance i) const;

	/// @copydoc SceneGraph::world_position()
	Quaternion world_rotation(TransformInstance i) const;

	/// @copydoc SceneGraph::world_position()
	Matrix4x4 world_pose(TransformInstance i) const;

	/// Returns the number of nodes in the graph.
	uint32_t num_nodes() const;

	/// Links the @a child node to the @a parent node.
	void link(TransformInstance child, TransformInstance parent);

	/// Unlinks the @a child node from its parent if it has any.
	/// After unlinking, the @child local pose is set to its previous world pose.
	void unlink(TransformInstance child);

	bool is_valid(TransformInstance i);

	void set_local(TransformInstance i);

	void transform(const Matrix4x4& parent, TransformInstance i);

public:

	enum { MARKER = 0x63a44dbf };

private:

	void grow();
	void allocate(uint32_t num);
	TransformInstance make_instance(uint32_t i);

	struct Pose
	{
		Pose& operator=(const Matrix4x4& m)
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

		Vector3 position;
		Matrix3x3 rotation;
		Vector3 scale;
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
		{
		}

		uint32_t size;
		uint32_t capacity;
		void* buffer;

		UnitId* unit;
		Matrix4x4* world;
		Pose* local;
		TransformInstance* parent;
		TransformInstance* first_child;
		TransformInstance* next_sibling;
		TransformInstance* prev_sibling;
	};

	uint32_t _marker;

	Allocator& _allocator;
	InstanceData _data;
	Hash<uint32_t> _map;
};

} // namespace crown
