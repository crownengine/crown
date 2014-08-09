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

#include <cstdlib>
#include <cstdio>
#include "config.h"
#include "error.h"

#if defined(CROWN_DEBUG)
	#define CE_ASSERT(condition, msg, ...) do { if (!(condition)) {\
		crown::error::abort(__FILE__, __LINE__, "\nAssertion failed: %s\n\t" msg "\n", #condition, ##__VA_ARGS__); }} while (0)
#else
	#define CE_ASSERT(...) ((void)0)
#endif

#define CE_ASSERT_NOT_NULL(x) CE_ASSERT(x != NULL, #x "must be not null")
#define CE_FATAL(msg) CE_ASSERT(false, msg)
