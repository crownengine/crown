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

#include "BinaryWriter.h"
#include "Stream.h"

namespace crown
{

//-----------------------------------------------------------------------------
BinaryWriter::BinaryWriter(Stream& stream) : m_stream(stream)
{
}

//-----------------------------------------------------------------------------
void BinaryWriter::write_byte(int8_t buffer)
{
	m_stream.write(&buffer, sizeof(int8_t));
}

//-----------------------------------------------------------------------------
void BinaryWriter::write_int16(int16_t buffer)
{
	m_stream.write(&buffer, sizeof(int16_t));
}

//-----------------------------------------------------------------------------
void BinaryWriter::write_uint16(uint16_t buffer)
{
	m_stream.write(&buffer, sizeof(uint16_t));
}

//-----------------------------------------------------------------------------
void BinaryWriter::write_int32(int32_t buffer)
{
	m_stream.write(&buffer, sizeof(int32_t));
}

//-----------------------------------------------------------------------------
void BinaryWriter::write_uint32(uint32_t buffer)
{
	m_stream.write(&buffer, sizeof(uint32_t));
}

//-----------------------------------------------------------------------------
void BinaryWriter::write_int64(int64_t buffer)
{
	m_stream.write(&buffer, sizeof(int64_t));
}

//-----------------------------------------------------------------------------
void BinaryWriter::write_double(double buffer)
{
	m_stream.write(&buffer, sizeof(double));
}

//-----------------------------------------------------------------------------
void BinaryWriter::write_float(float buffer)
{
	m_stream.write(&buffer, sizeof(float));
}

//-----------------------------------------------------------------------------
void BinaryWriter::insert_byte(int8_t val, size_t offset)
{
	size_t tmpSize = m_stream.size() - offset;
	int8_t* tmp = new int8_t[tmpSize];

	m_stream.seek(offset);
	m_stream.read(tmp, tmpSize);

	m_stream.seek(offset);

	m_stream.write_byte(val);

	m_stream.write(tmp, tmpSize);

	delete[] tmp;
}

} // namespace crown

