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

#include "IdArray.h"
#include "Mesh.h"
#include "Sprite.h"
#include "List.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "PoolAllocator.h"
#include "Resource.h"

#define MAX_MESHES 100
#define MAX_SPRITES 256

namespace crown
{

typedef Id MeshId;
typedef Id SpriteId;

class SceneGraph;
struct Camera;

class RenderWorld
{
public:

	RenderWorld();
	~RenderWorld();

	MeshId		create_mesh(ResourceId id, SceneGraph& sg, int32_t node);
	void 		destroy_mesh(MeshId id);
	Mesh* 		lookup_mesh(MeshId mesh);

	SpriteId	create_sprite(ResourceId id, SceneGraph& sg, int32_t node);
	void		destroy_sprite(SpriteId id);
	Sprite*		lookup_sprite(SpriteId id);

	void		update(const Matrix4x4& view, const Matrix4x4& projection, uint16_t x, uint16_t y, uint16_t width, uint16_t height, float dt);

private:

	PoolAllocator					m_mesh_pool;
	IdArray<MAX_MESHES, Mesh*>		m_mesh;

	PoolAllocator					m_sprite_pool;
	IdArray<MAX_SPRITES, Sprite*>	m_sprite;
};

} // namespace crown
