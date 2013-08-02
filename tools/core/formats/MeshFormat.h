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

#include "Types.h"

namespace crown
{

//
// STRUCT
// {
//     FIELD             : SIZE                    COMMENT
// }
//
// MeshHeader [1]
// {
//     version           : uint32_t                Version identifier
//     mesh_count        : uint32_t                Number of meshes in the file
//     joint_count       : uint32_t                Number of joints in the file
//     padding           : uint32_t * 16           Reserved
// }
// MeshChunk [1, 2, ..., n]
// {
//     vertex_count      : uint32_t                Number of vertices in the mesh
//     vertices          : float * vertex_count    Vertex data
//
//     tri_count         : uint32_t                Number of triangles in the mesh
//     tris              : uint16_t * tri_count    Triangle data as indices into 'vertices'
// }
//

// Bump the version whenever a change in the format is made.
const uint32_t MESH_VERSION = 1;

struct MeshHeader
{
	uint32_t	version;
	uint32_t	mesh_count;
	uint32_t	joint_count;
	uint32_t	padding[16];
};

} // namespace crown
