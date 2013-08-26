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

#include <cstring>

#include "Allocator.h"
#include "StringUtils.h"
#include "List.h"

namespace crown
{

///
class DynamicString
{
public:
	///
						DynamicString(Allocator& allocator);
	///
						DynamicString(Allocator& allocator, const char* s);
	///
						~DynamicString();
	///
	DynamicString&		operator+=(const DynamicString& s);
	DynamicString&		operator+=(const char* s);
	DynamicString&		operator+=(const char c);
	///
	DynamicString&		operator=(const DynamicString& s);
	DynamicString&		operator=(const char* s);
	DynamicString&		operator=(const char c);
	///
	const char*			c_str();

private:

	List<char>			m_string;
};

//-----------------------------------------------------------------------------
inline DynamicString::DynamicString(Allocator& allocator) :
	m_string(allocator)
{
}

//-----------------------------------------------------------------------------
inline DynamicString::DynamicString(Allocator& allocator, const char* s) :
	m_string(allocator)
{
	m_string.push(s, string::strlen(s));
}

//-----------------------------------------------------------------------------
inline DynamicString::~DynamicString()
{
}

//-----------------------------------------------------------------------------
inline DynamicString& DynamicString::operator+=(const DynamicString& s)
{
	const char* tmp = s.m_string.begin();

	return *this += tmp;
}

//-----------------------------------------------------------------------------
inline DynamicString& DynamicString::operator+=(const char* s)
{
	m_string.push(s, string::strlen(s));

	return *this;
}

//-----------------------------------------------------------------------------
inline DynamicString& DynamicString::operator+=(const char c)
{
	m_string.push_back(c);

	return *this;
}

//-----------------------------------------------------------------------------
inline DynamicString& DynamicString::operator=(const DynamicString& s)
{
	const char* tmp = s.m_string.begin();

	return *this = tmp;
}

//-----------------------------------------------------------------------------
inline DynamicString& DynamicString::operator=(const char* s)
{
	m_string.clear();

	m_string.push(s, string::strlen(s));

	return *this;
}

//-----------------------------------------------------------------------------
inline DynamicString& DynamicString::operator=(const char c)
{
	m_string.clear();

	m_string.push_back(c);

	return *this;
}

//-----------------------------------------------------------------------------
inline const char* DynamicString::c_str()
{
	m_string.push_back('\0');
	m_string.pop_back();

	return m_string.begin();
}

} // namespace crown