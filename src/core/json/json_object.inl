/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/containers/hash_map.inl"
#include "core/json/types.h"
#include "core/strings/string_view.inl"

namespace crown
{
/// Functions to manipulate JsonObject.
///
/// @ingroup JSON
namespace json_object
{
	/// Returns the number of keys in the object @a jo.
	inline u32 size(const JsonObject &jo)
	{
		return hash_map::size(jo._map);
	}

	/// Returns whether the object @a jo has the @a key.
	inline bool has(const JsonObject &jo, const char *key)
	{
		return hash_map::has(jo._map, StringView(key));
	}

	inline bool is_hole(const JsonObject &jo, const HashMap<StringView, const char *>::Entry *entry)
	{
		return hash_map::is_hole(jo._map, entry);
	}

	/// Returns a pointer to the first item in the object @a jo.
	inline const HashMap<StringView, const char *>::Entry *begin(const JsonObject &jo)
	{
		return hash_map::begin(jo._map);
	}

	/// Returns a pointer to the item following the last item in the object @a jo.
	inline const HashMap<StringView, const char *>::Entry *end(const JsonObject &jo)
	{
		return hash_map::end(jo._map);
	}

} // namespace json_object

inline JsonObject::JsonObject(Allocator &a)
	: _map(a)
	, _end(NULL)
{
}

/// Returns the value of the @a key or NULL.
inline const char *JsonObject::operator[](const char *key) const
{
	return hash_map::get(_map, StringView(key), (const char *)NULL);
}

/// Returns the value of the @a key or NULL.
inline const char *JsonObject::operator[](const StringView &key) const
{
	return hash_map::get(_map, key, (const char *)NULL);
}

} // namespace crown

#define JSON_OBJECT_SKIP_HOLE(jo, cur) HASH_MAP_SKIP_HOLE(jo._map, cur)

namespace crown
{
/// Functions to manipulate FlatJsonObject.
///
/// @ingroup JSON
namespace flat_json_object
{
	inline bool has(const FlatJsonObject &obj, const char *key)
	{
		TempAllocator512 ta;
		DynamicString k(ta);
		k = key;
		return hash_map::has(obj, k);
	}

	inline const char *get(FlatJsonObject &obj, const char *key)
	{
		TempAllocator512 ta;
		DynamicString k(ta);
		k = key;
		return hash_map::get(obj, k, (const char *)NULL);
	}

} // namespace flat_json_object

} // namespace crown
