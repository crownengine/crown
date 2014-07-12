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
#include "ResourceManager.h"
#include "TextureResource.h"

namespace crown
{

//-----------------------------------------------------------------------------
Material::Material(const MaterialResource* mr)
	: m_resource(mr)
{
	Blob vs_code = mr->get_vertex_shader();
	Blob fs_code = mr->get_fragment_shader();

	printf("vs: %d, %p\n", vs_code.m_size, (void*) vs_code.m_data);
	printf("fs: %d, %p\n", fs_code.m_size, (void*) fs_code.m_data);

	bgfx::ShaderHandle vs = bgfx::createShader(bgfx::makeRef((const void*) vs_code.m_data, vs_code.m_size));
	bgfx::ShaderHandle fs = bgfx::createShader(bgfx::makeRef((const void*) fs_code.m_data, fs_code.m_size));

	m_program = bgfx::createProgram(vs, fs);
	m_uniform = bgfx::createUniform("u_albedo", bgfx::UniformType::Uniform1iv);
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
void Material::bind()
{
	const void* tr = device()->resource_manager()->get("texture", "spaceship");

	bgfx::TextureHandle th;
	th.idx = (uintptr_t) tr;

	bgfx::setTexture(0, m_uniform, th);
	bgfx::setProgram(m_program);
}

} // namespace crown
