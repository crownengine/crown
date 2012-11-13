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

#include "Types.h"

namespace crown
{

class Generic
{
public:

	enum Type
	{
		GT_UNDEFINED	= 0,

		// Basic types
		GT_INT			= 1,
		GT_UINT			= 2,
		GT_USHORT		= 3,
		GT_FLOAT		= 4,
		GT_BOOL			= 5
	};

					Generic();
					Generic(const Generic& other);
					~Generic();

	explicit		Generic(int value);
	explicit		Generic(uint value);
	explicit		Generic(ushort value);
	explicit		Generic(float value);
	explicit		Generic(bool value);
	explicit		Generic(const char* value);

	inline Type		get_type() const { return m_type; }

	bool			as_int(int& out) const;
	int				as_int() const;

	bool			as_uint(uint& out) const;
	uint			as_uint() const;

	bool			as_ushort(ushort& out) const;
	ushort			as_ushort() const;

	bool			as_float(float& out) const;
	float			as_float() const;

	bool			as_bool(bool& out) const;
	bool			as_bool() const;

	template <typename T>
	bool			as_type(T* out) const;
	template <typename T>
	T*				as_type() const;

	const Generic&	operator=(const Generic& other);

	bool			operator==(const Generic& other);

private:

	union Data
	{
		int			int_value;
		uint		uint_value;
		ushort		ushort_value;
		float		float_value;
		bool		bool_value;
	};

	Type			m_type;
	Data			m_data;
};

template <typename T>
bool Generic::as_type(T* out) const
{
//	if (mType == GT_Object)
//	{
//		*out = dynamic_cas_t<T>(mData.object_value);
//		return *out != NULL || mData.object_value == NULL;
//	}

	return false;
}

template <typename T>
T* Generic::as_type() const
{
//	if (mType == GT_Object)
//	{
//		return dynamic_cast<T*>(mData.object_value);
//	}

	return NULL;
}

} //namespace crown

