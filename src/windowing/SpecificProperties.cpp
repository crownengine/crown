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

#include "SpecificProperties.h"
#include "Log.h"
#include "Point2.h"
#include "XMLReader.h"

namespace Crown
{

inline Str SerializeToStr(const Margins& in)
{
	return Str(in.left) + ", " + Str(in.top) + ", " + Str(in.right) + ", " + Str(in.bottom);
}

bool DeserializeFromStr(Margins& out, Str& input)
{
	List<Str> coords;
	input.Split(',', coords);

	if (coords.GetSize() != 4)
		return false;

	if (!coords[0].ParseInt(&out.left))
		return false;
	if (!coords[1].ParseInt(&out.top))
		return false;
	if (!coords[2].ParseInt(&out.right))
		return false;
	if (!coords[3].ParseInt(&out.bottom))
		return false;
	
	return true;
}

IntProperty::IntProperty(const Str& name, int* valuePtr):
	Property(name), mValuePtr(valuePtr)
{

}

PropertyType IntProperty::GetType() const
{
	return PT_Int;
}

Generic IntProperty::GetValue() const
{
	return Generic(*mValuePtr);
}

void IntProperty::SetValue(const Generic& value)
{
	if (!value.asInt(*mValuePtr))
	{
		Log::E("Value not valid for Int property '" + GetName() + "'");
	}
}

UIntProperty::UIntProperty(const Str& name, uint32_t* valuePtr):
	Property(name), mValuePtr(valuePtr)
{

}

PropertyType UIntProperty::GetType() const
{
	return PT_UInt;
}

Generic UIntProperty::GetValue() const
{
	return Generic(*mValuePtr);
}

void UIntProperty::SetValue(const Generic& value)
{
	if (!value.asUInt(*mValuePtr))
	{
		Log::E("Value not valid for UInt property '" + GetName() + "'");
	}
}

UShortProperty::UShortProperty(const Str& name, uint16_t* valuePtr):
	Property(name), mValuePtr(valuePtr)
{

}

PropertyType UShortProperty::GetType() const
{
	return PT_UShort;
}

Generic UShortProperty::GetValue() const
{
	return Generic(*mValuePtr);
}

void UShortProperty::SetValue(const Generic& value)
{
	if (!value.asUShort(*mValuePtr))
	{
		Log::E("Value not valid for UShort property '" + GetName() + "'");
	}
}

FloatProperty::FloatProperty(const Str& name, float* valuePtr):
	Property(name), mValuePtr(valuePtr)
{

}

PropertyType FloatProperty::GetType() const
{
	return PT_Float;
}

Generic FloatProperty::GetValue() const
{
	return Generic(*mValuePtr);
}

void FloatProperty::SetValue(const Generic& value)
{
	if (!value.asFloat(*mValuePtr))
	{
		Log::E("Value not valid for Float property '" + GetName() + "'");
	}
}

Point2Property::Point2Property(const Str& name, Point2* valuePtr):
	Property(name), mValuePtr(valuePtr)
{

}

PropertyType Point2Property::GetType() const
{
	return PT_Point2;
}

Generic Point2Property::GetValue() const
{
	return Generic(SerializeToStr(*mValuePtr));
}

void Point2Property::SetValue(const Generic& value)
{
	Str val = value.asStr();
	if (!DeserializeFromStr(*mValuePtr, (Str&)val))
	{
		Log::E("Value not valid for Point2 property '" + GetName() + "'");
	}
}

StrProperty::StrProperty(const Str& name, Str* valuePtr):
	Property(name), mValuePtr(valuePtr)
{

}

PropertyType StrProperty::GetType() const
{
	return PT_Str;
}

Generic StrProperty::GetValue() const
{
	return Generic(*mValuePtr);
}

void StrProperty::SetValue(const Generic& value)
{
	if (!value.asStr(*mValuePtr))
	{
		Log::E("Value not valid for Str property '" + GetName() + "'");
	}
}

MarginsProperty::MarginsProperty(const Str& name, Margins* valuePtr):
	Property(name), mValuePtr(valuePtr)
{

}

PropertyType MarginsProperty::GetType() const
{
	return PT_Margins;
}

Generic MarginsProperty::GetValue() const
{
	return Generic(SerializeToStr(*mValuePtr));
}

void MarginsProperty::SetValue(const Generic& value)
{
	Str val = value.asStr();
	if (!DeserializeFromStr(*mValuePtr, (Str&)val))
	{
		Log::E("Value not valid for Margins property '" + GetName() + "'");
	}
}

TemplateProperty::TemplateProperty(const Str& name, XMLNode** valuePtr):
	Property(name), mValuePtr(valuePtr)
{

}

PropertyType TemplateProperty::GetType() const
{
	return PT_Template;
}

Generic TemplateProperty::GetValue() const
{
	return Generic(*mValuePtr);
}

void TemplateProperty::SetValue(const Generic& value)
{
	XMLNode* node = NULL;
	if (!value.asType<XMLNode*>(&node))
	{
		Log::E("The TemplateProperty wants an XMLNode representing the template informations");
		return;		
	}

	if (*mValuePtr != NULL)
	{
		delete *mValuePtr;
	}
	*mValuePtr = node->Clone();
}

IWithPropertiesProperty::IWithPropertiesProperty(const Str& name, IWithProperties** valuePtr):
	Property(name), mValuePtr(valuePtr)
{

}

PropertyType IWithPropertiesProperty::GetType() const
{
	return PT_Template;
}

Generic IWithPropertiesProperty::GetValue() const
{
	return Generic(*mValuePtr);
}

void IWithPropertiesProperty::SetValue(const Generic& value)
{
	IWithProperties* castedValue = NULL;
	if (!value.asType<IWithProperties*>(&castedValue))
	{
		Log::E("The IWithPropertiesProperty wants an object implementing the interface IWithProperties");
		return;		
	}

	*mValuePtr = castedValue;
}

//GenericListProperty::GenericListProperty(const Str& name, Shared<IList<Generic> >* valuePtr):
//	Property(name), mValuePtr(valuePtr)
//{

//}

//PropertyType GenericListProperty::GetType() const
//{
//	return PT_GenericList;
//}

//Generic GenericListProperty::GetValue() const
//{
//	return Generic(mValuePtr->GetPointer());
//}

//void GenericListProperty::SetValue(const Generic& value)
//{
//	if (value.GetType() == Generic::GT_List)
//	{
//		if (value.asList(*mValuePtr))
//		{
//			return;
//		}
//	}


//	//if (!DeserializeFromStr(*mValuePtr, value.asStr()))
//	//{
//	//	Log::E("Value not valid for Margins property '" + GetName() + "'");
//	//}
//}


EnumProperty::EnumProperty(const Str& name, int* valuePtr):
	Property(name), mValuePtr(valuePtr)
{

}

PropertyType EnumProperty::GetType() const
{
	return PT_Int;
}

Generic EnumProperty::GetValue() const
{
	return Generic(*mValuePtr);
}

void EnumProperty::AddValueMapping(const Str& valueName, int enumValue)
{
	EnumPropertyPair pair = {valueName, enumValue};
	mValuesMapping.Append(pair);
}

void EnumProperty::SetValue(const Generic& value)
{
	const Str& strValue = value.asStr();
	for(int i = 0; i < mValuesMapping.GetSize(); i++)
	{
		if (strValue == mValuesMapping[i].valueName)
		{
			*mValuePtr = mValuesMapping[i].enumValue;
			return;
		}
	}
	Log::E("Value '" + strValue + "' not valid for Enum property '" + GetName() + "'");
}

}
