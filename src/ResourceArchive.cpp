#include "ResourceArchive.h"
#include <stdio.h>
#include "Filesystem.h"
#include "Resource.h"

namespace crown
{

ResourceArchive::ResourceArchive()
{
}

ResourceArchive::~ResourceArchive()
{
}

void ResourceArchive::open(const char* archive)
{
}

void ResourceArchive::close()
{
}

void ResourceArchive::find(ResourceId name)
{
	char ascii_name[16];
	snprintf(ascii_name, 16, "%x", name.name);
}

} // namespace crown

