#pragma once

#include "Resource.h"

namespace crown
{

/// Source of resources
class ResourceArchive
{
public:

						ResourceArchive();
						~ResourceArchive();

	void				open(const char* archive);
	void				close();

	void				find(ResourceId name);
	
private:
};

} // namespace crown

