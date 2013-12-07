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

#include "Types.h"
#include "List.h"
#include "SpriteResource.h"
#include "Compiler.h"
#include "OS.h"
#include "Vector2.h"
#include "JSONParser.h"

namespace crown
{

class Filesystem;

//-----------------------------------------------------------------------------
struct FrameData
{
	float x0, y0;
	float x1, y1;

	float scale_x, scale_y;
	float offset_x, offset_y;
};

//-----------------------------------------------------------------------------
class SpriteCompiler : public Compiler
{
public:
							SpriteCompiler();
							~SpriteCompiler();

	size_t					compile_impl(Filesystem& fs, const char* resource_path);
	void					write_impl(File* out_file);

	void					parse_frame(JSONElement frame);

private:

	List<StringId32>		m_names;
	List<FrameData> 		m_regions;
	List<float>				m_vertices;
};

} // namespace crown