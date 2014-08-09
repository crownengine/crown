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

#define NOMINMAX
#include <cstddef>
#include <stdint.h>
#include "config.h"

typedef uint32_t StringId32;
typedef uint64_t StringId64;

#define INVALID_ID 65535

struct Id
{
	uint16_t id;
	uint16_t index;

	void decode(uint32_t id_and_index)
	{
		id = (id_and_index & 0xFFFF0000) >> 16;
		index = id_and_index & 0xFFFF;
	}

	uint32_t encode() const
	{
		return (uint32_t(id) << 16) | uint32_t(index);
	}

	bool operator==(const Id& other)
	{
		return id == other.id && index == other.index;
	}

	bool operator!=(const Id& other)
	{
		return id != other.id || index != other.index;
	}
};
