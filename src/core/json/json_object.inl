/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/hash_map.inl"
#include "core/json/types.h"
#include "core/murmur.h"
#include "core/strings/string_view.inl"

namespace crown
{
template<>
struct hash<StringView>
{
	u32 operator()(const StringView& val) const
	{
		return (u32)murmur32(val._data, val._length, 0);
	}
};

/// Functions to manipulate JsonObject.
///
/// @ingroup JSON
namespace json_object
{
	/// Returns the number of keys in the object @a jo.
	inline u32 size(const JsonObject& jo)
	{
		return hash_map::size(jo._map);
	}

	/// Returns whether the object @a jo has the @a key.
	inline bool has(const JsonObject& jo, const char* key)
	{
		return hash_map::has(jo._map, StringView(key));
	}

	inline bool is_hole(const JsonObject& jo, const HashMap<StringView, const char*>::Entry* entry)
	{
		return hash_map::is_hole(jo._map, entry);
	}

	/// Returns a pointer to the first item in the object @a jo.
	inline const HashMap<StringView, const char*>::Entry* begin(const JsonObject& jo)
	{
		return hash_map::begin(jo._map);
	}

	/// Returns a pointer to the item following the last item in the object @a jo.
	inline const HashMap<StringView, const char*>::Entry* end(const JsonObject& jo)
	{
		return hash_map::end(jo._map);
	}

} // namespace json_object

inline JsonObject::JsonObject(Allocator& a)
	: _map(a)
{
}

/// Returns the value of the @a key or NULL.
inline const char* JsonObject::operator[](const char* key) const
{
	return hash_map::get(_map, StringView(key), (const char*)NULL);
}

/// Returns the value of the @a key or NULL.
inline const char* JsonObject::operator[](const StringView& key) const
{
	return hash_map::get(_map, key, (const char*)NULL);
}

}

#define JSON_OBJECT_SKIP_HOLE(jo, cur) HASH_MAP_SKIP_HOLE(jo._map, cur)
