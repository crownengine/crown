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

#include "BinaryReader.h"
#include "Stream.h"

namespace crown
{

//-----------------------------------------------------------------------------
BinaryReader::BinaryReader(Stream& stream) : m_stream(stream)
{
}

//-----------------------------------------------------------------------------
int8_t BinaryReader::read_byte()
{
	int8_t buffer;
	m_stream.read(&buffer, sizeof(int8_t));
	return buffer;
}

//-----------------------------------------------------------------------------
int16_t BinaryReader::read_int16()
{
	int16_t buffer;
	m_stream.read(&buffer, sizeof(int16_t));
	return buffer;
}

//-----------------------------------------------------------------------------
uint16_t BinaryReader::read_uint16()
{
	uint16_t buffer;
	m_stream.read(&buffer, sizeof(uint16_t));
	return buffer;
}

//-----------------------------------------------------------------------------
int32_t BinaryReader::read_int32()
{
	int32_t buffer;
	m_stream.read(&buffer, sizeof(int32_t));
	return buffer;
}

//-----------------------------------------------------------------------------
uint32_t BinaryReader::read_uint32()
{
	uint32_t buffer;
	m_stream.read(&buffer, sizeof(uint32_t));
	return buffer;
}

//-----------------------------------------------------------------------------
int64_t BinaryReader::read_int64()
{
	int64_t buffer;
	m_stream.read(&buffer, sizeof(int64_t));
	return buffer;
}

//-----------------------------------------------------------------------------
double BinaryReader::read_double()
{
	double buffer;
	m_stream.read(&buffer, sizeof(double));
	return buffer;
}

//-----------------------------------------------------------------------------
float BinaryReader::read_float()
{
	float buffer;
	m_stream.read(&buffer, sizeof(float));
	return buffer;
}

} // namespace crown
