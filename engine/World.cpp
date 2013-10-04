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

#include "Assert.h"
#include "World.h"
#include "Allocator.h"

namespace crown
{

//-----------------------------------------------------------------------------
World::World()
	: m_allocator(default_allocator(), 1048576)
	, m_is_init(false)
{
}

//-----------------------------------------------------------------------------
void World::init()
{
}

//-----------------------------------------------------------------------------
void World::shutdown()
{

}

//-----------------------------------------------------------------------------
UnitId World::spawn_unit(const char* /*name*/, const Vec3& pos, const Quat& rot)
{
	const UnitId unit = m_unit_table.create();

	m_units[unit.index].create(pos, rot);

	// m_units[unit.index].load(ur);

	return unit;
}

//-----------------------------------------------------------------------------
void World::kill_unit(UnitId unit)
{
	CE_ASSERT(m_unit_table.has(unit), "Unit does not exist");
	(void)unit;
}

//-----------------------------------------------------------------------------
void World::link_unit(UnitId child, UnitId parent)
{
	CE_ASSERT(m_unit_table.has(child), "Child unit does not exist");
	CE_ASSERT(m_unit_table.has(parent), "Parent unit does not exist");

	Unit& child_unit = m_units[child.index];
	Unit& parent_unit =  m_units[parent.index];

	parent_unit.m_scene_graph.link(child_unit.m_root_node, parent_unit.m_root_node);
}

//-----------------------------------------------------------------------------
void World::unlink_unit(UnitId child, UnitId parent)
{
	CE_ASSERT(m_unit_table.has(child), "Child unit does not exist");
	CE_ASSERT(m_unit_table.has(parent), "Parent unit does not exist");

	Unit& child_unit = m_units[child.index];
	Unit& parent_unit =  m_units[parent.index];

	parent_unit.m_scene_graph.unlink(child_unit.m_root_node);		
}

//-----------------------------------------------------------------------------
Unit* World::unit(UnitId unit)
{
	CE_ASSERT(m_unit_table.has(unit), "Unit does not exist");

	return &m_units[unit.index];
}

//-----------------------------------------------------------------------------
void World::update(float /*dt*/)
{
}

//-----------------------------------------------------------------------------
SoundWorld&	World::sound_world()
{
	return m_sound_world;
}


} // namespace crown
