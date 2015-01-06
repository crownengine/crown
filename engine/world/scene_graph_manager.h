/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "container_types.h"

namespace crown
{

struct SceneGraph;

/// Manages a collection of scene graphs.
///
/// @ingroup World
class SceneGraphManager
{
public:

	SceneGraphManager();
	~SceneGraphManager();

	/// Creates a new scene graph
	SceneGraph* create_scene_graph();

	/// Destroys the @a sg scene graph
	void destroy_scene_graph(SceneGraph* sg);

	/// Updates all the scene graphs
	void update();

private:

	Array<SceneGraph*> m_graphs;
};

} // namespace crown
