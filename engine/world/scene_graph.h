/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "types.h"
#include "math_types.h"
#include "memory_types.h"
#include "unit_resource.h"

namespace crown
{

/// Represents a collection of nodes, possibly linked together to form a tree.
///
/// @ingroup World
struct SceneGraph
{
	SceneGraph(Allocator& a, uint32_t index);

	/// Creates the graph with @a count items.
	/// @a name, @a local and @parent are the array containing the name of the nodes,
	/// the local poses of the nodes and the links between the nodes respectively.
	/// A parent of -1 means "no parent".
	void create(const Matrix4x4& root, uint32_t count, const UnitNode* nodes);

	/// Destroys the graph deallocating memory if necessary.
	void destroy();

	/// Returns the index of the node with the given @a name
	int32_t node(const char* name) const;

	/// @copydoc SceneGraph::node()
	int32_t node(StringId32 name) const;

	/// Returns whether the graph has the node with the given @a name.
	bool has_node(const char* name) const;

	/// Returns the number of nodes in the graph.
	uint32_t num_nodes() const;

	/// Returns whether the node @a child can be linked to @a parent.
	bool can_link(int32_t child, int32_t parent) const;

	/// Links the @a child node to the @a parent node.
	/// After the linking the @a child pose is reset to identity.
	/// @note The @a parent node must be either -1 (meaning no parent), or an index lesser than child.
	void link(int32_t child, int32_t parent);

	/// Unlinks the @a child node from its parent if it has any.
	/// After unlinking, the @child local pose is set to its previous world pose.
	void unlink(int32_t child);

	/// Sets the local position, rotation or pose of the given @a node.
	void set_local_position(int32_t node, const Vector3& pos);

	/// @copydoc SceneGraph::set_local_position()
	void set_local_rotation(int32_t node, const Quaternion& rot);

	/// @copydoc SceneGraph::set_local_position()
	void set_local_pose(int32_t node, const Matrix4x4& pose);

	/// Returns the local position, rotation or pose of the given @a node.
	Vector3 local_position(int32_t node) const;

	/// @copydoc SceneGraph::local_position()
	Quaternion local_rotation(int32_t node) const;

	/// @copydoc SceneGraph::local_position()
	Matrix4x4 local_pose(int32_t node) const;

	/// Sets the world position, rotation or pose of the given @a node.
	/// @note This should never be called by user code.
	void set_world_position(int32_t node, const Vector3& pos);

	/// @copydoc SceneGraph::set_world_position()
	void set_world_rotation(int32_t node, const Quaternion& rot);

	/// @copydoc SceneGraph::set_world_position()
	void set_world_pose(int32_t node, const Matrix4x4& pose);

	/// Returns the world position, rotation or pose of the given @a node.
	Vector3 world_position(int32_t node) const;

	/// @copydoc SceneGraph::world_position()
	Quaternion world_rotation(int32_t node) const;

	/// @copydoc SceneGraph::world_position()
	Matrix4x4 world_pose(int32_t node) const;

	/// Transforms local poses to world poses.
	void update();

public:

	/// Index into SceneGraphManager
	Allocator* m_allocator;
	uint32_t m_index;

	uint32_t m_num_nodes;
	uint8_t* m_flags;
	Matrix4x4* m_world_poses;
	Matrix4x4* m_local_poses;
	int32_t* m_parents;
	StringId32* m_names;
};

} // namespace crown
