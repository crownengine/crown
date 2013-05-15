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

#include "Types.h"

namespace crown
{

class Stream;

/// A reader that offers a convenient way to read text from a Stream
class TextReader
{
public:

						TextReader(Stream& s);

	/// Read a single character from the stream
	char				read_char();

	/// Reads characters from stream and stores them as a C string
	/// into string until (count-1) characters have been read or
	/// either a newline or the End-of-Stream is reached, whichever
	/// comes first.
	/// A newline character makes fgets stop reading, but it is considered
	/// a valid character and therefore it is included in the string copied to string.
	/// A null character is automatically appended in str after the characters read to
	/// signal the end of the C string.
	char*				read_string(char* string, uint32_t count);

private:

	Stream&				m_stream;
};

} // namespace crown

