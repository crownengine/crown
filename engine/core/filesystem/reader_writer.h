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

#include "types.h"
#include "file.h"

namespace crown
{

/// A reader that offers a convenient way to read text from a File
///
/// @ingroup Filesystem
class TextReader
{
public:

	TextReader(File& file) : _file(file) {}

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

		while(!_file.end_of_file() && bytes_read < size - 1)
		{
			_file.read(&current_char, 1);
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

	File& _file;
};

/// A reader that offers a convenient way to write text to a File
///
/// @ingroup Filesystem
class TextWriter
{
public:

	TextWriter(File& file) : _file(file) {}

	/// Writes the string pointed by string to the file.
	/// The function begins copying from the address specified (string)
	/// until it reaches the terminating null character ('\0').
	/// The final null character is not copied to the file.
	void write_string(const char* string)
	{
		_file.write(string, string::strlen(string));
	}

private:

	File& _file;
};

/// A writer that offers a convenient way to write to a File
///
/// @ingroup Filesystem
class BinaryWriter
{
public:

	BinaryWriter(File& file) : _file(file) {}

	void write(const void* data, size_t size)
	{
		_file.write(data, size);
	}

	template <typename T>
	void write(const T& data)
	{
		_file.write(&data, sizeof(T));
	}

	void skip(size_t bytes)
	{
		_file.skip(bytes);
	}

private:

	File& _file;
};

/// A reader that offers a convenient way to read from a File
///
/// @ingroup Filesystem
class BinaryReader
{
public:

	BinaryReader(File& file) : _file(file) {}

	void read(void* data, size_t size)
	{
		_file.read(data, size);
	}

	template <typename T>
	void read(T& data)
	{
		_file.read(&data, sizeof(T));
	}

	void skip(size_t bytes)
	{
		_file.skip(bytes);
	}

private:

	File& _file;
};

} // namespace crown
