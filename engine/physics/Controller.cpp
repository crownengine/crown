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

#include "Controller.h"
#include "MathUtils.h"
#include "PhysicsResource.h"
#include "Device.h"
#include "Vector3.h"

#include "PxCapsuleController.h"
using physx::PxCapsuleController;
using physx::PxCapsuleControllerDesc;
using physx::PxVec3;

namespace crown
{

//-----------------------------------------------------------------------------
Controller::Controller(const PhysicsResource* pr, PxScene* scene, PxControllerManager* manager)
	: m_resource(pr)
	, m_scene(scene)
	, m_manager(manager)
	, m_controller(NULL)
{
	const PhysicsController contr = pr->controller();

	PxCapsuleControllerDesc desc;
	desc.radius = math::cos(contr.radius);
	desc.height = contr.height;
	desc.slopeLimit = contr.slope_limit;
	desc.stepOffset = contr.step_offset;
	desc.contactOffset = contr.contact_offset;
	desc.upDirection = PxVec3(0.0, 1.0, 0.0);

	m_controller = manager->createController(*device()->physx(), scene, desc);
}

//-----------------------------------------------------------------------------
Controller::~Controller()
{
	// FIXME
}

//-----------------------------------------------------------------------------
void Controller::move(const Vector3& pos)
{
	// FIXME
}

} // namespace crown
