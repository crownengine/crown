/*
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

#include "ResourceManager.h"
#include "Mesh.h"

namespace crown
{

/**
 * Manages mesh loading.
 */
class MeshManager : public ResourceManager
{

public:

	MeshManager();

	/**
	 * Creates a mesh resource with the given name.
	 * If a resource with the same name already exists, the
	 * already existent resource will be returned.
	 * @param name The name of the resource
	 * @return A pointer to the created resource
	 */
	Mesh* Create(const char* name, bool& created);

	/**
	 * Loads a mesh resource from file.
	 * The name of the file determines the name of the resource and vice-versa.
	 * @param name Tha name of the resource
	 * @return A pointer to the loaded resource
	 */
	Mesh* Load(const char* name);

	/**
	 * Loads a prefab cube mesh.
	 * @param name The name of the mesh
	 * @param size The size of the cube in units
	 * @return A pointer to the loaded resource
	 */
	Mesh* LoadCube(const char* name, float size = 1.0f);

	/**
	 * Loads a prefab plane mesh.
	 * @note Plane differs from a grid in the way a plane has
	 * only four vertices while a grid has many more, depending
	 * on the size. You can imagine a plane as a singe sheet of
	 * paper.
	 * The function generates a plane of size 'width' x 'height' units,
	 * centered at the origin and facing +Z. 
	 * @param name The name of the mesh
	 * @param width The width of the plane in units
	 * @param height The height of the plane in units
	 * @return A pointer to the loaded resource
	 */
	Mesh* LoadPlane(const char* name, float width = 1.0f, float height = 1.0f);

	/**
	 * Loads a prefab plane mesh.
 	 * @note A grid differs from a plane in the way a grid
	 * has many more vertices than a plane, depending on the
	 * size. You can think of a grid as a set of small-tiled
	 * planes or as a single "triangulated" plane.
	 * @param name The name of the mesh
	 * @param size The size of the plane in units
	 * @param tileSize The size of a tile in units
	 * @return A pointer to the loaded resource
	 */
	Mesh* LoadGrid(const char* name, uint size = 20, float tileSize = 1.0f);

	virtual Mesh* CreateSpecific(const char* name);

private:

	// Disable copying
	MeshManager(const MeshManager&);
	MeshManager& operator=(const MeshManager&);
};

MeshManager* GetMeshManager();

} // namespace crown

