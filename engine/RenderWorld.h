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

#include "IdTable.h"
#include "Mesh.h"
#include "Sprite.h"
#include "List.h"
#include "Vector3.h"
#include "Quaternion.h"

#define MAX_MESHES 100
#define MAX_SPRITES 256

namespace crown
{

typedef Id MeshId;
typedef Id SpriteId;

struct Camera;

class RenderWorld
{
public:

	RenderWorld();
	~RenderWorld();

	MeshId 		create_mesh(const char* mesh, int32_t node = -1, const Vector3& pos = Vector3::ZERO, const Quaternion& rot = Quaternion::IDENTITY);
	void 		destroy_mesh(MeshId id);
	Mesh* 		lookup_mesh(MeshId mesh);

	SpriteId	create_sprite(const char* name, int32_t node = -1, const Vector3& pos = Vector3::ZERO, const Quaternion& rot = Quaternion::IDENTITY);
	void		destroy_sprite(SpriteId id);
	Sprite*		lookup_sprite(SpriteId id);

	void 		update(Camera& camera, float dt);

	MeshId 		allocate_mesh(MeshResource* mr, int32_t node, const Vector3& pos, const Quaternion& rot);
	void 		deallocate_mesh(MeshId id);

	SpriteId	allocate_sprite(SpriteResource* sr, int32_t node, const Vector3& pos, const Quaternion& rot);
	void 		deallocate_sprite(SpriteId id);

private:

	IdTable<MAX_MESHES>		m_mesh_table;
	uint32_t				m_sparse_to_packed[MAX_MESHES];
	List<Mesh>				m_mesh;

	IdTable<MAX_SPRITES>	m_sprite_table;
	uint32_t				m_sprite_sparse_to_packed[MAX_SPRITES];
	List<Sprite>			m_sprite;
};

} // namespace crown
