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

#include "Material.h"
#include "MaterialResource.h"
#include "Renderer.h"
#include "Device.h"
#include "ResourceManager.h"
#include "TextureResource.h"

namespace crown
{

//-----------------------------------------------------------------------------
Material::Material(const MaterialResource* mr)
	: m_resource(mr)
{
}

//-----------------------------------------------------------------------------
Material::~Material()
{
}

//-----------------------------------------------------------------------------
const MaterialResource* Material::resource()
{
	return m_resource;
}

//-----------------------------------------------------------------------------
void Material::bind(Renderer& r, UniformId uniform)
{
	const ResourceId tr_id = m_resource->get_texture_layer(0);
	const TextureResource* tr = (TextureResource*) device()->resource_manager()->data(tr_id);

	r.set_texture(0, uniform, tr->texture(), TEXTURE_FILTER_LINEAR | TEXTURE_WRAP_U_CLAMP_REPEAT | TEXTURE_WRAP_V_CLAMP_REPEAT);
}

} // namespace crown
