#pragma once

#include "Types.h"
#include "Resource.h"

namespace crown
{

class ResourceArchive;

class TextResource
{
public:

	static TextResource*		load(ResourceArchive* archive, ResourceId id);
	static void					unload(TextResource* text);

	uint32_t					length;
	char*						data;
};

} // namespace crown

