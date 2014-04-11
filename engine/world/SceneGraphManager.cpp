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

#include "SceneGraphManager.h"
#include "SceneGraph.h"
#include "Array.h"
#include "Memory.h"

namespace crown
{

//-----------------------------------------------------------------------------
SceneGraphManager::SceneGraphManager()
	: m_graphs(default_allocator())
{
}

//-----------------------------------------------------------------------------
SceneGraphManager::~SceneGraphManager()
{
}

//-----------------------------------------------------------------------------
SceneGraph* SceneGraphManager::create_scene_graph()
{
	uint32_t index = array::size(m_graphs);
	SceneGraph* sg = CE_NEW(default_allocator(), SceneGraph)(default_allocator(), index);
	array::push_back(m_graphs, sg);
	return sg;
}

//-----------------------------------------------------------------------------
void SceneGraphManager::destroy_scene_graph(SceneGraph* sg)
{
	CE_ASSERT_NOT_NULL(sg);

	m_graphs[sg->m_index] = m_graphs[array::size(m_graphs) - 1];
	m_graphs[sg->m_index]->m_index = sg->m_index;
	array::pop_back(m_graphs);

	CE_DELETE(default_allocator(), sg);
}

//-----------------------------------------------------------------------------
void SceneGraphManager::update()
{
	for (uint32_t i = 0; i < array::size(m_graphs); i++)
	{
		m_graphs[i]->update();
	}
}

} // namespace crown
