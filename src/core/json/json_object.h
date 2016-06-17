/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "json_types.h"
#include "map.h"

namespace crown
{
/// Functions to manipulate JsonObject.
///
/// @ingroup JSON
namespace json_object
{
	/// Returns the number of keys in the object @a jo.
	inline u32 size(const JsonObject& jo)
	{
		return map::size(jo._map);
	}

	/// Returns whether the object @a jo has the @a key.
	inline bool has(const JsonObject& jo, const char* key)
	{
		return map::has(jo._map, FixedString(key));
	}

	/// Returns a pointer to the first item in the object @a jo.
	inline const Map<FixedString, const char*>::Node* begin(const JsonObject& jo)
	{
		return map::begin(jo._map);
	}

	/// Returns a pointer to the item following the last item in the object @a jo.
	inline const Map<FixedString, const char*>::Node* end(const JsonObject& jo)
	{
		return map::end(jo._map);
	}
} // namespace json_object

inline JsonObject::JsonObject(Allocator& a)
	: _map(a)
{
}

/// Returns the value of the @a key or NULL.
inline const char* JsonObject::operator[](const char* key) const
{
	return map::get(_map, FixedString(key), (const char*)NULL);
}

/// Returns the value of the @a key or NULL.
inline const char* JsonObject::operator[](const FixedString& key) const
{
	return map::get(_map, key, (const char*)NULL);
}

}
