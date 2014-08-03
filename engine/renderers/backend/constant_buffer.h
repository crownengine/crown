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

#include "assert.h"
#include "renderer_types.h"

namespace crown
{

extern const size_t UNIFORM_SIZE_TABLE[UniformType::END];

#define MAX_CONSTANT_BUFFER_SIZE 1024 * 1024

class ConstantBuffer
{
public:

	ConstantBuffer()
		: m_size(0)
	{
		commit();
	}

	void clear()
	{
		m_size = 0;
	}

	uint32_t position() const
	{
		return m_size;
	}

	void reset(uint32_t begin)
	{
		m_size = begin;
	}

	void write(uint32_t data)
	{
		write(&data, sizeof(uint32_t));
	}

	uint32_t read()
	{
		uint32_t data;
		read(&data, sizeof(uint32_t));
		return data;
	}

	const void* read(size_t size)
	{
		CE_ASSERT(m_size + size < MAX_CONSTANT_BUFFER_SIZE, "Constant buffer overflow");

		const void* data = (void*) &m_buffer[m_size];
		m_size += size;

		return data;
	}

	void write(const void* data, size_t size)
	{
		CE_ASSERT(m_size + size < MAX_CONSTANT_BUFFER_SIZE, "Constant buffer overflow");

		memcpy(&m_buffer[m_size], data, size);
		m_size += size;
	}

	void read(void* data, size_t size)
	{
		CE_ASSERT(m_size + size < MAX_CONSTANT_BUFFER_SIZE, "Constant buffer overflow");

		memcpy(data, &m_buffer[m_size], size);
		m_size += size;
	}

	void write_constant(UniformId id, UniformType::Enum type, const void* data, uint8_t num)
	{
		const uint32_t size = UNIFORM_SIZE_TABLE[type] * num;

		write(type);
		write(&id, sizeof(UniformId));
		write(&size, sizeof(uint32_t));
		write(data, size);
	}

	void commit()
	{
		write(UniformType::END);
		m_size = 0;
	}

private:

	size_t m_size;
	char m_buffer[MAX_CONSTANT_BUFFER_SIZE];
};

} // namespace crown
