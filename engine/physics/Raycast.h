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

#include "Log.h"
#include "PhysicsTypes.h"
#include "PxQueryFiltering.h"
#include "PxScene.h"
#include "PxVec3.h"
#include "EventStream.h"

using physx::PxQueryFilterData;
using physx::PxQueryFlag;
using physx::PxHitFlag;
using physx::PxHitFlags;
using physx::PxRaycastHit;
using physx::PxRaycastBuffer;
using physx::PxScene;
using physx::PxVec3;

namespace crown
{

struct Vector3;

struct Raycast
{
	/// Constructor
			Raycast(PxScene* scene, EventStream& events, const char* callback, SceneQueryMode::Enum mode, SceneQueryFilter::Enum filter);

	/// Performs a raycast against objects in the scene. The ray is casted from position @a from, has direction @a dir and is long @a length
	/// If any actor is hit along the ray, @a EventStream is filled according to @a mode previously specified and callback will be called for processing.
	/// @a SceneQueryMode::ANY: the callback is called with just true or false depending on whether the ray hit anything or not.
	/// @a SceneQueryMode::CLOSEST: the first argument will tell if there was a hit or not, as before. 
	/// If there was a hit, the callback will also be called with the position of the hit, the distance from the origin, the normal of the surface that 
	/// was hit and the actor that was hit.
	/// @a SceneQueryMode::ALL: as @a SceneQueryMode::CLOSEST, with more tuples
	void	cast(const Vector3& from, const Vector3& dir, const float length);

private:

	PxScene* 				m_scene;
	PxRaycastHit 			m_hits[CE_MAX_RAY_INTERSECTIONS];	
	PxRaycastBuffer			m_buffer;
	PxQueryFilterData 		m_fd;

	EventStream&			m_events;
	const char*				m_callback;

	SceneQueryMode::Enum		m_mode;
	SceneQueryFilter::Enum		m_filter;
};

} // namespace crown