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

#include "StringUtils.h"

namespace crown
{

/// @defgroup Path Path 

/// Functions for operating on strings as file paths.
///
/// @ingroup Path
namespace path
{
	/// Returns whether the path segment @a segment is valid.
	/// @note
	/// The rules for valid segments are as follows:
	/// a) The empty string is not valid.
	/// b) Any string containing the slash character ('/') is not valid.
	/// c) Common notations for current ('.') and parent ('..') directory are forbidden.
	/// d) Any string containing segment or device separator characters on the local file system,
	/// such as the backslash ('\') and colon (':') on some file systems are not valid.
	/// (Thanks org.eclipse.core.runtime for the documentation ;D).
	bool is_valid_segment(const char* segment);

	/// Returns whether @a path is valid.
	bool is_valid_path(const char* path);

	/// Returns whether the path @a path is absolute.
	bool is_absolute_path(const char* path);

	void pathname(const char* path, char* str, size_t len);
	void filename(const char* path, char* str, size_t len);
	void basename(const char* path, char* str, size_t len);
	void extension(const char* path, char* str, size_t len);
	void filename_without_extension(const char* path, char* str, size_t len);

	//bool segments(const char* path, Array<Str>& ret);
	void strip_trailing_separator(const char* path, char* ret, size_t len);
} // namespace path
} // namespace crown
