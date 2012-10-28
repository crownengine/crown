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
#include "MathUtils.h"

namespace Crown
{

enum SeekMode
{
	SM_SeekFromBegin	= 0,
	SM_SeekFromCurrent	= 1,
	SM_SeekFromEnd		= 2
};

enum StreamOpenMode
{
	SOM_READ	= 1,
	SOM_WRITE	= 2
};

/**
	An abstraction to access data streams.

	It represents a flow of data attached to a 'file' which can be an archived file,
	a regular file, a location in memory or anything that can be read or wrote.
	A Stream is an abstraction to interact with these in an uniform way; every stream
	comes with a convenient set of methods to facilitate reading from it, writing to
	it and so on.
*/
class Stream
{

public:

						/**
							Constructor.
						@param openMode
							Whether to open for writing or reading
						*/
						Stream(StreamOpenMode openMode) : mOpenMode(openMode) {}
						/**
							Destructor.
						*/
	virtual				~Stream() {};
						/**
							Sets the position indicator of the stream to a new position.
						@param newPos
							The new position
						@param mode
							Position from where to move
						*/
	virtual void		Seek(int newPos, SeekMode mode) = 0;
						/**
							Reads a byte from the stream starting at current position.
						@return
							The byte read
						*/
	virtual uchar		ReadByte() = 0;
						/**
							Reads a block of data from the stream.
						@param buffer
							Pointer to a block of memory of size 'size'
						@param size
							The number of bytes to read
						*/
	virtual void		ReadDataBlock(void* buffer, size_t size) = 0;
						/**
							Copies a chunk of 'size' bytes of data from this to another stream.
						@param stream
							The output stream
						@param size
							The number of bytes to copy
						@return
							True if success, false otherwise
						*/
	virtual bool		CopyTo(Stream* stream, size_t size = 0) = 0;
						/**
							Zips a chunk of 'size' bytes of data from this to another stream. A default implementation is given
						@param stream
							The output stream
						@param size
							The number of bytes to zip
						@return
							True if success, false otherwise
						*/
	virtual bool		ZipTo(Stream* stream, size_t size, size_t& zippedSize);
						/**
							Unzip a zipped stream of data from this to another stream. A default implementation is given
						@param stream
							The output stream
						@return
							True if success, false otherwise
						*/
	virtual bool		UnzipTo(Stream* stream, size_t& unzippedSize);
						/**
							Writes a byte to the stream starting at current position.
						@param val
							The byte to write
						*/
	virtual void		WriteByte(uchar val) = 0;
						/**
							Writes a block of data to the stream.
						@param buffer
							Pointer to a block of memory of size 'size'
						@param size
							The number of bytes to write
						*/
	virtual void		WriteDataBlock(const void* buffer, size_t size) = 0;
						/**
							Forces the previouses write operations to complete.
							Generally, when a Stream is attached to a file,
							write operations are not performed instantly, the output data
							may be stored to a temporary buffer before making its way to
							the file. This method forces all the pending output operations
							to be written to the stream.
						*/
	virtual void		Flush() = 0;
						/**
							Returns whether the stream is valid.
							A stream is valid when the buffer where it operates
							exists. (i.e. a file descriptor is attached to the stream, 
							a memory area is attached to the stream etc.)
						@return
							True if valid, false otherwise
						*/
	virtual bool		IsValid() const = 0;
						/**
							Returns whether the position is at end of stream.
						@return
							True if at end of stream, false otherwise
						*/
	virtual bool		EndOfStream() const = 0;
						/**
							Returns the size of stream in bytes.
						@return
							The size in bytes
						*/
	virtual size_t		GetSize() const = 0;
						/**
							Returns the current position in stream.
							Generally, for binary data, it means the number of bytes
							from the beginning of the stream.
						@return
							The current position
						*/
	virtual size_t		GetPosition() const = 0;
						/**
							Returns whether the stream can be read.
						@return
							True if readable, false otherwise
						*/
	virtual inline bool	CanRead() const { return Math::TestBitmask(mOpenMode, SOM_READ); }
						/**
							Returns whether the stream can be wrote.
						@return
							True if writeable, false otherwise
						*/
	virtual inline bool	CanWrite() const { return Math::TestBitmask(mOpenMode, SOM_WRITE); }
						/**
							Returns whether the stream can be sought.
						@returns
							True is seekable, false otherwise
						*/
	virtual bool		CanSeek() const = 0;

protected:

	StreamOpenMode		mOpenMode;
};

//! A reader that offers a convenient way to read from a stream
class BinaryReader
{

public:

						BinaryReader(Stream* s);
	virtual				~BinaryReader();

	uchar				ReadByte();
	short				ReadInt16();
	ushort				ReadUint16();
	int					ReadInt32();
	uint				ReadUint32();
	long long			ReadInt64();
	double				ReadDouble();
	float				ReadFloat();

	inline Stream*		GetStream() { return mStream; }
	inline void			SetStream(Stream* stream) { mStream = stream; }

private:

	Stream*				mStream;
};

//! A writer that offers a convenient way to write to a stream
class BinaryWriter
{

public:

						BinaryWriter(Stream* s);
	virtual				~BinaryWriter();

	void				WriteByte(uchar);
	void				WriteInt16(short);
	void				WriteUint16(ushort);
	void				WriteInt32(int);
	void				WriteUint32(uint);
	void				WriteInt64(long long);
	void				WriteDouble(double);
	void				WriteFloat(float);

	void				InsertByte(uchar val, size_t offset);

	inline Stream*		GetStream() { return mStream; }
	inline void			SetStream(Stream* stream) { mStream = stream; }

private:

	Stream*				mStream;
};

//! A reader that offers a convenient way to read text to a stream
class TextReader
{

public:

						TextReader(Stream* s);
	virtual				~TextReader();

	char				ReadChar();
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
	char*				ReadString(char* string, int count);

	inline Stream*		GetStream() { return mStream; }
	inline void			SetStream(Stream* stream) { mStream = stream; }

private:

	Stream*				mStream;
};

//! A reader that offers a convenient way to write text to a stream
class TextWriter
{

public:

						TextWriter(Stream* s);
	virtual				~TextWriter();

	void				WriteChar(char c);
						/**
							Writes the string pointed by string to the stream.
							The function begins copying from the address specified (string)
							until it reaches the terminating null character ('\0').
							This final null character is not copied to the stream.
						*/
	void				WriteString(const char* string);

	inline Stream*		GetStream() { return mStream; }
	inline void			SetStream(Stream* stream) { mStream = stream; }

private:

	Stream*				mStream;
};

} // namespace Crown

