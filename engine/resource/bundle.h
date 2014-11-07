/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "resource_id.h"

namespace crown
{

class Filesystem;
class File;

class Bundle
{
public:

	static Bundle* create(Allocator& a, Filesystem& fs);
	static void destroy(Allocator& a, Bundle* bundle);

	virtual ~Bundle() {}

	/// Opens the resource file containing @a name resource
	/// and returns a stream from which read the data from.
	/// @note
	/// The resource stream points exactly at the start
	/// of the useful resource data, so you do not have to
	/// care about skipping headers, metadatas and so on.
	virtual File* open(ResourceId name) = 0;

	/// Closes the resource file.
	virtual void close(File* resource) = 0;
};

} // namespace crown

