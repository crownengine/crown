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

// Core
#include "Assert.h"
#include "Types.h"
#include "Args.h"
#include "Log.h"

// Core/Math
#include "Color4.h"
#include "Interpolation.h"
#include "Intersection.h"
#include "Mat3.h"
#include "Mat4.h"
#include "MathUtils.h"
#include "Plane.h"
#include "Point2.h"
#include "Quat.h"
#include "Random.h"
#include "Ray.h"
#include "Shape.h"
#include "Triangle.h"
#include "Vec2.h"
#include "Vec3.h"
#include "Vec4.h"

// Core/Bv
#include "Box.h"
#include "Circle.h"
#include "Frustum.h"
#include "Rect.h"
#include "Sphere.h"

// Core/Containers
#include "Dictionary.h"
#include "RBTree.h"
#include "List.h"
#include "Generic.h"
#include "Queue.h"
#include "IdTable.h"

// Core/Strings
#include "String.h"
#include "Hash.h"
#include "Path.h"

// Core/Mem
#include "Memory.h"
#include "Allocator.h"
#include "MallocAllocator.h"
#include "ProxyAllocator.h"

// Core/Filesystem
#include "File.h"
#include "DiskFile.h"
#include "MemoryFile.h"
#include "NullFile.h"
#include "BinaryReader.h"
#include "BinaryWriter.h"
#include "TextReader.h"
#include "TextWriter.h"
#include "Filesystem.h"

// Core/Threads
#include "Thread.h"
#include "Mutex.h"
#include "Cond.h"

// Core/Settings
#include "IntSetting.h"
#include "FloatSetting.h"

// Engine
#include "Camera.h"
#include "Device.h"
#include "Glyph.h"
#include "ResourceArchive.h"
#include "ArchiveResourceArchive.h"
#include "FileResourceArchive.h"
#include "ResourceManager.h"
#include "Skybox.h"

#include "TextResource.h"
#include "TextureResource.h"
#include "ScriptResource.h"
#include "MaterialResource.h"
#include "FontResource.h"

// Engine/Input
#include "EventDispatcher.h"
#include "InputManager.h"
#include "Keyboard.h"
#include "KeyCode.h"
#include "Mouse.h"
#include "Touch.h"

// Engine/Renderers
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Renderer.h"
#include "Material.h"
#include "Texture.h"
#include "DebugRenderer.h"
#include "PixelFormat.h"
#include "VertexFormat.h"

