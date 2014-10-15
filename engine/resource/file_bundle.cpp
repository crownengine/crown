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

#include "memory.h"
#include "bundle.h"
#include "filesystem.h"
#include "resource.h"
#include "string_utils.h"
#include "types.h"
#include "os.h"
#include "log.h"

namespace crown
{

class FileBundle : public Bundle
{
public:


	FileBundle(Filesystem& fs) : m_filesystem(fs) {}

	File* open(ResourceId name)
	{
		// Convert name/type into strings
		char resource_name[512];
		snprintf(resource_name, 512, "%.16"PRIx64"-%.16"PRIx64, name.type, name.name);

		// Open the resource and check magic number/version
		File* file = m_filesystem.open(resource_name, FOM_READ);

		CE_ASSERT(file != NULL, "Resource %s does not exist", resource_name);
		return file;
	}

	void close(File* resource)
	{
		m_filesystem.close(resource);
	}

private:

	Filesystem& m_filesystem;
};

Bundle* Bundle::create(Allocator& a, Filesystem& fs)
{
	return CE_NEW(a, FileBundle)(fs);
}

void Bundle::destroy(Allocator& a, Bundle* bundle)
{
	CE_DELETE(a, bundle);
}

} // namespace crown
