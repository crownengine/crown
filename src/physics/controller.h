/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "physics_callback.h"
#include "world_types.h"
#include "resource_types.h"
#include "math_types.h"
#include "PxController.h"
#include "PxControllerManager.h"

using physx::PxController;
using physx::PxControllerManager;
using physx::PxPhysics;
using physx::PxScene;
using physx::PxU32;

namespace crown
{

struct SceneGraph;

///
/// @ingroup Physics
struct Controller
{
	Controller(const ControllerResource* cr, SceneGraph& sg, UnitId id, PxPhysics* physics, PxControllerManager* manager);
	~Controller();

	/// Moves the controller to @a pos.
	void move(const Vector3& pos);

	/// Sets the contoller height.
	void set_height(float height);

	/// Returns whether the contoller collides upwards.
	bool collides_up() const;

	/// Returns whether the controller collides downwards.
	bool collides_down() const;

	/// Returns whether the controller collides sidewards.
	bool collides_sides() const;

	/// Returns the position of the controller.
	Vector3 position() const;

	void update();

private:

	const ControllerResource* m_resource;

	SceneGraph& m_scene_graph;
	UnitId _unit_id;
	PxControllerManager* m_manager;
	PxController* m_controller;
	PxU32 m_flags;
	PhysicsControllerCallback m_callback;
};

} // namespace crown
