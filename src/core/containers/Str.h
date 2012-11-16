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
#include "Allocator.h"

namespace crown
{

class Str
{

public:

	//! Constructor
	Str();

	//! Constructor
	explicit Str(char c);

	//! Constructor
	Str(const char* str);

	//! Constructor
	explicit Str(int n);

	//! Constructor
	explicit Str(uint32_t n);

	//! Constructor
	explicit Str(float n);

	//! Constructor
	Str(uint32_t count, char c);

	//! Destructor
	~Str();

	//! Copy constructor
	Str(const Str& Str);

	//! Returns the Str's length
	uint32_t GetLength() const;

	//! Provides an interface for accessing Str's elements through an index
	char operator[](uint32_t index) const;

	Str& operator=(const Str& Str);
	Str& operator=(char c);
	Str& operator=(const char* str);
	Str& operator=(int n);
	Str& operator=(uint32_t n);
	Str& operator=(float n);

	Str operator+(const Str& b) const;
	Str operator+(const char* str) const;
	Str operator+(int n) const;
	Str operator+(uint32_t n) const;
	Str operator+(float n) const;

	Str& operator+=(const Str& b);
	Str& operator+=(const char* str);
	Str& operator+=(int n);
	Str& operator+=(uint32_t n);
	Str& operator+=(float n);

	friend Str operator+(const char* str, const Str& string);

	//! Returns whether this and b are equal
	bool operator==(const Str& b) const;
	bool operator==(const char* str) const;

	//! Returns whether this and b are not equal
	bool operator!=(const Str& b) const;

	bool operator>(const Str& b) const;
	bool operator>=(const Str& b) const;
	bool operator<(const Str& b) const;
	bool operator<=(const Str& b) const;

	//! Makes the Str lower case
	void MakeLower();

	//! Makes the Str upper case
	void MakeUpper();

	//! Checks wether the Str begins with the given subStr
	bool StartsWith(const Str& begin) const;

	//! Checks wether the Str ends with the given subStr
	bool EndsWith(const Str& end) const;

	//! Returns the index of the first occurrence of a character in this Str or -1 if not found
	int FindFirst(char ch) const;

	//! Returns the index of the last occurrence of a character in this Str or -1 if not found
	int FindLast(char ch) const;

	//! Returns the index of the first occurrence of Str in this Str or -1 if not found
	int Find(const Str& Str) const;

	//! Returns the subStr of this Str starting at position 'start' and ending at position 'end - 1'
	Str GetSubstring(uint32_t start, uint32_t end) const;

	//! Returns the Str with the subStr from position 'start' to position 'end - 1' removed
	void Remove(uint32_t start, uint32_t end);

	//! Replaces all the occurencies of the given character with the new one
	void Replace(char toFind, char toReplace);

	//! Replaces all the occurencies of the given Str with the new one
	void Replace(const Str& toFind, const Str& toReplace);

	//! Splits the Str according to a specific delimiter character
	void Split(char ch, List<Str>& split) const;

	//! Trims spaces at the beginning and at the end of the Str
	Str Trim();

	//! Returns the number of occurrence of char 'ch' 
	int GetOccurrenceCount(char ch) const;

	char* c_str() const;

	static inline const Str& Empty()
	{
		return mEmpty;
	}

	static StrId32			Hash32(const char* str);
	static StrId32			Hash32(const char* str, size_t len);
	static StrId64			Hash64(const char* str);
	static StrId64			Hash64(const char* str, size_t len);

	static bool				IsAlpha(char c);
	static bool				IsDigit(char c);
	static bool				IsUpper(char c);
	static bool				IsLower(char c);
	static bool				IsWhitespace(char c);

	static size_t			StrLen(const char* str);
	static const char*		StrStr(const char* str1, const char* str2);
	static int				StrCmp(const char* str1, const char* str2);
	static char*			StrCpy(char* dest, const char* src);

	static int				FindFirst(const char* str, char c);
	static int				FindLast(const char* str, char c);

	bool					ParseInt(int* value) const;
	bool					ParseUInt(uint32_t* value) const;
	bool					ParseFloat(float* value) const;

	static const char*		EMPTY;

private:

	uint32_t mLength;
	char* mText;

