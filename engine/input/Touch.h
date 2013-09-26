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
#include "Vec2.h"

namespace crown
{

const uint32_t MAX_POINTER_IDS = 4;

struct PointerData
{
	uint32_t	x;
	uint32_t	y;
	float		relative_x;
	float		relative_y;
	bool		up;
};

/// Interface for accessing touch input device.
struct Touch
{
	/// Returns whether the touch pointer @a id is up.
	bool touch_up(uint16_t id) const
	{
		return m_pointers[id].up == true;
	}

	/// Returns whether the touch pointer @a id is down.
	bool touch_down(uint16_t id) const
	{
		return m_pointers[id].up == false;
	}

	/// Returns the position of the pointer @a id in windows space.
	/// @note
	/// Coordinates in window space have the origin at the
	/// upper-left corner of the window. +X extends from left
	/// to right and +Y extends from top to bottom.
	Vec2 touch_xy(uint16_t id) const
	{
		const PointerData& data = m_pointers[id];

		return Vec2(data.x, data.y);
	}

	/// Returns the relative position of the pointer @a id in window space.
	/// @note
	/// Coordinates in window space have the origin at the
	/// upper-left corner of the window. +X extends from left
	/// to right and +Y extends from top to bottom.
	/// @note
	/// Relative coordinates are mapped to a float varying
	/// from 0.0 to 1.0 where 0.0 is the origin and 1.0 the
	/// maximum extent of the cosidered axis.
	Vec2 touch_relative_xy(uint16_t id)
	{
		const PointerData& data = m_pointers[id];

		return Vec2(data.relative_x, data.relative_y);
	}

public:

	PointerData		m_pointers[MAX_POINTER_IDS];

	friend class	Device;
};

}

