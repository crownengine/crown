/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
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

Controller::Controller(const ControllerResource* cr, SceneGraph& sg, UnitId id, PxPhysics* physics, PxControllerManager* manager)
	: m_resource(cr)
	, m_scene_graph(sg)
	, _unit_id(id)
	, m_manager(manager)
	, m_controller(NULL)
{
	TransformInstance ti = sg.get(id);
	const Vector3 pos = sg.world_position(ti);

	PxCapsuleControllerDesc desc;
	desc.climbingMode = PxCapsuleClimbingMode::eCONSTRAINED;
	desc.nonWalkableMode = PxCCTNonWalkableMode::eFORCE_SLIDING;
	desc.radius = cr->radius;
	desc.height = cr->height;
	desc.slopeLimit = cosf(cr->slope_limit);
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
	return vector3(pos.x, pos.y, pos.z);
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
	TransformInstance ti = m_scene_graph.get(_unit_id);
	m_scene_graph.set_local_position(ti, position());
}

} // namespace crown
