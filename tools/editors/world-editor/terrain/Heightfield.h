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
#include "HeapAllocator.h"

namespace crown
{

/// Represents a heightfiled.
class Heightfield
{
public:

				Heightfield();
				~Heightfield();

	/// (Re)Creates the heightfield of sizes @a width by @a height.
	/// The param @a meters_per_tile indicates how many meters should be mapped per tile unit. (Higher values
	///	means better quality at same sizes. You can also specify an @a initial_height for all the tiles of the heightmap.
	void		recreate(uint32_t width, uint32_t height, uint32_t meters_per_tile, float initial_altitude, float min_altitude, float max_altitude);

	/// Clears the content of the heightfield, switching it to the initial parameters passed to @a recreate().
	void		clear();

	uint32_t	width() const;
	uint32_t	height() const;

	float		min_altitude() const;
	float		max_altitude() const;

	void		set_min_altitude(float min);
	void		set_max_altitude(float max);

	/// Returns the altitude value for the tile at @a x and @a y coordinates.
	float		altitude(uint32_t x, uint32_t y) const;

	/// Sets the @a height value for the tile @a x and @a y coordinates.
	void		set_altitude(uint32_t x, uint32_t y, float altitude);

	/// Sets the @a height value for all the tiles.
	void		set_altitudes(float altitude);

private:

	uint32_t	coords_to_index(uint32_t x, uint32_t y) const;

private:

	HeapAllocator	m_allocator;

	uint32_t		m_initial_width;
	uint32_t		m_initial_height;
	uint32_t		m_initial_meters_per_tile;
	float			m_initial_altitude;
	float			m_initial_min_altitude;
	float			m_initial_max_altitude;

	uint32_t		m_width;
	uint32_t		m_height;
	uint32_t		m_meters_per_tile;
	float			m_min_altitude;
	float			m_max_altitude;

	// A buffer of heights of width by height.
	float*			m_altitudes;
};

} // namespace crown

