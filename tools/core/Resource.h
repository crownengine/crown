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

namespace crown
{

/// Hashed values for supported resource types
const char* const TEXTURE_EXTENSION			= "tga";
const char* const MESH_EXTENSION			= "dae";
const char* const SCRIPT_EXTENSION			= "lua";
const char* const TEXT_EXTENSION			= "txt";
const char* const MATERIAL_EXTENSION		= "material";
const char* const VERTEX_SHADER_EXTENSION	= "vs";
const char* const PIXEL_SHADER_EXTENSION	= "ps";

const uint32_t TEXTURE_TYPE					= 0x1410A16A;
const uint32_t MESH_TYPE					= 0xE8239EEC;
const uint32_t SCRIPT_TYPE					= 0xD96E7C37;
const uint32_t TEXT_TYPE					= 0x9000BF0B;
const uint32_t MATERIAL_TYPE				= 0x46807A92;
const uint32_t VERTEX_SHADER_TYPE			= 0xDC7F061F;
const uint32_t PIXEL_SHADER_TYPE			= 0x2A461B45;

} // namespace crown
