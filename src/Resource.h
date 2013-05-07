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

#include "Types.h"
#include "Hash.h"
#include "String.h"

namespace crown
{

/// Hashed values for supported resource types
const char* const TEXTURE_EXTENSION		= "tga";
const char* const MESH_EXTENSION		= "dae";
const char* const SCRIPT_EXTENSION		= "lua";
const char* const TEXT_EXTENSION		= "txt";
const char* const MATERIAL_EXTENSION	= "material";

const uint32_t TEXTURE_TYPE		= hash::murmur2_32(TEXTURE_EXTENSION,  string::strlen(TEXTURE_EXTENSION),  0);
const uint32_t MESH_TYPE		= hash::murmur2_32(MESH_EXTENSION,     string::strlen(MESH_EXTENSION),     0);
const uint32_t SCRIPT_TYPE		= hash::murmur2_32(SCRIPT_EXTENSION,   string::strlen(SCRIPT_EXTENSION),   0);
const uint32_t TEXT_TYPE		= hash::murmur2_32(TEXT_EXTENSION,     string::strlen(TEXT_EXTENSION),     0);
const uint32_t MATERIAL_TYPE	= hash::murmur2_32(MATERIAL_EXTENSION, string::strlen(MATERIAL_EXTENSION), 0);

/// Enumerates the loading states of a resource
enum ResourceState
{
	RS_UNLOADED		= 0,		//< The resource is not loaded, so it cannot be used
	RS_LOADING		= 1,		//< The resource loader started to load the resource but it is yet not ready to use
	RS_LOADED		= 2			//< The resource loader finished to load the texture meaning it can be used
};

/// ResourceId uniquely identifies a resource by its name and type.
/// In order to speed up the lookup by the manager, it also keeps
/// the index to the resource list where it is stored.
struct ResourceId
{
	uint32_t		name;		// Hashed resource name
	uint32_t		type;		// Hashed resource type
	uint32_t		index;		// Index into the ResourceManager internal list

	bool			operator==(const ResourceId& b)
	{
		return name == b.name && type == b.type && index == b.index;
	}
};

} // namespace crown

