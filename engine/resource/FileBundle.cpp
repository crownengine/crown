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

#include <stdio.h>
#include <inttypes.h>

#include "Allocator.h"
#include "Bundle.h"
#include "Filesystem.h"
#include "Resource.h"
#include "StringUtils.h" 
#include "Types.h"
#include "OS.h"

namespace crown
{

const uint32_t	RESOURCE_MAGIC_NUMBER		= 0xCE010101;
const uint32_t	RESOURCE_VERSION			= 2;

/// Contains the header data common to all
/// types of resources passing through the
/// standard Compiler mechanics.
struct ResourceHeader
{
	uint32_t	magic;		// Magic number used to identify the file
	uint32_t	version;	// Version of the compiler used to compile the resource
	uint32_t	size;		// Size of the resource data _not_ including this header in bytes
};

class FileBundle : public Bundle
{
public:


	//-----------------------------------------------------------------------------
	FileBundle(Filesystem& fs) : m_filesystem(fs) {}

	//-----------------------------------------------------------------------------
	File* open(ResourceId name)
	{
		// Convert name/type into strings
		char resource_name[512];
		snprintf(resource_name, 512, "%"PRIx64"", name.id);
		
		// Search the resource in the filesystem
		// bool exists = m_filesystem.exists(resource_name);
		// CE_ASSERT(exists == true, "Resource does not exist: %s", resource_name);

		// Open the resource and check magic number/version
		File* file = m_filesystem.open(resource_name, FOM_READ);

		CE_ASSERT(file != NULL, "Resource %s does not exist", resource_name);

		ResourceHeader header;
		file->read(&header, sizeof(ResourceHeader));

		CE_ASSERT(header.magic == RESOURCE_MAGIC_NUMBER, "Resource is not valid: %s", resource_name);
		CE_ASSERT(header.version == RESOURCE_VERSION, "Resource version mismatch: %s", resource_name);

		return file;
	}

	//-----------------------------------------------------------------------------
	void close(File* resource)
	{
		m_filesystem.close(resource);
	}

private:

	Filesystem& m_filesystem;
};

//-----------------------------------------------------------------------------
Bundle* Bundle::create(Allocator& a, Filesystem& fs)
{
	return CE_NEW(a, FileBundle)(fs);
}

//-----------------------------------------------------------------------------
void Bundle::destroy(Allocator& a, Bundle* bundle)
{
	CE_DELETE(a, bundle);
}

} // namespace crown
