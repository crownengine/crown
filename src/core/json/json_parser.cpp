/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "json_parser.h"
#include "njson.h"
#include "temp_allocator.h"
#include "string_utils.h"
#include "vector.h"
#include "map.h"
#include "vector2.h"
#include "vector3.h"
#include "vector4.h"
#include "quaternion.h"
#include "matrix4x4.h"
#include "file.h"

namespace crown
{

JSONElement::JSONElement()
	: _at(NULL)
{
}

JSONElement::JSONElement(const char* at)
	: _at(at)
{
}

JSONElement::JSONElement(const JSONElement& other)
	: _at(other._at)
{
}

JSONElement& JSONElement::operator=(const JSONElement& other)
{
	// Our begin is the other's at
	_at = other._at;
	return *this;
}

JSONElement JSONElement::operator[](uint32_t i)
{
	Array<const char*> array(default_allocator());

	njson::parse_array(_at, array);

	CE_ASSERT(i < array::size(array), "Index out of bounds");

	return JSONElement(array[i]);
}

JSONElement JSONElement::index(uint32_t i)
{
	return this->operator[](i);
}

JSONElement JSONElement::index_or_nil(uint32_t i)
{
	if (_at != NULL)
	{
		Array<const char*> array(default_allocator());

		njson::parse_array(_at, array);

		if (i >= array::size(array))
		{
			return JSONElement();
		}

		return JSONElement(array[i]);
	}

	return JSONElement();
}

JSONElement JSONElement::key(const char* k)
{
	Map<DynamicString, const char*> object(default_allocator());
	njson::parse(_at, object);

	const char* value = map::get(object, DynamicString(k), (const char*) NULL);
	CE_ASSERT(value != NULL, "Key not found: '%s'", k);

	return JSONElement(value);
}

JSONElement JSONElement::key_or_nil(const char* k)
{
	if (_at != NULL)
	{
		Map<DynamicString, const char*> object(default_allocator());
		njson::parse(_at, object);

		const char* value = map::get(object, DynamicString(k), (const char*) NULL);

		if (value)
			return JSONElement(value);
	}

	return JSONElement();
}

bool JSONElement::has_key(const char* k) const
{
	Map<DynamicString, const char*> object(default_allocator());
	njson::parse(_at, object);

	return map::has(object, DynamicString(k));
}

bool JSONElement::to_bool(bool def) const
{
	return is_nil() ? def : njson::parse_bool(_at);
}

int32_t JSONElement::to_int(int32_t def) const
{
	return is_nil() ? def : njson::parse_int(_at);
}

float JSONElement::to_float(float def) const
{
	return is_nil() ? def : njson::parse_float(_at);
}

void JSONElement::to_string(DynamicString& str, const char* def) const
{
	if (is_nil())
		str = def;
	else
		njson::parse_string(_at, str);
}

Vector2 JSONElement::to_vector2(const Vector2& def) const
{
	if (is_nil())
		return def;

	TempAllocator64 alloc;
	Array<const char*> array(alloc);
	njson::parse_array(_at, array);

	return Vector2(njson::parse_float(array[0]),
					njson::parse_float(array[1]));
}

Vector3 JSONElement::to_vector3(const Vector3& def) const
{
	if (is_nil())
		return def;

	TempAllocator64 alloc;
	Array<const char*> array(alloc);
	njson::parse_array(_at, array);

	return Vector3(njson::parse_float(array[0]),
					njson::parse_float(array[1]),
					njson::parse_float(array[2]));
}

Vector4 JSONElement::to_vector4(const Vector4& def) const
{
	if (is_nil())
		return def;

	TempAllocator64 alloc;
	Array<const char*> array(alloc);
	njson::parse_array(_at, array);

	return Vector4(njson::parse_float(array[0]),
					njson::parse_float(array[1]),
					njson::parse_float(array[2]),
					njson::parse_float(array[3]));
}

Quaternion JSONElement::to_quaternion(const Quaternion& def) const
{
	if (is_nil())
		return def;

	TempAllocator64 alloc;
	Array<const char*> array(alloc);
	njson::parse_array(_at, array);

	return Quaternion(njson::parse_float(array[0]),
					njson::parse_float(array[1]),
					njson::parse_float(array[2]),
					njson::parse_float(array[3]));
}

Matrix4x4 JSONElement::to_matrix4x4(const Matrix4x4& def) const
{
	if (is_nil())
		return def;

	TempAllocator128 alloc;
	Array<float> array(alloc);
	to_array(array);

	return Matrix4x4(array::begin(array));
}

StringId32 JSONElement::to_string_id(const StringId32 def) const
{
	if (is_nil())
		return def;

	TempAllocator1024 alloc;
	DynamicString str(alloc);
	njson::parse_string(_at, str);
	return str.to_string_id();
}

ResourceId JSONElement::to_resource_id(const char* type) const
{
	CE_ASSERT_NOT_NULL(type);
	// TempAllocator1024 alloc;
	DynamicString str(default_allocator());
	njson::parse_string(_at, str);
	return ResourceId(type, str.c_str());
}

void JSONElement::to_array(Array<bool>& array) const
{
	Array<const char*> temp(default_allocator());

	njson::parse_array(_at, temp);

	for (uint32_t i = 0; i < array::size(temp); i++)
	{
		array::push_back(array, njson::parse_bool(temp[i]));
	}
}

void JSONElement::to_array(Array<int16_t>& array) const
{
	Array<const char*> temp(default_allocator());

	njson::parse_array(_at, temp);

	for (uint32_t i = 0; i < array::size(temp); i++)
	{
		array::push_back(array, (int16_t)njson::parse_int(temp[i]));
	}
}

void JSONElement::to_array(Array<uint16_t>& array) const
{
	Array<const char*> temp(default_allocator());

	njson::parse_array(_at, temp);

	for (uint32_t i = 0; i < array::size(temp); i++)
	{
		array::push_back(array, (uint16_t)njson::parse_int(temp[i]));
	}
}

void JSONElement::to_array(Array<int32_t>& array) const
{
	Array<const char*> temp(default_allocator());

	njson::parse_array(_at, temp);

	for (uint32_t i = 0; i < array::size(temp); i++)
	{
		array::push_back(array, (int32_t)njson::parse_int(temp[i]));
	}
}

void JSONElement::to_array(Array<uint32_t>& array) const
{
	Array<const char*> temp(default_allocator());

	njson::parse_array(_at, temp);

	for (uint32_t i = 0; i < array::size(temp); i++)
	{
		array::push_back(array, (uint32_t)njson::parse_int(temp[i]));
	}
}

void JSONElement::to_array(Array<float>& array) const
{
	Array<const char*> temp(default_allocator());

	njson::parse_array(_at, temp);

	for (uint32_t i = 0; i < array::size(temp); i++)
	{
		array::push_back(array, njson::parse_float(temp[i]));
	}
}

void JSONElement::to_array(Vector<DynamicString>& array) const
{
	Array<const char*> temp(default_allocator());

	njson::parse_array(_at, temp);

	for (uint32_t i = 0; i < array::size(temp); i++)
	{
		DynamicString str;
		njson::parse_string(temp[i], str);
		vector::push_back(array, str);
	}
}

void JSONElement::to_keys(Vector<DynamicString>& keys) const
{
	Map<DynamicString, const char*> object(default_allocator());
	njson::parse(_at, object);

	const Map<DynamicString, const char*>::Node* it = map::begin(object);
	while (it != map::end(object))
	{
		vector::push_back(keys, (*it).key);
		it++;
	}
}

bool JSONElement::is_nil() const
{
	if (_at != NULL)
	{
		return njson::type(_at) == NJSONValueType::NIL;
	}

	return true;
}

bool JSONElement::is_bool() const
{
	if (_at != NULL)
	{
		return njson::type(_at) == NJSONValueType::BOOL;
	}

	return false;
}

bool JSONElement::is_number() const
{
	if (_at != NULL)
	{
		return njson::type(_at) == NJSONValueType::NUMBER;
	}

	return false;
}

bool JSONElement::is_string() const
{
	if (_at != NULL)
	{
		return njson::type(_at) == NJSONValueType::STRING;
	}

	return false;
}

bool JSONElement::is_array() const
{
	if (_at != NULL)
	{
		return njson::type(_at) == NJSONValueType::ARRAY;
	}

	return false;
}

bool JSONElement::is_object() const
{
	if (_at != NULL)
	{
		return njson::type(_at) == NJSONValueType::OBJECT;
	}

	return false;
}

uint32_t JSONElement::size() const
{
	if (_at == NULL)
	{
		return 0;
	}

	switch(njson::type(_at))
	{
		case NJSONValueType::NIL:
		{
			return 1;
		}
		case NJSONValueType::OBJECT:
		{
			Map<DynamicString, const char*> object(default_allocator());
			njson::parse(_at, object);
			return map::size(object);
		}
		case NJSONValueType::ARRAY:
		{
			Array<const char*> array(default_allocator());
			njson::parse_array(_at, array);
			return array::size(array);
		}
		case NJSONValueType::STRING:
		{
			DynamicString string;
			njson::parse_string(_at, string);
			return string.length();
		}
		case NJSONValueType::NUMBER:
		{
			return 1;
		}
		case NJSONValueType::BOOL:
		{
			return 1;
		}
		default:
		{
			CE_FATAL("Oops, unknown value type");
			return 0;
		}
	}
}

JSONParser::JSONParser(const char* s)
	: _file(false)
	, _document(s)
{
	CE_ASSERT_NOT_NULL(s);
}

JSONParser::JSONParser(File& f)
	: _file(true)
	, _document(NULL)
{
	const size_t size = f.size();
	char* doc = (char*) default_allocator().allocate(size + 1);
	f.read(doc, size);
	doc[size] = '\0';
	_document = doc;
}

JSONParser::JSONParser(Buffer& b)
	: _file(false)
	, _document(NULL)
{
	array::push_back(b, '\0');
	_document = array::begin(b);
}

JSONParser::~JSONParser()
{
	if (_file)
	{
		default_allocator().deallocate((void*) _document);
	}
}

JSONElement JSONParser::root()
{
	const char* ch = _document;
	while ((*ch) && (*ch) <= ' ') ch++;

	return JSONElement(ch);
}

} //namespace crown
