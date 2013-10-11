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

#include "Sprite.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "TextureResource.h"
#include "Device.h"
#include "Renderer.h"

namespace crown
{

static float quad_vertices[] =
{
	-1.0f, -1.0f, 
	 0.0f, 0.0f, 1.0f,
	 1.0f, 1.0f, 1.0f, 1.0f,
	 0.0f, 0.0f,

	 1.0f, -1.0f,
	 0.0f, 0.0f, 1.0f,
	 1.0f, 1.0f, 1.0f, 1.0f,
	 1.0f, 0.0f,

	 1.0f,  1.0f,
	 0.0f, 0.0f, 1.0f,
	 1.0f, 1.0f, 1.0f, 1.0f,
	 1.0f, 1.0f,

	-1.0f,  1.0f,
	 0.0f, 0.0f, 1.0f,
	 1.0f, 1.0f, 1.0f, 1.0f,
	 0.0f, 1.0f
};

static uint16_t quad_indices[] =
{
	0, 1, 3,
	1, 2, 3
};

//-----------------------------------------------------------------------------
static const char* sprite_vs =
	"uniform mat4      	u_model;"
	"uniform mat4      	u_model_view_projection;"

	"in vec4           	a_position;"
	"in vec4           	a_normal;"
	"in vec2           	a_tex_coord0;"
	"in vec4           	a_color;"

	"varying out vec2	tex_coord0;"
	"varying out vec4	color;"

	"void main(void)"
	"{"
	"	tex_coord0 = a_tex_coord0;"
	"   color = a_color;"
	"	gl_Position = u_model_view_projection * a_position;"
	"}";

//-----------------------------------------------------------------------------
static const char* sprite_fs = 
	"in vec2            tex_coord0;"
	"in vec4            color;"

	"uniform sampler2D  u_tex;"

	"void main(void)"
	"{"
	"	gl_FragColor = texture(u_tex, tex_coord0) * color;"
	"}";

//-----------------------------------------------------------------------------
void Sprite::create(const TextureResource* tr, int32_t node, const Vector3& pos, const Quaternion& rot)
{
	Renderer* r = device()->renderer();

	m_texture = tr->m_texture;

	m_vb = r->create_vertex_buffer(4, VertexFormat::P2_N3_C4_T2, quad_vertices);
	m_ib = r->create_index_buffer(6, quad_indices);

	m_vertex = r->create_shader(ShaderType::VERTEX, sprite_vs);
	m_fragment = r->create_shader(ShaderType::FRAGMENT, sprite_fs);

	m_program = r->create_gpu_program(m_vertex, m_fragment);

	m_uniform = r->create_uniform("u_tex", UniformType::INTEGER_1, 1);


	set_local_position(pos);
	set_local_rotation(rot);

	m_node = node;
}

//-----------------------------------------------------------------------------
Vector3 Sprite::local_position() const
{
	Vector3 tmp = m_local_pose.translation();

	return tmp;
}

//-----------------------------------------------------------------------------
Quaternion Sprite::local_rotation() const
{
	Quaternion tmp = m_local_pose.to_quat();

	return tmp;
}

//-----------------------------------------------------------------------------
Matrix4x4 Sprite::local_pose() const
{
	return m_local_pose;
}

//-----------------------------------------------------------------------------
Vector3 Sprite::world_position() const
{
	Vector3 tmp = m_world_pose.translation();

	return tmp;
}

//-----------------------------------------------------------------------------
Quaternion Sprite::world_rotation() const
{
	Quaternion tmp = m_world_pose.to_quat();

	return tmp;
}

//-----------------------------------------------------------------------------
Matrix4x4 Sprite::world_pose() const
{
	return m_world_pose;	
}

//-----------------------------------------------------------------------------
void Sprite::set_local_position(const Vector3& pos)
{
	m_local_pose.set_translation(pos);
}

//-----------------------------------------------------------------------------
void Sprite::set_local_rotation(const Quaternion& rot)
{
	Matrix4x4& local_pose = m_local_pose;

	Vector3 local_translation = local_pose.translation();
	local_pose = rot.to_mat4();
	local_pose.set_translation(local_translation);	
}

//-----------------------------------------------------------------------------
void Sprite::set_local_pose(const Matrix4x4& pose)
{
	m_local_pose = pose;
}


} // namespace crown