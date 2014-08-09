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

#include "PxController.h"
#include "PxControllerManager.h"
#include "physics_callback.h"

using physx::PxController;
using physx::PxControllerManager;
using physx::PxPhysics;
using physx::PxScene;
using physx::PxU32;

namespace crown
{

struct PhysicsResource;
struct Vector3;
struct SceneGraph;
class PhysicsControllerCallback;

///
/// @ingroup Physics
struct Controller
{
							Controller(const PhysicsResource* pr, SceneGraph& sg, int32_t node, PxPhysics* physics, PxControllerManager* manager);
							~Controller();

	void					move(const Vector3& pos);
	void					set_height(float height);

	bool					collides_up() const;
	bool					collides_down() const;
	bool					collides_sides() const;

	Vector3					position() const;
	void					update();

private:

	const PhysicsResource*	m_resource;

	SceneGraph&				m_scene_graph;
	int32_t					m_node;
	PxControllerManager*	m_manager;
	PxController*			m_controller;
	PxU32					m_flags;

	PhysicsControllerCallback m_callback;
};

} // namespace crown
