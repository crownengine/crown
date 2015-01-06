/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "render_world_types.h"
#include "vector3.h"
#include "matrix4x4.h"
#include "quaternion.h"
#include "sprite_resource.h"
#include "material.h"
#include <bgfx.h>

namespace crown
{

class Renderer;
class RenderWorld;
struct SceneGraph;
struct Unit;
typedef Id MaterialId;

struct Sprite
{
	Sprite(RenderWorld& render_world, SceneGraph& sg, int32_t node, const SpriteResource* sr);

	/// Returns the local position of the sprite.
	Vector3 local_position() const;

	/// Returns the local rotation of the sprite.
	Quaternion local_rotation() const;

	/// Returns the local pose of the sprite.
	Matrix4x4 local_pose() const;

	/// Returns the world position of the sprite.
	Vector3 world_position() const;

	/// Returns the world rotation of the sprite.
	Quaternion world_rotation() const;

	/// Returns the world pose of the sprite.
	Matrix4x4 world_pose() const;

	/// Sets the local position of the sprite.
	void set_local_position(Unit* unit, const Vector3& pos);

	/// Sets the local rotation of the sprite.
	void set_local_rotation(Unit* unit, const Quaternion& rot);

	/// Sets the local pose of the sprite.
	void set_local_pose(Unit* unit, const Matrix4x4& pose);

	void set_material(MaterialId id);

	void set_frame(uint32_t i);

	void set_depth(int32_t z);

	void render();

public:

	RenderWorld& m_render_world;
	SceneGraph& m_scene_graph;
	int32_t m_node;
	const SpriteResource* m_resource;
	MaterialId m_material;
	uint32_t m_frame;
	int32_t _depth;
};

} // namespace crown
