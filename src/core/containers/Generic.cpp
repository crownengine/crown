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

#include "Generic.h"
#include "Str.h"
#include "List.h"
#include "Types.h"

namespace Crown
{

//-----------------------------------------------------------------------------
Generic::Generic():
	mType(GT_Undefined)
{
	mData.intValue = 0;
}

//-----------------------------------------------------------------------------
Generic::Generic(const Generic& other)
{
	*this = other;
}

//-----------------------------------------------------------------------------
Generic::~Generic()
{
	if (mType == GT_Str)
	{
		delete mData.stringValue;
	}
//	else if (mType == GT_List)
//	{
//		delete mData.listValue;
//	}
}

//-----------------------------------------------------------------------------
Generic::Generic(int value):
	mType(GT_Int)
{
	mData.intValue = value;
}

//-----------------------------------------------------------------------------
Generic::Generic(uint value):
	mType(GT_UInt)
{
	mData.uintValue = value;
}

//-----------------------------------------------------------------------------
Generic::Generic(ushort value):
	mType(GT_UShort)
{
	mData.ushortValue = value;
}

//-----------------------------------------------------------------------------
Generic::Generic(float value):
	mType(GT_Float)
{
	mData.floatValue = value;
}

//-----------------------------------------------------------------------------
Generic::Generic(bool value):
	mType(GT_Bool)
{
	mData.boolValue = value;
}

//-----------------------------------------------------------------------------
Generic::Generic(const Str& value):
	mType(GT_Str)
{
	mData.stringValue = new Str(value);
}

//-----------------------------------------------------------------------------
Generic::Generic(const char* value):
	mType(GT_Str)
{
	mData.stringValue = new Str(value);
}

////-----------------------------------------------------------------------------
//Generic::Generic(IList<Generic>* value):
//	mType(GT_List)
//{
//	mData.listValue = new Shared<IList<Generic> >(value);
//}

////-----------------------------------------------------------------------------
//Generic::Generic(Object* value):
//	mType(GT_Object)
//{
//	mData.objectValue = value;
//}

//-----------------------------------------------------------------------------
bool Generic::asInt(int& out) const
{
	if (mType == GT_Int)
	{
		out = mData.intValue;
		return true;
	}

	if (mType == GT_UInt)
	{
		out = (int)mData.uintValue;
		return true;
	}

	if (mType == GT_UShort)
	{
		out = (int)mData.ushortValue;
		return true;
	}

	if (mType == GT_Float)
	{
		out = (int)mData.floatValue;
		return true;
	}

	if (mType == GT_Bool)
	{
		out = mData.boolValue?1:0;
		return true;
	}

	if (mType == GT_Str)
	{
		return mData.stringValue->ParseInt(&out);
	}

	return false;
}

//-----------------------------------------------------------------------------
int Generic::asInt() const
{
	int value = 0;
	asInt(value);
	return value;
}

//-----------------------------------------------------------------------------
bool Generic::asUInt(uint& out) const
{
	if (mType == GT_Int)
	{
		out = (uint)mData.intValue;
		return true;
	}

	if (mType == GT_UInt)
	{
		out = mData.uintValue;
		return true;
	}

	if (mType == GT_UShort)
	{
		out = (uint)mData.ushortValue;
		return true;
	}

	if (mType == GT_Float)
	{
		out = (uint)mData.floatValue;
		return true;
	}

	if (mType == GT_Bool)
	{
		out = mData.boolValue?1:0;
		return true;
	}

	if (mType == GT_Str)
	{
		return mData.stringValue->ParseUInt(&out);
	}

	return false;
}

//-----------------------------------------------------------------------------
uint Generic::asUInt() const
{
	uint value = 0;
	asUInt(value);
	return value;
}

//-----------------------------------------------------------------------------
bool Generic::asUShort(ushort& out) const
{
	if (mType == GT_Int)
	{
		out = (ushort)mData.intValue;
		return true;
	}

	if (mType == GT_UInt)
	{
		out = (ushort)mData.uintValue;
		return true;
	}

	if (mType == GT_UShort)
	{
		out = mData.ushortValue;
		return true;
	}

	if (mType == GT_Float)
	{
		out = (ushort)mData.floatValue;
		return true;
	}

	if (mType == GT_Bool)
	{
		out = mData.boolValue?1:0;
		return true;
	}

	if (mType == GT_Str)
	{
		uint tmp;
		if (!mData.stringValue->ParseUInt(&tmp))
		{
			return false;
		}
		out = (ushort)tmp;
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
ushort Generic::asUShort() const
{
	ushort value = 0;
	asUShort(value);
	return value;
}

//-----------------------------------------------------------------------------
bool Generic::asFloat(float& out) const
{
	if (mType == GT_Int)
	{
		out = (float)mData.intValue;
		return true;
	}

	if (mType == GT_UInt)
	{
		out = (float)mData.uintValue;
		return true;
	}

	if (mType == GT_UShort)
	{
		out = (float)mData.ushortValue;
		return true;
	}

	if (mType == GT_Float)
	{
		out = mData.floatValue;
		return true;
	}

	if (mType == GT_Bool)
	{
		out = mData.boolValue?1.0f:0.0f;
		return true;
	}

	if (mType == GT_Str)
	{
		return mData.stringValue->ParseFloat(&out);
	}

	return false;
}

//-----------------------------------------------------------------------------
float Generic::asFloat() const
{
	float value = 0.0f;
	asFloat(value);
	return value;
}

//-----------------------------------------------------------------------------
bool Generic::asBool(bool& out) const
{
	if (mType == GT_Int)
	{
		out = mData.intValue==0?false:true;
		return true;
	}

	if (mType == GT_UInt)
	{
		out = mData.uintValue==0?false:true;
		return true;
	}

	if (mType == GT_UShort)
	{
		out = mData.ushortValue==0?false:true;
		return true;
	}

	if (mType == GT_Float)
	{
		//Does it make sense? xD
		out = mData.floatValue==0.0f?false:true;
		return true;
	}

	if (mType == GT_Bool)
	{
		out = mData.boolValue;
		return true;
	}

	if (mType == GT_Str)
	{
		if (*mData.stringValue == "True" || *mData.stringValue == "true")
		{
			out = true;
			return true;
		}

		if (*mData.stringValue == "False" || *mData.stringValue == "false")
		{
			out = false;
			return true;
		}

		return false;
	}

	return false;
}

//-----------------------------------------------------------------------------
bool Generic::asBool() const
{
	bool value = false;
	asBool(value);
	return value;
}

//-----------------------------------------------------------------------------
bool Generic::asStr(Str& out) const
{
	if (mType == GT_Int)
	{
		out = mData.intValue;
		return true;
	}

	if (mType == GT_UInt)
	{
		out = mData.uintValue;
		return true;
	}

	if (mType == GT_UShort)
	{
		out = mData.ushortValue;
		return true;
	}

	if (mType == GT_Float)
	{
		out = mData.floatValue;
		return true;
	}

	if (mType == GT_Bool)
	{
		if (mData.boolValue)
		{
			out = "True";
		}
		else
		{
			out = "False";
		}

		return true;
	}

	if (mType == GT_Str)
	{
		out = *mData.stringValue;
		return true;
	}

//	if (mType == GT_List)
//	{
//		out = (*mData.listValue)->ToStr();
//		return true;
//	}

//	if (mType == GT_Object)
//	{
//		out = mData.objectValue->ToStr();
//		return true;
//	}

	return false;
}

//-----------------------------------------------------------------------------
Str Generic::asStr() const
{
	Str value;
	asStr(value);
	return value;
}

////-----------------------------------------------------------------------------
//bool Generic::asList(Shared<IList<Generic> >& out) const
//{
//	if (mType == GT_List)
//	{
//		out = *mData.listValue;
//		return true;
//	}

//	return false;
//}

////-----------------------------------------------------------------------------
//bool Generic::asObject(Object** out) const
//{
//	if (mType == GT_Object)
//	{
//		*out = mData.objectValue;
//		return true;
//	}

//	*out = NULL;
//	return false;
//}

////-----------------------------------------------------------------------------
//Object* Generic::asObject() const
//{
//	if (mType == GT_Object)
//	{
//		return mData.objectValue;
//	}

//	return NULL;
//}

//-----------------------------------------------------------------------------
bool Generic::operator==(const Generic& other)
{
	if (mType != other.mType)
	{
		return false;
	}

	switch (mType)
	{
		case GT_Int:
			return mData.intValue == other.mData.intValue;
			break;
		case GT_UInt:
			return mData.uintValue == other.mData.uintValue;
			break;
		case GT_UShort:
			return mData.ushortValue == other.mData.ushortValue;
			break;
		case GT_Float:
			return mData.floatValue == other.mData.floatValue;
			break;
		case GT_Bool:
			return mData.boolValue == other.mData.boolValue;
			break;
		case GT_Str:
			return mData.stringValue == other.mData.stringValue;
			break;
//		case GT_List:
//			return *mData.listValue == *other.mData.listValue;
//			break;
//		case GT_Object:
//			return mData.objectValue == other.mData.objectValue;
//			break;
		case GT_Undefined:
			return true;
			break;
	}

	return false;
}

//-----------------------------------------------------------------------------
const Generic& Generic::operator=(const Generic& other)
{
	mType = other.mType;

	switch (mType)
	{
		case GT_Int:
			mData.intValue = other.mData.intValue;
			break;
		case GT_UInt:
			mData.uintValue = other.mData.uintValue;
			break;
		case GT_UShort:
			mData.ushortValue = other.mData.ushortValue;
			break;
		case GT_Float:
			mData.floatValue = other.mData.floatValue;
			break;
		case GT_Bool:
			mData.boolValue = other.mData.boolValue;
			break;
		case GT_Str:
			mData.stringValue = new Str(*other.mData.stringValue);
			break;
//		case GT_List:
//			mData.listValue = new Shared<IList<Generic> >(*other.mData.listValue);
//			break;
//		case GT_Object:
//			mData.objectValue = other.mData.objectValue;
//			break;
		case GT_Undefined:
			//Copy nothing
			break;
	}

	return other;
}

////-----------------------------------------------------------------------------
//Str GenericList::ToStr() const
//{
//	Str out;
//	out = "[";
//	List<Generic>::Enumerator en = getBegin();
//	bool first = true;

//	while (en.next())
//	{
//		if (!first)
//		{
//			out += ",";
//		}

//		first = false;
//		out += en.current().asStr();
//	}

//	return out + "]";
//}

}
