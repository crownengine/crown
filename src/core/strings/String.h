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

#include <cassert>
#include <cstdio>
#include <cstring>
#include "List.h"
#include "Types.h"

namespace crown
{
namespace string
{

const char* const	EMPTY = "";

bool				is_alpha(char c);
bool				is_digit(char c);
bool				is_upper(char c);
bool				is_lower(char c);
bool				is_whitespace(char c);

size_t				strlen(const char* str);
const char*			strstr(const char* str1, const char* str2);
int32_t				strcmp(const char* str1, const char* str2);
char*				strcpy(char* dest, const char* src);
char*				strncpy(char* dest, const char* src, size_t len);

const char*			begin(const char* str);
const char*			end(const char* str);

const char*			find_first(const char* str, char c);
const char*			find_last(const char* str, char c);
void				substring(const char* begin, const char* end, char* out, size_t len);

//inline void MakeLower()
//{
//	for (uint32_t i = 0; i < mLength; i++)
//	{
//		if (is_upper(mText[i]))
//		{
//			mText[i] += 32;
//		}
//	}
//}

//inline void MakeUpper()
//{
//	for (uint32_t i = 0; i < mLength; i++)
//	{
//		if (is_lower(mText[i]))
//		{
//			mText[i] -= 32;
//		}
//	}
//}

//inline bool StartsWith(const Str& begin) const
//{
//	if (mLength < begin.mLength)
//		return false;

//	for (uint32_t i = 0; i < begin.mLength; i++)
//	{
//		if (mText[i] != begin.mText[i])
//			return false;
//	}

//	return true;
//}

//inline bool EndsWith(const Str& end) const
//{
//	if (mLength < end.mLength)
//		return false;

//	uint32_t beginIndex = mLength - end.mLength;
//	for (uint32_t i = beginIndex; i < mLength; i++)
//	{
//		if (mText[i] != end.mText[i - beginIndex])
//			return false;
//	}

//	return true;
//}

//inline int32_t Find(const Str& Str) const
//{
//	bool found = true;

//	for (uint32_t i = 0; i < mLength; i++)
//	{
//		if (mLength - i < Str.mLength)
//		{
//			return -1;
//		}

//		for (uint32_t j = 0; j < Str.mLength; j++)
//		{
//			if (mText[i + j] != Str.mText[j])
//			{
//				found = false;
//			}
//		}

//		if (found)
//		{
//			return i;
//		}

//		found = true;
//	}

//	return -1;
//}

//inline void Remove(uint32_t start, uint32_t end)
//{
//	assert(start <= mLength);
//	assert(end <= mLength);
//	assert(start <= end);
//	uint32_t len = end - start;
//	char* tmp = new char[mLength - len + 1];

//	uint32_t i;
//	for (i = 0; i < start; i++)
//	{
//		tmp[i] = mText[i];
//	}
//	i += len;
//	for (; i < mLength; i++)
//	{
//		tmp[i - len] = mText[i];
//	}
//	mLength = mLength - len;
//	tmp[mLength] = '\0';
//	delete[] mText;
//	mText = tmp;
//}

////! Replaces all the occurencies of the given character with the new one
//inline void Replace(char toFind, char toReplace)
//{
//	for (uint32_t i = 0; i < mLength; i++)
//	{
//		if (mText[i] == toFind)
//			mText[i] = toReplace;
//	}
//}

////! Replaces all the occurencies of the given Str with the new one
//inline void Replace(const Str& toFind, const Str& toReplace)
//{
//	assert(toReplace.mLength > 0);
//	if (mLength < toReplace.mLength)
//		return;

//	List<char> tmp(get_default_allocator());

//	uint32_t i;
//	for (i = 0; i < mLength - (toFind.mLength - 1); i++)
//	{
//		bool found = true;
//		for(uint32_t j = 0; j < toFind.mLength; j++)
//			if (mText[i + j] != toFind.mText[j])
//			{
//				found = false;
//				break;
//			}
//		if (found)
//		{
//			for(uint32_t j = 0; j < toReplace.mLength; j++)
//				tmp.push_back(toReplace.mText[j]);
//			i += toFind.mLength-1;
//		}
//		else
//			tmp.push_back(mText[i]);
//	}

//	while(i <= mLength)
//	{
//		tmp.push_back(mText[i]);
//		i++;
//	}

//	*this = tmp.begin();
//}

//inline void Split(char ch, List<Str>& split) const
//{
//	uint32_t lastChar = 0;
//	uint32_t strPtr = 0;
//	uint32_t charCount = 0;

//	while (strPtr <= mLength)
//	{
//		lastChar = strPtr;

//		while (mText[strPtr] != ch && mText[strPtr] != '\0')
//		{
//			strPtr++;
//			charCount++;
//		}

//		if (charCount > 0)
//		{
//			split.push_back(this->GetSubstring(lastChar, lastChar + charCount));
//		}

//		charCount = 0;
//		strPtr++;
//	}
//}

//inline Str Trim()
//{
//	int32_t beginIndex = 0;
//	int32_t endIndex = mLength - 1;

//	while (is_whitespace(mText[beginIndex]))
//	{
//		beginIndex++;
//	}

//	while (is_whitespace(mText[endIndex]))
//	{
//		endIndex--;
//	}

//	return GetSubstring(beginIndex, endIndex + 1);
//}

//inline int32_t GetOccurrenceCount(char ch) const
//{
//	int32_t count = 0;

//	for (uint32_t i = 0; i < mLength; i++)
//	{
//		if (mText[i] == ch)
//		{
//			count++;
//		}
//	}

//	return count;
//}

//-----------------------------------------------------------------------------
inline bool is_alpha(char c)
{
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

//-----------------------------------------------------------------------------
inline bool is_digit(char c)
{
	return !(c < '0' || c > '9');
}

//-----------------------------------------------------------------------------
inline bool is_upper(char c)
{
	return (c >= 'A' && c <= 'Z');
}

//-----------------------------------------------------------------------------
inline bool is_lower(char c)
{
	return (c >= 'a' && c <= 'z');
}

//-----------------------------------------------------------------------------
inline bool is_whitespace(char c)
{
	return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

//-----------------------------------------------------------------------------
inline size_t strlen(const char* str)
{
	size_t chars = 0;

	while(*str)
	{
		if ((*str & 0xC0) != 0x80)
		{
			chars++;
		}
		str++;
	}

	return chars;
}

//-----------------------------------------------------------------------------
inline const char* strstr(const char* str1, const char* str2)
{
	return ::strstr(str1, str2);
}

//-----------------------------------------------------------------------------
inline int32_t strcmp(const char* str1, const char* str2)
{
	return ::strcmp(str1, str2);
}

//-----------------------------------------------------------------------------
inline char* strcpy(char* dest, const char* src)
{
	return ::strcpy(dest, src);
}

//-----------------------------------------------------------------------------
inline char* strncpy(char* dest, const char* src, size_t len)
{
	return ::strncpy(dest, src, len);
}

//-----------------------------------------------------------------------------
inline const char* begin(const char* str)
{
	assert(str != NULL);
	
	return str;
}

//-----------------------------------------------------------------------------
inline const char* end(const char* str)
{
	assert(str != NULL);
	
	return str + string::strlen(str) + 1;
}

//-----------------------------------------------------------------------------
inline const char* find_first(const char* str, char c)
{
	assert(str != NULL);

	const char* str_begin = string::begin(str);
	
	while (str_begin != string::end(str))
	{
		if ((*str_begin) == c)
		{
			return str_begin;
		}
		
		str_begin++;
	}
	
	return string::end(str);
}

//-----------------------------------------------------------------------------
inline const char* find_last(const char* str, char c)
{
	assert(str != NULL);
	
	const char* str_end = string::end(str) - 1;
	
	while (str_end != string::begin(str) - 1)
	{
		if ((*str_end) == c)
		{
			return str_end;
		}
		
		str_end--;
	}
	
	return string::end(str);
}

//-----------------------------------------------------------------------------
inline void substring(const char* begin, const char* end, char* out, size_t len)
{
	assert(begin != NULL);
	assert(end != NULL);
	assert(out != NULL);
	
	size_t i = 0;
	
	char* out_iterator = out;

	while (begin != end && i < len)
	{
		(*out_iterator) = (*begin);
		
		begin++;
		out_iterator++;
		i++;
	}

	out[i] = '\0';
}

} // namespace string
} // namespace crown

