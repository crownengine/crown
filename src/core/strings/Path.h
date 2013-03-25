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

#include "String.h"
#include "OS.h"

namespace crown
{
namespace path
{

bool is_valid_segment(const char* segment);
bool is_valid_path(const char* path);
bool is_absolute_path(const char* path);

void pathname(const char* path, char* str, size_t len);
void filename(const char* path, char* str, size_t len);
void basename(const char* path, char* str, size_t len);
void extension(const char* path, char* str, size_t len);

//bool segments(const char* path, List<Str>& ret);
bool remove_trailing_separator(const char* path, char* ret);

/// Returns whether the segment is valid.
/// @note
/// The rules for valid segments are as follows:
/// a) The empty string is not valid.
/// b) Any string containing the slash character ('/') is not valid.
/// c) Common notations for current ('.') and parent ('..') directory are forbidden.
/// d) Any string containing segment or device separator characters on the local file system,
/// such as the backslash ('\') and colon (':') on some file systems.
/// (Thanks org.eclipse.core.runtime for the documentation ;D).
bool is_valid_segment(const char* segment)
{
	size_t segment_len = string::strlen(segment);

	if (segment_len == 0)
	{
		return false;
	}

	if (segment_len == 1 && segment[0] == '.')
	{
		return false;
	}

	if (segment_len == 2 && segment[0] == '.' && segment[1] == '.')
	{
		return false;
	}

	for (size_t i = 0; i < segment_len; i++)
	{
		if (segment[i] == '/' ||
			segment[i] == '\\' ||
			segment[i] == ':')
		{
			return false;
		}
	}

	return true;
}

/// Returns whether the path is valid.
/// @note
/// The rules for valid paths are as follows:
/// a) The empty string is not valid.
/// b) If the path is absolute, it mustn't contain any leading character.
bool is_valid_path(const char* path)
{
	(void)path;
//	size_t path_len = string::strlen(path);

//	if (pathLen == 0)
//	{
//		return false;
//	}

//	if (is_root_path(path))
//	{
//		return true;
//	}

//	List<Str> segmentList;
//	if (!get_segments(Str(path), segmentList))
//	{
//		return false;
//	}

//	size_t i = 0;
//	if (IsAbsolutePath(path) && path[0] != '/')
//	{
//		i = 1;
//	}

//	for (; i < segmentList.GetSize(); i++)
//	{
//		if (!IsValidSegment(segmentList[i].c_str()))
//		{
//			return false;
//		}
//	}

	return true;
}

/// Returns whether @path is absolute (i.e. starts with '/').
bool is_absolute_path(const char* path)
{
	assert(path != NULL);

	return path[0] == '/';
}

/// Returns the pathname of the path.
/// @note
/// e.g. "/home/project/texture.tga" -> "/home/project"
/// e.g. "/home/project" -> "/home"
/// e.g. "/home" -> "/"
void pathname(const char* path, char* str, size_t len)
{
	assert(path != NULL);
	assert(str != NULL);
	
	int32_t last_separator = string::find_last(path, '/');

	if (last_separator == -1 || last_separator == 0)
	{
		string::strncpy(str, "", len);
	}
	else
	{
		size_t final_len = (len >= (size_t)last_separator) ? last_separator : len;
		
		string::strncpy(str, path, final_len);
		str[final_len] = '\0';
	}
}

/// Returns the filename of the path.
/// @note
/// e.g. "/home/project/texture.tga" -> "texture.tga"
/// e.g. "/home/project/texture" -> "texture"
/// e.g. "/home -> "home"
/// e.g. "/" -> ""
void filename(const char* path, char* str, size_t len)
{
	assert(path != NULL);
	assert(str != NULL);
	
	size_t path_len = string::strlen(path);

	int32_t last_separator = string::find_last(path, '/');

	if (last_separator == -1)
	{
		size_t final_len = (len >= (size_t)last_separator) ? last_separator : len;
		
		string::strncpy(str, path, final_len);
		str[final_len] = '\0';
	}
	else
	{
		if (last_separator == 0 && path_len == 1)
		{
			string::strncpy(str, "", len);
		}
		else
		{
			size_t final_len = (len >= (size_t)(path_len - last_separator)) ? (path_len - last_separator) : len;
			
			string::strncpy(str, path + last_separator + 1, final_len);
			str[final_len] = '\0';
		}
	}
}

/// Returns the basename of the path.
/// @note
/// e.g. "/home/project/texture.tga" -> "texture"
/// e.g. "/home/project" -> "project"
/// e.g. "/" -> ""
void basename(const char* path, char* str, size_t len)
{
	assert(path != NULL);
	assert(str != NULL);
	
	size_t path_len = string::strlen(path);

	int32_t last_separator = string::find_last(path, '/');
	int32_t last_dot = string::find_last(path, '.');

	if (last_separator != -1 && last_dot != -1)
	{
		size_t basename_length = path_len - (last_separator + (path_len - last_dot)) - 1;
		
		size_t final_len = (len >= (size_t)(basename_length)) ? basename_length : len;
		
		string::strncpy(str, path + last_separator + 1, final_len);
		str[final_len] = '\0';
	}
	// "/texture"
	// "/"
	else if (last_separator != -1 && last_dot == -1)
	{
		size_t final_len = (len >= (size_t)(path_len - last_separator)) ? (path_len - last_separator) : len;
		string::strncpy(str, path + last_separator + 1, final_len);
		str[final_len] = '\0';
	}
	// "texture"
	// ""
	else if (last_separator == -1 && last_dot == -1)
	{
		size_t final_len = (len >= (size_t)path_len) ? path_len : len;
		string::strncpy(str, path, final_len);
		str[final_len] = '\0';
	}
	// "texture.tga"
	else if (last_separator == -1 && last_dot != -1)
	{
		size_t final_len = (len >= (size_t)last_dot) ? last_dot : len;
		string::strncpy(str, path, final_len);
		str[final_len] = '\0';
	}
}

/// Returns the extension of the path.
/// @note
/// e.g. "/home/project/texture.tga" -> "tga"
/// e.g. "/home/project.x/texture" -> ""
void extension(const char* path, char* str, size_t len)
{
	assert(path != NULL);
	assert(str != NULL);

	size_t path_len = string::strlen(path);
	
	int32_t last_dot = string::find_last(path, '.');
	
	if (last_dot == -1)
	{
		string::strncpy(str, "", len);
	}
	else
	{
		size_t final_len = (len >= (size_t)(path_len - last_dot)) ? (path_len - last_dot) : len;

		string::strncpy(str, path + last_dot + 1, final_len);
		str[final_len] = '\0';
	}
}

/// Returns the segments contained in path.
//bool segments(const char* path, List<Str>& ret)
//{
//	path.Split(os::PATH_SEPARATOR, ret);

//	if (ret.GetSize() > 0)
//	{
//		return true;
//	}

//	return false;
//}

/// Fills 'ret' with the same path but without the trailing directory separator.
/// @note
/// (e.g. /home/babbeo/texture.tga/ -> /home/babbeo/texture.tga).
bool remove_trailing_separator(const char* path, char* ret)
{
//	size_t path_len = string::strlen(path);
//	
//	if (path_len == 0 || is_root_path(path))
//	{
//		strcpy(ret, path);
//		return true;
//	}

//	if (path[path_len - 1] == os::PATH_SEPARATOR)
//	{
//		string::strncpy(ret, path, path_len - 1);
//		return true;
//	}

//	string::strncpy(ret, path, path_len - 1);
//	return true;
	return true;
}

} // namespace path
} // namespace crown

