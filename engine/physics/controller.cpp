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

#include "controller.h"
#include "math_utils.h"
#include "physics_resource.h"
#include "scene_graph.h"
#include "vector3.h"
#include "physics_callback.h"

#include "PxCapsuleController.h"
#include "PxPhysicsAPI.h"
using physx::PxCapsuleClimbingMode;
using physx::PxCapsuleController;
using physx::PxCapsuleControllerDesc;
using physx::PxCCTNonWalkableMode;
using physx::PxControllerFilters;
using physx::PxControllerFlag;
using physx::PxExtendedVec3;
using physx::PxVec3;

namespace crown
{

Controller::Controller(const PhysicsResource* pr, SceneGraph& sg, int32_t node, PxPhysics* physics, PxControllerManager* manager)
	: m_resource(pr)
	, m_scene_graph(sg)
	, m_node(node)
	, m_manager(manager)
	, m_controller(NULL)
{
	const PhysicsController* contr = physics_resource::controller(pr);
	const Vector3 pos = sg.world_position(m_node);

	PxCapsuleControllerDesc desc;
	desc.climbingMode = PxCapsuleClimbingMode::eCONSTRAINED;
	desc.nonWalkableMode = PxCCTNonWalkableMode::eFORCE_SLIDING;
	desc.radius = contr->radius;
	desc.height = contr->height;
	desc.slopeLimit = math::cos(contr->slope_limit);
	desc.stepOffset = contr->step_offset;
	desc.contactOffset = contr->contact_offset;
	desc.upDirection = PxVec3(0.0, 1.0, 0.0);
	desc.material = physics->createMaterial(0.5f, 0.5f, 0.5f);
	desc.position = PxExtendedVec3(pos.x, pos.y, pos.z);
	desc.reportCallback = &m_callback;
	CE_ASSERT(desc.isValid(), "Capsule is not valid");

	m_controller = manager->createController(desc);
	CE_ASSERT(m_controller, "Failed to create controller");
}

Controller::~Controller()
{
	m_controller->release();
}

void Controller::move(const Vector3& pos)
{
	const PxVec3 disp(pos.x, pos.y, pos.z);
	m_flags = m_controller->move(disp, 0.001, 1.0 / 60.0, PxControllerFilters());
}

void Controller::set_height(float height)
{
	m_controller->resize(height);
}

Vector3 Controller::position() const
{
	PxExtendedVec3 pos = m_controller->getPosition();
	return Vector3(pos.x, pos.y, pos.z);
}

bool Controller::collides_up() const
{
	return (m_flags & PxControllerFlag::eCOLLISION_UP) != 0;
}

bool Controller::collides_down() const
{
	return (m_flags & PxControllerFlag::eCOLLISION_DOWN) != 0;
}

bool Controller::collides_sides() const
{
	return (m_flags & PxControllerFlag::eCOLLISION_SIDES) != 0;
}

void Controller::update()
{
	m_scene_graph.set_world_position(m_node, position());
}

} // namespace crown
