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

#pragma once

#include "Types.h"

namespace crown
{

enum StreamOpenMode
{
	SOM_READ		= 1,
	SOM_WRITE		= 2
};

class Compressor;

/// An abstraction to access data streams.
/// 
/// It represents a flow of data attached to a 'file' which can be an archived file,
/// a regular file, a location in memory or anything that can be read or wrote.
/// A Stream is an abstraction to interact with these in an uniform way; every stream
/// comes with a convenient set of methods to facilitate reading from it, writing to
/// it and so on.
class Stream
{
public:

						/// Constructor
						Stream(StreamOpenMode mode) : m_open_mode(mode) {}

						/// Destructor
	virtual				~Stream() {};

						/// Sets the position indicator of the stream to position.
	virtual void		seek(size_t position) = 0;

						/// Sets the position indicator to the end of the stream
	virtual void		seek_to_end() = 0;

						/// Sets the position indicator to bytes after current position
	virtual void		skip(size_t bytes) = 0;

						/// Reads a byte from the stream starting at current position.
	virtual uint8_t		read_byte() = 0;

						/// Reads a block of data from the stream.
	virtual void		read(void* buffer, size_t size) = 0;

						/// Writes a byte to the stream starting at current position.
	virtual void		write_byte(uint8_t val) = 0;

						/// Writes a block of data to the stream.
	virtual void		write(const void* buffer, size_t size) = 0;

						/// Copies a chunk of 'size' bytes of data from this to another stream.
	virtual bool		copy_to(Stream* stream, size_t size = 0) = 0;

						/// Zips a chunk of 'size' bytes of data from this to another stream using compressor.
	virtual bool		compress_to(Stream* stream, size_t size, size_t& compressed_size, Compressor* compressor);

						/// Unzip a zipped stream of data from this to another stream using compressor.
	virtual bool		uncompress_to(Stream* stream, size_t& uncompressed_size, Compressor* compressor);

						/// Forces the previouses write operations to complete.
						/// Generally, when a Stream is attached to a file,
						/// write operations are not performed instantly, the output data
						/// may be stored to a temporary buffer before making its way to
						/// the file. This method forces all the pending output operations
						/// to be written to the stream.
	virtual void		flush() = 0;

						/// Returns whether the stream is valid.
						/// A stream is valid when the buffer where it operates
						/// exists. (i.e. a file descriptor is attached to the stream, 
						/// a memory area is attached to the stream etc.)
	virtual bool		is_valid() const = 0;

						/// Returns whether the position is at end of stream.
	virtual bool		end_of_stream() const = 0;

						/// Returns the size of stream in bytes.
	virtual size_t		size() const = 0;

						/// Returns the current position in stream.
						/// Generally, for binary data, it means the number of bytes
						/// from the beginning of the stream.
	virtual size_t		position() const = 0;

						/// Returns whether the stream can be read.
	virtual bool		can_read() const = 0;

						/// Returns whether the stream can be wrote.
	virtual bool		can_write() const = 0;

						/// Returns whether the stream can be sought.
	virtual bool		can_seek() const = 0;

protected:

	StreamOpenMode		m_open_mode;
};

///! A reader that offers a convenient way to read from a stream
class BinaryReader
{

public:

						BinaryReader(Stream* s);
	virtual				~BinaryReader();

	int8_t				read_byte();
	int16_t				read_int16();
	uint16_t			read_uint16();
	int32_t				read_int32();
	uint32_t			read_uint32();
	int64_t				read_int64();
	double				read_double();
	float				read_float();

	inline Stream*		get_stream() { return m_stream; }
	inline void			set_stream(Stream* stream) { m_stream = stream; }

private:

	Stream*				m_stream;
};

///! A writer that offers a convenient way to write to a stream
class BinaryWriter
{

public:

						BinaryWriter(Stream* s);
	virtual				~BinaryWriter();

	void				write_byte(int8_t);
	void				write_int16(int16_t);
	void				write_uint16(uint16_t);
	void				write_int32(int32_t);
	void				write_uint32(uint32_t);
	void				write_int64(int64_t);
	void				write_double(double);
	void				write_float(float);

	void				insert_byte(int8_t val, size_t offset);

	inline Stream*		get_stream() { return m_stream; }
	inline void			set_stream(Stream* stream) { m_stream = stream; }

private:

	Stream*				m_stream;
};

///! A reader that offers a convenient way to read text to a stream
class TextReader
{

public:

						TextReader(Stream* s);
	virtual				~TextReader();

	char				read_char();
						/**
							Reads characters from stream and stores them as a C string
							into string until (count-1) characters have been read or
							either a newline or the End-of-Stream is reached, whichever
							comes first.
							A newline character makes fgets stop reading, but it is considered
							a valid character and therefore it is included in the string copied to string.
							A null character is automatically appended in str after the characters read to
							signal the end of the C string.
						*/
	char*				read_string(char* string, uint32_t count);

	inline Stream*		get_stream() { return m_stream; }
	inline void			set_stream(Stream* stream) { m_stream = stream; }

private:

	Stream*				m_stream;
};

///! A reader that offers a convenient way to write text to a stream
class TextWriter
{

public:

						TextWriter(Stream* s);
	virtual				~TextWriter();

	void				write_char(char c);
						/**
							Writes the string point32_ted by string to the stream.
							The function begins copying from the address specified (string)
							until it reaches the terminating null character ('\0').
							This final null character is not copied to the stream.
						*/
	void				write_string(const char* string);

	inline Stream*		get_stream() { return m_stream; }
	inline void			set_stream(Stream* stream) { m_stream = stream; }

private:

	Stream*				m_stream;
};

} /// namespace crown

