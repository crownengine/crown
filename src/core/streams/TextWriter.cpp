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

#include "TextWriter.h"
#include "Stream.h"

namespace crown
{

//-----------------------------------------------------------------------------
TextWriter::TextWriter(Stream* stream) : m_stream(stream)
{
}

//-----------------------------------------------------------------------------
TextWriter::~TextWriter()
{
}

void TextWriter::write_char(char c)
{
	m_stream->write_byte(c);
}

//-----------------------------------------------------------------------------
void TextWriter::write_string(const char* string)
{
	size_t count = 0;

	while(string[count] != '\0')
	{
		m_stream->write_byte(string[count]);
		count++;
	}
}

} // namespace crown

