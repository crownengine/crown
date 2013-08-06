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

#include "SceneGraph.h"
#include "HeapAllocator.h"
#include "IdTable.h"
#include "Unit.h"

namespace crown
{

#define MAX_CAMERAS 32
#define MAX_UNITS 1024

class Vec3;
class Quat;

class World
{
public:
	
					World();

	UnitId			spawn_unit(const char* name, const Vec3& pos = Vec3::ZERO, const Quat& rot = Quat(Vec3(0, 1, 0), 0.0f));
	void			kill_unit(UnitId unit);

	void			link_unit(UnitId child, UnitId parent);
	void			unlink_unit(UnitId child);

	Unit*			unit(UnitId unit);

	void			update(float dt);

private:

	HeapAllocator	m_allocator;

	SceneGraph		m_scene_graph;

	IdTable			m_unit_table;
	Unit*			m_units[MAX_UNITS];
};

} // namespace crown
