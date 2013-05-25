/*
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

#include "PixelShaderResource.h"
#include "FileStream.h"
#include "ResourceArchive.h"
#include "Allocator.h"
#include "Device.h"
#include "Log.h"

namespace crown
{

//-----------------------------------------------------------------------------
void* PixelShaderResource::load(Allocator& allocator, ResourceArchive& archive, ResourceId id)
{
	FileStream* stream = archive.open(id);

	ce_assert(stream != NULL, "Resource does not exist: %.8X%.8X", id.name, id.type);

	PixelShaderResource* resource = (PixelShaderResource*)allocator.allocate(sizeof(PixelShaderResource));

	stream->read(&resource->m_program_text_length, sizeof(uint32_t));

	resource->m_program_text = (char*)allocator.allocate(sizeof(char) * (resource->m_program_text_length + 1));

	stream->read(resource->m_program_text, (size_t)resource->m_program_text_length);
	
	resource->m_program_text[resource->m_program_text_length] = '\0';

	archive.close(stream);

	return resource;
}

//-----------------------------------------------------------------------------
void PixelShaderResource::unload(Allocator& allocator, void* resource)
{
	ce_assert(resource != NULL, "Resource not loaded");

	((PixelShaderResource*)resource)->m_program_text_length = 0;

	allocator.deallocate(((PixelShaderResource*)resource)->m_program_text);
	allocator.deallocate(resource);
}

//-----------------------------------------------------------------------------
void PixelShaderResource::online(void* resource)
{
	Renderer* renderer = device()->renderer();
	PixelShaderResource* pixel_shader_resource = (PixelShaderResource*) resource;

	pixel_shader_resource->m_pixel_shader_id = renderer->create_pixel_shader(pixel_shader_resource->m_program_text);
}

//-----------------------------------------------------------------------------
void PixelShaderResource::offline()
{
}

} // namespace crown
