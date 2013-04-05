#pragma once

#include "Types.h"
#include "Resource.h"

namespace crown
{

class ResourceArchive;
class Allocator;

class TextResource
{
public:

	static TextResource*		load(Allocator& allocator, ResourceArchive* archive, ResourceId id);
	static void					unload(Allocator& allocator, TextResource* text);

	uint32_t					length;
	char*						data;
};

} // namespace crown
