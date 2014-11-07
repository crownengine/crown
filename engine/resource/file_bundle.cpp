/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include <stdio.h>
#include <inttypes.h>

#include "memory.h"
#include "bundle.h"
#include "filesystem.h"

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
