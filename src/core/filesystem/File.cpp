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

#include "File.h"
#include "Types.h"
#include "Compressor.h"
#include "MallocAllocator.h"

namespace crown
{

//-----------------------------------------------------------------------------
bool File::compress_to(File& file, size_t size, size_t& zipped_size, Compressor& compressor)
{
	MallocAllocator allocator;
	void* in_buffer = (void*)allocator.allocate(size);

	read(in_buffer, size);

	void* compressed_buffer = compressor.compress(in_buffer, size, zipped_size);

	file.write(compressed_buffer, zipped_size);

	return true;
}

//-----------------------------------------------------------------------------
bool File::uncompress_to(File& file, size_t& unzipped_size, Compressor& compressor)
{
	MallocAllocator allocator;

	size_t file_size = size();
	void* in_buffer = (void*)allocator.allocate(file_size);

	read(in_buffer, file_size);

	void* uncompressed_buffer = compressor.uncompress(in_buffer, file_size, unzipped_size);

	file.write(uncompressed_buffer, unzipped_size);

	return true;
}

} // namespace crown

