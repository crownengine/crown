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

// Core
#include "Assert.h"
#include "Types.h"
#include "Args.h"
#include "Log.h"

// Core/Math
#include "AABB.h"
#include "Color4.h"
#include "Frustum.h"
#include "Intersection.h"
#include "MathUtils.h"
#include "Matrix3x3.h"
#include "Matrix4x4.h"
#include "Plane.h"
#include "Quaternion.h"
#include "Random.h"
#include "Ray.h"
#include "Sphere.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

// Core/Containers
#include "ContainerTypes.h"
#include "EventStream.h"
#include "Hash.h"
#include "IdArray.h"
#include "IdTable.h"
#include "Map.h"
#include "PriorityQueue.h"
#include "Queue.h"
#include "Vector.h"

// Core/Strings
#include "StringUtils.h"
#include "StringStream.h"
#include "StringUtils.h"
#include "Path.h"

// Core/Mem
#include "Memory.h"
#include "Allocator.h"
#include "TempAllocator.h"
#include "LinearAllocator.h"
#include "StackAllocator.h"
#include "ProxyAllocator.h"
#include "PoolAllocator.h"

// Core/Filesystem
#include "DiskFile.h"
#include "File.h"
#include "Filesystem.h"
#include "NullFile.h"
#include "ReaderWriter.h"

// Core/Json
#include "JSON.h"
#include "JSONParser.h"

// Core/Settings
#include "IntSetting.h"
#include "FloatSetting.h"
#include "StringSetting.h"

// Engine
#include "Camera.h"
#include "Device.h"
#include "ConsoleServer.h"

// Engine/Input
#include "Keyboard.h"
#include "KeyCode.h"
#include "Mouse.h"
#include "Touch.h"

// Engine/Lua
#include "LuaStack.h"
#include "LuaEnvironment.h"

// Engine/Os
#include "Thread.h"
#include "Mutex.h"
#include "Cond.h"
#include "OsFile.h"
#include "OsWindow.h"
#include "OsSocket.h"

// Engine/Renderers
#include "Renderer.h"
#include "RenderContext.h"
#include "PixelFormat.h"
#include "VertexFormat.h"

// Engine/Resource
#include "Bundle.h"
#include "ResourceLoader.h"
#include "ResourceManager.h"
#include "TextureResource.h"
#include "MeshResource.h"
#include "SoundResource.h"
#include "MaterialResource.h"
#include "FontResource.h"

// Engine/Audio
#include "SoundWorld.h"
