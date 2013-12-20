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

#include "WorldManager.h"
#include "World.h"

namespace crown
{

//-----------------------------------------------------------------------------
WorldManager::WorldManager()
	: m_allocator("world-manager", default_allocator())
{
}

//-----------------------------------------------------------------------------
WorldManager::~WorldManager()
{
}

//-----------------------------------------------------------------------------
WorldId WorldManager::create_world()
{
	World* w = CE_NEW(m_allocator, World)();
	const WorldId w_id = m_worlds.create(w);
	w->set_id(w_id);
	return w_id;
}

//-----------------------------------------------------------------------------
void WorldManager::destroy_world(WorldId id)
{
	CE_DELETE(m_allocator, m_worlds.lookup(id));
	m_worlds.destroy(id);
}

//-----------------------------------------------------------------------------
World* WorldManager::lookup_world(WorldId id)
{
	return m_worlds.lookup(id);
}

//-----------------------------------------------------------------------------
IdArray<MAX_WORLDS, World*>& WorldManager::worlds()
{
	return m_worlds;
}

} // namespace crown
