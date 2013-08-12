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
#include "FileBundle.h"
#include "Filesystem.h"
#include "Resource.h"
#include "DiskFile.h"
#include "Log.h"
#include "StringUtils.h" 
#include "OS.h"

namespace crown
{

//-----------------------------------------------------------------------------
FileBundle::FileBundle(Filesystem& fs) :
	m_filesystem(fs)
{
}

//-----------------------------------------------------------------------------
FileBundle::~FileBundle()
{
}

//-----------------------------------------------------------------------------
DiskFile* FileBundle::open(ResourceId name)
{
	// Convert name/type into strings
	char resource_name[512];
	snprintf(resource_name, 512, "%.8X%.8X", name.name, name.type);

	Log::i("name: %s", resource_name);
	
	// Search the resource in the filesystem
	bool exists = m_filesystem.exists("disk", resource_name);
	CE_ASSERT(exists == true, "Resource does not exist: %s", resource_name);

	// Open the resource and check magic number/version
	DiskFile* file = (DiskFile*)m_filesystem.open("disk", resource_name, FOM_READ);

	ResourceHeader header;
	file->read(&header, sizeof(ResourceHeader));

	CE_ASSERT(header.magic == RESOURCE_MAGIC_NUMBER, "Resource is not valid: %s", resource_name);
	CE_ASSERT(header.version == RESOURCE_VERSION, "Resource version mismatch: %s", resource_name);

	return file;
}

//-----------------------------------------------------------------------------
void FileBundle::close(DiskFile* resource)
{
	m_filesystem.close(resource);
}

} // namespace crown
