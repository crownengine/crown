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

#include "Assert.h"

namespace crown
{

#define MAX_CONSTANT_BUFFER_SIZE 1024 * 1024

enum UniformType
{
	UNIFORM_INTEGER_1,
	UNIFORM_INTEGER_2,
	UNIFORM_INTEGER_3,
	UNIFORM_INTEGER_4,
	UNIFORM_FLOAT_1,
	UNIFORM_FLOAT_2,
	UNIFORM_FLOAT_3,
	UNIFORM_FLOAT_4,
	UNIFORM_FLOAT_3_X_3,
	UNIFORM_FLOAT_4_X_4,
	UNIFORM_END
};

const size_t UNIFORM_SIZE_TABLE[UNIFORM_END] =
{
	sizeof(int32_t) * 1,
	sizeof(int32_t) * 2,
	sizeof(int32_t) * 3,
	sizeof(int32_t) * 4,
	sizeof(float) * 1,
	sizeof(float) * 2,
	sizeof(float) * 3,
	sizeof(float) * 4,
	sizeof(float) * 9,
	sizeof(float) * 16
};

typedef Id UniformId;

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

	void write_constant(UniformId id, UniformType type, void* data, uint8_t num)
	{
		size_t size = UNIFORM_SIZE_TABLE[type] * num;

		write(type);
		write(&id, sizeof(UniformId));
		write(&size, sizeof(size_t));
		write(data, size);
	}

	void commit()
	{
		write(UNIFORM_END);
		m_size = 0;
	}

private:

	size_t m_size;
	char m_buffer[MAX_CONSTANT_BUFFER_SIZE];
};

} // namespace crown
