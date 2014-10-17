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

#pragma once

#include "resource.h"

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

