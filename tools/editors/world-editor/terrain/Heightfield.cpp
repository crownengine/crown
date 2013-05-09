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

#include "Heightfield.h"

namespace crown
{

//-----------------------------------------------------------------------------
Heightfield::Heightfield() :
	m_initial_width(0),
	m_initial_height(0),
	m_initial_meters_per_tile(0),
	m_initial_altitude(0.0f),
	m_initial_min_altitude(0.0f),
	m_initial_max_altitude(0.0f),

	m_width(0),
	m_height(0),
	m_meters_per_tile(0),

	m_min_altitude(0.0f),
	m_max_altitude(0.0f),

	m_altitudes(NULL)
{
}

//-----------------------------------------------------------------------------
Heightfield::~Heightfield()
{
	if (m_altitudes != NULL)
	{
		m_allocator.deallocate(m_altitudes);
	}
}

//-----------------------------------------------------------------------------
void Heightfield::recreate(uint32_t width, uint32_t height, uint32_t meters_per_tile, float initial_altitude, float min_altitude, float max_altitude)
{
	// Recreate the heightfield if already existent
	if (m_altitudes != NULL)
	{
		m_allocator.deallocate(m_altitudes);
		m_altitudes = NULL;
	}

	m_initial_width = width;
	m_initial_height = height;
	m_initial_meters_per_tile = meters_per_tile;
	m_initial_altitude = initial_altitude;
	m_initial_min_altitude = min_altitude;
	m_initial_max_altitude = max_altitude;

	m_width = width;
	m_height = height;
	m_meters_per_tile = meters_per_tile;
	m_min_altitude = min_altitude;
	m_max_altitude = max_altitude;

	m_altitudes = (float*)m_allocator.allocate(width * height * sizeof(float));

	set_altitudes(m_initial_altitude);
}

//-----------------------------------------------------------------------------
void Heightfield::clear()
{
	m_width = m_initial_width;
	m_height = m_initial_height;
	m_meters_per_tile = m_initial_meters_per_tile;
	m_min_altitude = m_initial_min_altitude;
	m_max_altitude = m_initial_max_altitude;

	set_altitudes(m_initial_altitude);
}

//-----------------------------------------------------------------------------
uint32_t Heightfield::width() const
{
	return m_width;
}

//-----------------------------------------------------------------------------
uint32_t Heightfield::height() const
{
	return m_height;
}

//-----------------------------------------------------------------------------
float Heightfield::min_altitude() const
{
	return m_min_altitude;
}

//-----------------------------------------------------------------------------
float Heightfield::max_altitude() const
{
	return m_max_altitude;
}

//-----------------------------------------------------------------------------
void Heightfield::set_min_altitude(float min)
{
	m_min_altitude = min;
}

//-----------------------------------------------------------------------------
void Heightfield::set_max_altitude(float max)
{
	m_max_altitude = max;
}

//-----------------------------------------------------------------------------
float Heightfield::altitude(uint32_t x, uint32_t y) const
{
	assert(x < m_width);
	assert(y < m_height);

	return m_altitudes[coords_to_index(x, y)];
}

//-----------------------------------------------------------------------------
void Heightfield::set_altitude(uint32_t x, uint32_t y, float altitude)
{
	const uint32_t adjusted_altitude = (altitude < m_min_altitude) ? m_min_altitude : (altitude > m_max_altitude) ? m_max_altitude : altitude;

	m_altitudes[coords_to_index(x, y)] = adjusted_altitude;
}

//-----------------------------------------------------------------------------
void Heightfield::set_altitudes(float altitude)
{
	for (uint32_t w = 0; w < m_width; w++)
	{
		for (uint32_t h = 0; h < m_height; h++)
		{
			set_altitude(w, h, altitude);
		}
	}
}

//-----------------------------------------------------------------------------
uint32_t Heightfield::coords_to_index(uint32_t x, uint32_t y) const
{
	assert(x < m_width);
	assert(y < m_height);

	return m_width * y + x;
}

} // namespace crown

