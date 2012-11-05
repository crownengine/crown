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
#include "Shared.h"

namespace crown
{

class Str;

class Generic
{
public:

	enum Type
	{
		GT_Undefined = 0,
		/* -- Basic types -- */
		GT_Int,
		GT_UInt,
		GT_UShort,
		GT_Float,
		GT_Bool,

		/* -- Container types -- */
		GT_Str
//		GT_List,
//		GT_Object
	};

					Generic();
					Generic(const Generic& other);
	virtual			~Generic();

					Generic(int value);
					Generic(uint value);
					Generic(ushort value);
					Generic(float value);
					Generic(bool value);
					Generic(const char* value);
					Generic(const Str& value);
//					Generic(IList<Generic>* value);
//					Generic(Object* value);

	inline Type		GetType() const
	{
		return mType;
	}

	bool			asInt(int& out) const;
	int				asInt() const;

	bool			asUInt(uint& out) const;
	uint			asUInt() const;

	bool			asUShort(ushort& out) const;
	ushort			asUShort() const;

	bool			asFloat(float& out) const;
	float			asFloat() const;

	bool			asBool(bool& out) const;
	bool			asBool() const;

	bool			asStr(Str& out) const;
	Str				asStr() const;

//	bool			asList(Shared<IList<Generic> >& out) const;

//	bool			asObject(Object** out) const;
//	Object*			asObject() const;

	template <typename T>
	bool			asType(T* out) const;
	template <typename T>
	T*				asType() const;

	const Generic&	operator=(const Generic& other);

	bool			operator==(const Generic& other);

private:

	union Data
	{
		int			intValue;
		uint		uintValue;
		ushort		ushortValue;
		float		floatValue;
		bool		boolValue;
		Str*		stringValue;
//		Shared<IList<Generic> >* listValue;
//		Object* objectValue;
	};

	Type			mType;
	Data			mData;
};

template <typename T>
bool Generic::asType(T* out) const
{
//	if (mType == GT_Object)
//	{
//		*out = dynamic_cast<T>(mData.objectValue);
//		return *out != NULL || mData.objectValue == NULL;
//	}

	return false;
}

template <typename T>
T* Generic::asType() const
{
//	if (mType == GT_Object)
//	{
//		return dynamic_cast<T*>(mData.objectValue);
//	}

	return NULL;
}

} //namespace crown

