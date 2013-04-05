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

#pragma once

// Check operating system (code borrowed from Qt4)
#if defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	// The operating system is Windows
	#define WINDOWS
#elif defined(__linux__) || defined(__linux)
	// The operating system is Linux
	#define LINUX
#else
	#error "Can't detect operating system"
#endif

#ifdef WINDOWS
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS	//To suppress safe clr function warnings (Es: strcpy -> strcpy_s
#endif
#endif

//#define CROWN_OS_ANDROID

// Debug mode
// #define DEBUG

/*
	Renderer settings.

	Here you choose which renderer to use for drawing
	things to screen. Uncomment your preferred renderer
	to choose it.

	Note that only _one_ renderer can be chosen at a time.

	Note also that:
	a) you can choose OpenGL if and only if you are on
	   a Linux or Windows machine.
	b) you can choose OpenGL|ES if and only if you are
	   on a Linux or Android machine.
*/
//#define CROWN_USE_OPENGL		//!< Whether to build with OpenGL
#define CROWN_USE_OPENGLES		//!< Whether to build with OpenGL|ES

//#define CROWN_USE_WINDOWING	//!< Whether to build with windowing

#define CROWN_USE_FLOAT

