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

#include "TextResource.h"
#include "DiskFile.h"
#include "Bundle.h"
#include "Log.h"
#include "Allocator.h"

namespace crown
{

//-----------------------------------------------------------------------------
void* TextResource::load(Allocator& allocator, Bundle& bundle, ResourceId id)
{
	DiskFile* stream = bundle.open(id);

	CE_ASSERT(stream != NULL, "Resource does not exist: %.8X%.8X", id.name, id.type);

	TextResource* resource = (TextResource*)allocator.allocate(sizeof(TextResource));

	stream->read(&resource->length, sizeof(uint32_t));
	
	resource->data = (char*)allocator.allocate(sizeof(char) * (resource->length + 1));

	stream->read(resource->data, (size_t)resource->length);
	
	resource->data[resource->length] = '\0';

	bundle.close(stream);

	return resource;
}

//-----------------------------------------------------------------------------
void TextResource::unload(Allocator& allocator, void* resource)
{
	CE_ASSERT(resource != NULL, "Resource not loaded");

	((TextResource*)resource)->length = 0;

	allocator.deallocate(((TextResource*)resource)->data);
	allocator.deallocate(resource);
}

} // namespace crown
