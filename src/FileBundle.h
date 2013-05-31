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
#include "Bundle.h"

namespace crown
{

class Filesystem;
class DiskFile;

// The header of every compiled resource file.
// KEEP IN SYNC WITH CompiledResource struct in Compiler.h!
struct ResourceHeader
{
	uint32_t	magic;		// Magic number used to identify the file
	uint32_t	version;	// Version of the compiler used to compile the resource
	uint32_t	name;		// Name of the resource (murmur2_32 hash)
	uint32_t	type;		// Type of the resource (murmur2_32 hash)
	uint32_t	size;		// Size of the resource data _not_ including header (in bytes)
};

/// Source of resources
class FileBundle : public Bundle
{
public:

					FileBundle(Filesystem& fs);
					~FileBundle();

	/// @copydoc Bundle::open()
	DiskFile*		open(ResourceId name);

	/// @copydoc Bundle::close()
	void			close(DiskFile* resource);


private:

	Filesystem&		m_filesystem;
};

} // namespace crown
