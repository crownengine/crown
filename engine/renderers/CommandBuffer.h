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

#define MAX_COMMAND_BUFFER_SIZE 1024 * 1024

enum CommandType
{
	COMMAND_INIT_RENDERER,
	COMMAND_SHUTDOWN_RENDERER,

	COMMAND_CREATE_VERTEX_BUFFER,
	COMMAND_UPDATE_VERTEX_BUFFER,
	COMMAND_DESTROY_VERTEX_BUFFER,

	COMMAND_CREATE_INDEX_BUFFER,
	COMMAND_DESTROY_INDEX_BUFFER,

	COMMAND_CREATE_TEXTURE,
	COMMAND_UPDATE_TEXTURE,
	COMMAND_DESTROY_TEXTURE,

	COMMAND_CREATE_SHADER,
	COMMAND_DESTROY_SHADER,

	COMMAND_CREATE_GPU_PROGRAM,
	COMMAND_DESTROY_GPU_PROGRAM,

	COMMAND_CREATE_UNIFORM,
	COMMAND_DESTROY_UNIFORM,

	COMMAND_END
};

class CommandBuffer
{
public:

	//-----------------------------------------------------------------------------
	CommandBuffer() : m_size(0)
	{
		// Ensure at least a command is in queue
		commit();
	}

	//-----------------------------------------------------------------------------
	void clear()
	{
		m_size = 0;
	}

	//-----------------------------------------------------------------------------
	template<typename T>
	void write(const T type)
	{
		write((void*)&type, sizeof(T));
	}

	template<typename T>
	void read(T& data)
	{
		read((void*)&data, sizeof(T));
	}

	//-----------------------------------------------------------------------------
	void write(void* data, size_t size)
	{
		CE_ASSERT(m_size + size < MAX_COMMAND_BUFFER_SIZE, "CommandBuffer overflow");

		memcpy(&m_buffer[m_size], data, size);
		m_size += size;
	}

	//-----------------------------------------------------------------------------
	void read(void* data, size_t size)
	{
		CE_ASSERT(m_size + size < MAX_COMMAND_BUFFER_SIZE, "Reading beyond buffer");
		
		memcpy(data, &m_buffer[m_size], size);
		m_size += size;
	}

	//-----------------------------------------------------------------------------
	void commit()
	{
		write(COMMAND_END);
		m_size = 0;
	}

public:

	size_t	m_size;
	char	m_buffer[MAX_COMMAND_BUFFER_SIZE];
};

} // namespace crown
