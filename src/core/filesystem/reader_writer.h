/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "file.h"
#include "string_utils.h"

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
	uint32_t read_string(char* string, uint32_t size)
	{
		char current_char;
		uint32_t bytes_read = 0;

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
		_file.write(string, strlen32(string));
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

	void write(const void* data, uint32_t size)
	{
		_file.write(data, size);
	}

	template <typename T>
	void write(const T& data)
	{
		_file.write(&data, sizeof(T));
	}

	void skip(uint32_t bytes)
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

	void read(void* data, uint32_t size)
	{
		_file.read(data, size);
	}

	template <typename T>
	void read(T& data)
	{
		_file.read(&data, sizeof(T));
	}

	void skip(uint32_t bytes)
	{
		_file.skip(bytes);
	}

private:

	File& _file;
};

} // namespace crown