	static Str mEmpty;
};

inline Str::Str() : mLength(0), mText(0)
{
	*this = mEmpty;
}

inline Str::Str(char c) : mLength(0), mText(0)
{
	*this = c;
}

inline Str::Str(const char* str) : mLength(0), mText(0)
{
	*this = str;
}

inline Str::Str(int n) : mLength(0), mText(0)
{
	*this = n;
}

inline Str::Str(uint32_t n) : mLength(0), mText(0)
{
	*this = n;
}

inline Str::Str(float n) : mLength(0), mText(0)
{
	*this = n;
}

inline Str::Str(uint32_t count, char c) : mLength(0), mText(0)
{
	mText = new char[count + 1];
	uint32_t i = 0;

	while (i < count)
	{
		mText[i++] = c;
	}

	mText[i] = '\0';
	mLength = count;
}

inline Str::~Str()
{
	if (mText)
	{
		delete[] mText;
	}
}

inline Str::Str(const Str& Str) : mLength(0), mText(0)
{
	*this = Str;
}

inline uint32_t Str::GetLength() const
{
	uint32_t chars = 0;

	for (uint32_t i = 0; i < mLength; i++)
	{
		if ((mText[i] & 0xC0) != 0x80)
		{
			chars++;
		}
	}

	return chars;
}

inline char Str::operator[](uint32_t index) const
{
	assert(index < mLength);
	return mText[index];
}

inline Str& Str::operator=(const Str& Str)
{
	if (this == &Str)
	{
		return *this;
	}

	if (mText)
	{
		delete[] mText;
	}

	mLength = Str.mLength;
	mText = new char[mLength + 1];

	for (uint32_t i = 0; i < mLength; i++)
	{
		mText[i] = Str.mText[i];
	}

	mText[mLength] = '\0';
	return *this;
}

inline Str& Str::operator=(char c)
{
	if (mText)
	{
		delete[] mText;
	}

	mLength = 1;

	mText = new char[2];

	mText[0] = c;
	mText[1] = '\0';

	return *this;
}

inline Str& Str::operator=(const char* str)
{
	assert(str != 0);

	if (mText)
	{
		delete[] mText;
	}

	uint32_t len = 0;

	while (str[len++]) ;

	mLength = len - 1;
	mText = new char[len];

	for (uint32_t i = 0; i < len; i++)
	{
		mText[i] = str[i];
	}

	return *this;
}

inline Str& Str::operator=(int n)
{
	char tmp[12] = {0};
	bool negative = false;
	uint32_t i = 10;

	if (n < 0)
	{
		n *= -1;
		negative = true;
	}

	do
	{
		tmp[i--] = (n % 10) + '0';
		n = n / 10;
	}
	while (n && i);

	if (negative)
	{
		tmp[i--] = '-';
	}

	*this = &tmp[i + 1];
	return *this;
}

inline Str& Str::operator=(uint32_t n)
{
	char tmp[11] = {0};
	uint32_t i = 9;

	do
	{
		tmp[i--] = (n % 10) + '0';
		n = n / 10;
	}
	while (n && i);

	*this = &tmp[i + 1];
	return *this;
}

inline Str& Str::operator=(float n)
{
	char tmp[20] = {0};
	sprintf(tmp, "%f", n);
	*this = tmp;
	return *this;
}

inline Str Str::operator+(const Str& b) const
{
	if (this == &b)
	{
		return *this;
	}

	Str tmp;
	uint32_t len = mLength + b.mLength;
	tmp.mLength = len;
	delete[] tmp.mText;
	tmp.mText = new char[len + 1];

	for (uint32_t i = 0; i < mLength; i++)
	{
		tmp.mText[i] = mText[i];
	}

	for (uint32_t i = 0; i < b.mLength; i++)
	{
		tmp.mText[i + mLength] = b.mText[i];
	}

	tmp.mText[len] = '\0';
	return tmp;
}

inline Str Str::operator+(const char* str) const
{
	return *this + Str(str);
}

inline Str Str::operator+(int n) const
{
	return *this + Str(n);
}

inline Str Str::operator+(uint32_t n) const
{
	return *this + Str(n);
}

inline Str Str::operator+(float n) const
{
	return *this + Str(n);
}

inline Str& Str::operator+=(const Str& b)
{
	*this = *this + b;
	return *this;
}

inline Str& Str::operator+=(const char* str)
{
	*this = *this + str;
	return *this;
}

inline Str& Str::operator+=(int n)
{
	*this = *this + n;
	return *this;
}

inline Str& Str::operator+=(uint32_t n)
{
	*this = *this + n;
	return *this;
}

inline Str& Str::operator+=(float n)
{
	*this = *this + n;
	return *this;
}

inline bool Str::operator==(const Str& b) const
{
	if (mLength != b.mLength)
	{
		return false;
	}

	for (uint32_t i = 0; i < mLength; i++)
		if (mText[i] != b.mText[i])
		{
			return false;
		}

	return true;
}

inline bool Str::operator==(const char* str) const
{
	size_t strLength = Str::StrLen(str);
	if (mLength != (uint32_t)strLength)
	{
		return false;
	}

	for (uint32_t i = 0; i < mLength; i++)
		if (mText[i] != str[i])
		{
			return false;
		}

	return true;
}

inline bool Str::operator!=(const Str& b) const
{
	return !(*this == b);
}

inline bool Str::operator>(const Str& b) const
{
	return strcmp(mText, b.mText) > 0;
}

inline bool Str::operator>=(const Str& b) const
{
	return strcmp(mText, b.mText) >= 0;
}

inline bool Str::operator<(const Str& b) const
{
	return strcmp(mText, b.mText) < 0;
}

inline bool Str::operator<=(const Str& b) const
{
	return strcmp(mText, b.mText) <= 0;
}

inline void Str::MakeLower()
{
	for (uint32_t i = 0; i < mLength; i++)
	{
		if (Str::IsUpper(mText[i]))
		{
			mText[i] += 32;
		}
	}
}

inline void Str::MakeUpper()
{
	for (uint32_t i = 0; i < mLength; i++)
	{
		if (Str::IsLower(mText[i]))
		{
			mText[i] -= 32;
		}
	}
}

inline bool Str::StartsWith(const Str& begin) const
{
	if (mLength < begin.mLength)
		return false;

	for (uint32_t i = 0; i < begin.mLength; i++)
	{
		if (mText[i] != begin.mText[i])
			return false;
	}

	return true;
}

inline bool Str::EndsWith(const Str& end) const
{
	if (mLength < end.mLength)
		return false;

	uint32_t beginIndex = mLength - end.mLength;
	for (uint32_t i = beginIndex; i < mLength; i++)
	{
		if (mText[i] != end.mText[i - beginIndex])
			return false;
	}

	return true;
}

inline int Str::FindFirst(char ch) const
{
	for (uint32_t i = 0; i < mLength; i++)
	{
		if (mText[i] == ch)
		{
			return i;
		}
	}

	return -1;
}

inline int Str::FindLast(char ch) const
{
	for (uint32_t i = mLength; i > 0; i--)
	{
		if (mText[i - 1] == ch)
		{
			return i - 1;
		}
	}

	return -1;
}

inline int Str::Find(const Str& Str) const
{
	bool found = true;

	for (uint32_t i = 0; i < mLength; i++)
	{
		if (mLength - i < Str.mLength)
		{
			return -1;
		}

		for (uint32_t j = 0; j < Str.mLength; j++)
		{
			if (mText[i + j] != Str.mText[j])
			{
				found = false;
			}
		}

		if (found)
		{
			return i;
		}

		found = true;
	}

	return -1;
}

inline Str Str::GetSubstring(uint32_t start, uint32_t end) const
{
	assert(start <= mLength);
	assert(end <= mLength);
	assert(start <= end);
	uint32_t len = end - start;
	char* tmp = new char[len + 1];

	for (uint32_t i = 0; i < len; i++)
	{
		tmp[i] = mText[i + start];
	}

	tmp[len] = '\0';
	Str ret(tmp);
	delete[] tmp;
	return ret;
}

inline void Str::Remove(uint32_t start, uint32_t end)
{
	assert(start <= mLength);
	assert(end <= mLength);
	assert(start <= end);
	uint32_t len = end - start;
	char* tmp = new char[mLength - len + 1];

	uint32_t i;
	for (i = 0; i < start; i++)
	{
		tmp[i] = mText[i];
	}
	i += len;
	for (; i < mLength; i++)
	{
		tmp[i - len] = mText[i];
	}
	mLength = mLength - len;
	tmp[mLength] = '\0';
	delete[] mText;
	mText = tmp;
}

//! Replaces all the occurencies of the given character with the new one
inline void Str::Replace(char toFind, char toReplace)
{
	for (uint32_t i = 0; i < mLength; i++)
	{
		if (mText[i] == toFind)
			mText[i] = toReplace;
	}
}

//! Replaces all the occurencies of the given Str with the new one
inline void Str::Replace(const Str& toFind, const Str& toReplace)
{
	assert(toReplace.mLength > 0);
	if (mLength < toReplace.mLength)
		return;

	List<char> tmp(get_default_allocator());

	uint32_t i;
	for (i = 0; i < mLength - (toFind.mLength - 1); i++)
	{
		bool found = true;
		for(uint32_t j = 0; j < toFind.mLength; j++)
			if (mText[i + j] != toFind.mText[j])
			{
				found = false;
				break;
			}
		if (found)
		{
			for(uint32_t j = 0; j < toReplace.mLength; j++)
				tmp.push_back(toReplace.mText[j]);
			i += toFind.mLength-1;
		}
		else
			tmp.push_back(mText[i]);
	}

	while(i <= mLength)
	{
		tmp.push_back(mText[i]);
		i++;
	}

	*this = tmp.begin();
}

inline void Str::Split(char ch, List<Str>& split) const
{
	uint32_t lastChar = 0;
	uint32_t strPtr = 0;
	uint32_t charCount = 0;

	while (strPtr <= mLength)
	{
		lastChar = strPtr;

		while (mText[strPtr] != ch && mText[strPtr] != '\0')
		{
			strPtr++;
			charCount++;
		}

		if (charCount > 0)
		{
			split.push_back(this->GetSubstring(lastChar, lastChar + charCount));
		}

		charCount = 0;
		strPtr++;
	}
}

inline Str Str::Trim()
{
	int beginIndex = 0;
	int endIndex = mLength - 1;

	while (Str::IsWhitespace(mText[beginIndex]))
	{
		beginIndex++;
	}

	while (Str::IsWhitespace(mText[endIndex]))
	{
		endIndex--;
	}

	return GetSubstring(beginIndex, endIndex + 1);
}

inline int Str::GetOccurrenceCount(char ch) const
{
	int count = 0;

	for (uint32_t i = 0; i < mLength; i++)
	{
		if (mText[i] == ch)
		{
			count++;
		}
	}

	return count;
}

inline char* Str::c_str() const
{
	return (char*)mText;
}

inline Str operator+(const char* str, const Str& string)
{
	return Str(str) + string;
}

inline bool Str::ParseInt(int* value) const
{
	if (sscanf(mText, "%d", value) != 1)
	{
		return false;
	}

	return true;
}

inline bool Str::ParseUInt(uint32_t* value) const
{
	if (sscanf(mText, "%u", value) != 1)
	{
		return false;
	}

	return true;
}

inline bool Str::ParseFloat(float* value) const
{
	if (sscanf(mText, "%f", value) != 1)
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
inline bool Str::IsAlpha(char c)
{
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

//-----------------------------------------------------------------------------
inline bool Str::IsDigit(char c)
{
	return !(c < '0' || c > '9');
}

//-----------------------------------------------------------------------------
inline bool Str::IsUpper(char c)
{
	return (c >= 'A' && c <= 'Z');
}

//-----------------------------------------------------------------------------
inline bool Str::IsLower(char c)
{
	return (c >= 'a' && c <= 'z');
}

//-----------------------------------------------------------------------------
inline bool Str::IsWhitespace(char c)
{
	return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

//-----------------------------------------------------------------------------
inline StrId32 Str::Hash32(const char* str)
{
	assert(str != NULL);

	// FNV-1a
	uint32_t hash = 2166136261u;

	for (size_t i = 0; str[i] != '\0'; i++)
	{
		hash ^= str[i];
		hash *= 16777619u;
	}

	return hash;
}

//-----------------------------------------------------------------------------
inline StrId32 Str::Hash32(const char* str, size_t len)
{
	assert(str != NULL);
	assert(len <= Str::StrLen(str));

	// FNV-1a
	uint32_t hash = 2166136261u;

	for (size_t i = 0; i < len; i++)
	{
		hash ^= str[i];
		hash *= 16777619u;
	}

	return hash;
}

//-----------------------------------------------------------------------------
inline StrId64 Str::Hash64(const char* str)
{
	assert(str != NULL);

	// FNV-1a
	uint64_t hash = 14695981039346656037u;

	for (size_t i = 0; str[i] != '\0'; i++)
	{
		hash ^= str[i];
		hash *= 1099511628211u;
	}

	return hash;
}

//-----------------------------------------------------------------------------
inline StrId64 Str::Hash64(const char* str, size_t len)
{
	assert(str != NULL);
	assert(len <= Str::StrLen(str));

	// FNV-1a
	uint64_t hash = 14695981039346656037u;

	for (size_t i = 0; i < len; i++)
	{
		hash ^= str[i];
		hash *= 1099511628211u;
	}

	return hash;
}

//-----------------------------------------------------------------------------
inline size_t Str::StrLen(const char* str)
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
inline const char* Str::StrStr(const char* str1, const char* str2)
{
	return strstr(str1, str2);
}

//-----------------------------------------------------------------------------
inline int Str::StrCmp(const char* str1, const char* str2)
{
	return strcmp(str1, str2);
}

//-----------------------------------------------------------------------------
inline char* Str::StrCpy(char* dest, const char* src)
{
	return strcpy(dest, src);
}

//-----------------------------------------------------------------------------
inline int Str::FindFirst(const char* str, char c)
{
	assert(str != NULL);

	size_t strLen = Str::StrLen(str);

	for (size_t i = 0; i < strLen; i++)
	{
		if (str[i] == c)
		{
			return i;
		}
	}

	return -1;
}

//-----------------------------------------------------------------------------
inline int Str::FindLast(const char* str, char c)
{
	assert(str != NULL);

	size_t strLen = Str::StrLen(str);

	for (size_t i = strLen; i > 0; i--)
	{
		if (str[i - 1] == c)
		{
			return i - 1;
		}
	}

	return -1;
}

} // namespace crown

