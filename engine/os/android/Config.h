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

#define CROWN_VERSION_MAJOR 0 
#define CROWN_VERSION_MINOR 1 
#define CROWN_VERSION_MICRO 11

#define CROWN_DEVELOPMENT

#ifdef ANDROID
	#define PRId64 "lld"
	#define PRIu64 "llu"
	#define PRIi64 "lli"
	#define PRIx64 "llx"
#endif

#define CROWN_MAX_TEXTURE_UNITS		8
#define CROWN_MAX_TEXTURES			32
#define CROWN_MAX_RENDER_TARGETS	32
#define CROWN_MAX_VERTEX_BUFFERS	1024
#define CROWN_MAX_INDEX_BUFFERS		1024
#define CROWN_MAX_SHADERS			1024
#define CROWN_MAX_GPU_PROGRAMS		1024
#define CROWN_MAX_UNIFORMS			128

// Including NUL character
#define CROWN_MAX_UNIFORM_NAME_LENGTH	64
