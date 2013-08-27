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

/// A reader that offers a convenient way to read text from a File
class TextReader
{
public:

	//-----------------------------------------------------------------------------
	TextReader(File& file) : m_file(file)
	{
	}

	/// Reads characters from file and stores them as a C string
	/// into string until (size-1) characters have been read or
	/// either a newline or the End-of-File is reached, whichever
	/// comes first.
	/// A newline character makes fgets stop reading, but it is considered
	/// a valid character and therefore it is included in the string copied to string.
	/// A null character is automatically appended in str after the characters read to
	/// signal the end of the C string.
	size_t read_string(char* string, size_t size)
	{
		char current_char;
		size_t bytes_read = 0;

		while(!m_file.end_of_file() && bytes_read < size - 1)
		{
			m_file.read(&current_char, 1);
			string[bytes_read] = current_char;

			bytes_read++;

			if (current_char == '\n')
			{
				break;
			}
		}

		string[bytes_read] = '\0';

		return bytes_read;
	}

private:

	File& m_file;
};

} // namespace crown

