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
#include "assert.h"
#include "types.h"
#include "args.h"
#include "log.h"

// Core/Math
#include "aabb.h"
#include "color4.h"
#include "frustum.h"
#include "intersection.h"
#include "math_utils.h"
#include "matrix3x3.h"
#include "matrix4x4.h"
#include "plane.h"
#include "quaternion.h"
#include "random.h"
#include "ray.h"
#include "sphere.h"
#include "vector2.h"
#include "vector3.h"
#include "vector4.h"

// Core/Containers
#include "container_types.h"
#include "event_stream.h"
#include "hash.h"
#include "id_array.h"
#include "id_table.h"
#include "map.h"
#include "priority_queue.h"
#include "queue.h"
#include "vector.h"

// Core/Strings
#include "string_utils.h"
#include "string_stream.h"
#include "string_utils.h"
#include "path.h"

// Core/Mem
#include "memory.h"
#include "allocator.h"
#include "temp_allocator.h"
#include "linear_allocator.h"
#include "stack_allocator.h"
#include "proxy_allocator.h"
#include "pool_allocator.h"

// Core/Filesystem
#include "disk_file.h"
#include "file.h"
#include "filesystem.h"
#include "null_file.h"
#include "reader_writer.h"

// Core/Json
#include "json.h"
#include "json_parser.h"

// Core/Settings
#include "int_setting.h"
#include "float_setting.h"
#include "string_setting.h"

// Engine
#include "camera.h"
#include "device.h"
#include "console_server.h"

// Engine/Input
#include "keyboard.h"
#include "key_code.h"
#include "mouse.h"
#include "touch.h"

// Engine/Lua
#include "lua_stack.h"
#include "lua_environment.h"

// Engine/Os
#include "thread.h"
#include "mutex.h"
#include "os_file.h"
#include "os_window.h"
#include "socket.h"

// Engine/Renderers
#include "renderer.h"
#include "render_context.h"
#include "pixel_format.h"
#include "vertex_format.h"

// Engine/Resource
#include "bundle.h"
#include "resource_loader.h"
#include "resource_manager.h"
#include "texture_resource.h"
#include "mesh_resource.h"
#include "sound_resource.h"
#include "material_resource.h"
#include "font_resource.h"

// Engine/Audio
#include "sound_world.h"
