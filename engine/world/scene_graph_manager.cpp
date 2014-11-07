/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "scene_graph_manager.h"
#include "scene_graph.h"
#include "array.h"
#include "memory.h"

namespace crown
{

SceneGraphManager::SceneGraphManager()
	: m_graphs(default_allocator())
{
}

SceneGraphManager::~SceneGraphManager()
{
}

SceneGraph* SceneGraphManager::create_scene_graph()
{
	uint32_t index = array::size(m_graphs);
	SceneGraph* sg = CE_NEW(default_allocator(), SceneGraph)(default_allocator(), index);
	array::push_back(m_graphs, sg);
	return sg;
}

void SceneGraphManager::destroy_scene_graph(SceneGraph* sg)
{
	CE_ASSERT_NOT_NULL(sg);

	m_graphs[sg->m_index] = m_graphs[array::size(m_graphs) - 1];
	m_graphs[sg->m_index]->m_index = sg->m_index;
	array::pop_back(m_graphs);

	CE_DELETE(default_allocator(), sg);
}

void SceneGraphManager::update()
{
	for (uint32_t i = 0; i < array::size(m_graphs); i++)
	{
		m_graphs[i]->update();
	}
}

} // namespace crown
