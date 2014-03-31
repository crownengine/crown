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

#include "Types.h"

namespace crown
{

class File;

/// A writer that offers a convenient way to write to a File
///
/// @ingroup Filesystem
class BinaryWriter
{
public:

	//-----------------------------------------------------------------------------
	BinaryWriter(File& file) : m_file(file) {}

	//-----------------------------------------------------------------------------
	void write_byte(int8_t buffer)
	{
		m_file.write(&buffer, sizeof(int8_t));
	}

	//-----------------------------------------------------------------------------
	void write_int16(int16_t buffer)
	{
		m_file.write(&buffer, sizeof(int16_t));
	}

	//-----------------------------------------------------------------------------
	void write_uint16(uint16_t buffer)
	{
		m_file.write(&buffer, sizeof(uint16_t));
	}

	//-----------------------------------------------------------------------------
	void write_int32(int32_t buffer)
	{
		m_file.write(&buffer, sizeof(int32_t));
	}

	//-----------------------------------------------------------------------------
	void write_uint32(uint32_t buffer)
	{
		m_file.write(&buffer, sizeof(uint32_t));
	}

	//-----------------------------------------------------------------------------
	void write_int64(int64_t buffer)
	{
		m_file.write(&buffer, sizeof(int64_t));
	}

	//-----------------------------------------------------------------------------
	void write_double(double buffer)
	{
		m_file.write(&buffer, sizeof(double));
	}

	//-----------------------------------------------------------------------------
	void write_float(float buffer)
	{
		m_file.write(&buffer, sizeof(float));
	}

private:

	File& m_file;
};

} // namespace crown

