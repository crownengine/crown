/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
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

Controller::Controller(const ControllerResource* cr, SceneGraph& sg, int32_t node, PxPhysics* physics, PxControllerManager* manager)
	: m_resource(cr)
	, m_scene_graph(sg)
	, m_node(node)
	, m_manager(manager)
	, m_controller(NULL)
{
	const Vector3 pos = sg.world_position(m_node);

	PxCapsuleControllerDesc desc;
	desc.climbingMode = PxCapsuleClimbingMode::eCONSTRAINED;
	desc.nonWalkableMode = PxCCTNonWalkableMode::eFORCE_SLIDING;
	desc.radius = cr->radius;
	desc.height = cr->height;
	desc.slopeLimit = cos(cr->slope_limit);
	desc.stepOffset = cr->step_offset;
	desc.contactOffset = cr->contact_offset;
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
