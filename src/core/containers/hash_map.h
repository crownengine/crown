/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "container_types.h"
#include "array.h"
#include "vector.h"
#include <string.h> // memcpy

namespace crown
{

/// Functions to manipulate HashMap
///
/// @ingroup Containers
namespace hash_map
{
	/// Returns the number of items in the map @a m.
	template <typename TKey, typename TValue, typename Hash> uint32_t size(const HashMap<TKey, TValue, Hash>& m);

	/// Returns whether the given @a key exists in the map @a m.
	template <typename TKey, typename TValue, typename Hash> bool has(const HashMap<TKey, TValue, Hash>& m, const TKey& key);

	/// Returns the value for the given @a key or @a deffault if
	/// the key does not exist in the map.
	template <typename TKey, typename TValue, typename Hash> const TValue& get(const HashMap<TKey, TValue, Hash>& m, const TKey& key, const TValue& deffault);

	/// Sets the @a value for the @a key in the map.
	template <typename TKey, typename TValue, typename Hash> void set(HashMap<TKey, TValue, Hash>& m, const TKey& key, const TValue& value);

	/// Removes the @a key from the map if it exists.
	template <typename TKey, typename TValue, typename Hash> void remove(HashMap<TKey, TValue, Hash>& m, const TKey& key);

	/// Removes all the items in the map.
	/// @note Calls destructor on the items.
	template <typename TKey, typename TValue, typename Hash> void clear(HashMap<TKey, TValue, Hash>& m);

	/// Returns a pointer to the first item in the map, can be used to
	/// efficiently iterate over the elements (in random order).
	template <typename TKey, typename TValue, typename Hash> const typename HashMap<TKey, TValue, Hash>::Node* begin(const HashMap<TKey, TValue, Hash>& m);
	template <typename TKey, typename TValue, typename Hash> const typename HashMap<TKey, TValue, Hash>::Node* end(const HashMap<TKey, TValue, Hash>& m);
} // namespace hash_map

namespace hash_map_internal
{
	const uint32_t END_OF_LIST = 0xffffffffu;

	struct FindResult
	{
		uint32_t data_i;
	};

	template <typename TKey, typename TValue, typename Hash> uint32_t add_entry(HashMap<TKey, TValue, Hash>& m, const TKey& key)
	{
		typename HashMap<TKey, TValue, Hash>::Entry e;
		e.key = key;
		e.hash = 0;
		const uint32_t ei = vector::size(m._data);
		vector::push_back(m._data, e);
		return ei;
	}

	template <typename TKey, typename TValue, typename Hash> void rehash(HashMap<TKey, TValue, Hash>& m, uint32_t new_size)
	{
		HashMap<TKey, TValue, Hash> tm(*m._data._allocator);
		vector::reserve(tm._data, new_size);

		for (uint32_t i = 0, n = vector::size(m._data); i < n; ++i)
			hash_map::set(tm, m._data[i].key, m._data[i].value);

		m.~HashMap<TKey, TValue, Hash>();
		HashMap<TKey, TValue, Hash> empty(*m._data._allocator);
		memcpy(&m, &tm, sizeof(HashMap<TKey, TValue, Hash>));
		memcpy(&tm, &empty, sizeof(HashMap<TKey, TValue, Hash>));
	}

	template <typename TKey, typename TValue, typename Hash> void grow(HashMap<TKey, TValue, Hash>& m)
	{
		const uint32_t new_size = vector::size(m._data) * 2 + 10;
		rehash(m, new_size);
	}

	template <typename TKey, typename TValue, typename Hash> bool full(const HashMap<TKey, TValue, Hash>& m)
	{
		return vector::size(m._data) >= vector::capacity(m._data) * 0.7f;
	}

	template <typename TKey, typename TValue, typename Hash> uint32_t probe_distance(const HashMap<TKey, TValue, Hash>& m, uint32_t hash, uint32_t i)
	{
	}

	template <typename TKey, typename TValue, typename Hash> FindResult find(const HashMap<TKey, TValue, Hash>& m, const TKey& key)
	{
		FindResult fr;
		fr.data_i = END_OF_LIST;

		if (vector::size(m._data) == 0)
			return fr;

		fr.data_i = (key) % vector::size(m._data);
		for (;;)
		{
			// Empty slot
			if (m._data[fr.data_i].hash == 0)
				return fr;

			fr.data_i = (fr.data_i + 1) % vector::size(m._data);
		}

		return fr;
	}

	template <typename TKey, typename TValue, typename Hash> uint32_t find_or_make(HashMap<TKey, TValue, Hash>& m, const TKey& key)
	{
		const FindResult fr = find(m, key);
		if (fr.data_i != END_OF_LIST)
			return fr.data_i;

		return add_entry(m, key);
	}

	template <typename TKey, typename TValue, typename Hash> uint32_t find_or_fail(const HashMap<TKey, TValue, Hash>& m, const TKey& key)
	{
		return find(m, key).data_i;
	}

} // namespace hash_map_internal

namespace hash_map
{
	template <typename TKey, typename TValue, typename Hash> uint32_t size(const HashMap<TKey, TValue, Hash>& m)
	{
		return vector::size(m._data);
	}

	template <typename TKey, typename TValue, typename Hash> bool has(const HashMap<TKey, TValue, Hash>& m, const TKey& key)
	{
	}

	template <typename TKey, typename TValue, typename Hash> const TValue& get(const HashMap<TKey, TValue, Hash>& m, const TKey& key, const TValue& deffault)
	{
		const uint32_t i = hash_map_internal::find_or_fail(m, key);
		return i == hash_map_internal::END_OF_LIST ? deffault : m._data[i].value;
	}

	template <typename TKey, typename TValue, typename Hash> void set(HashMap<TKey, TValue, Hash>& m, const TKey& key, const TValue& value)
	{
		if (vector::size(m._data) == 0)
			hash_map_internal::grow(m);

		const uint32_t i = hash_map_internal::find_or_make(m, key);
		m._data[i].value = value;

		if (hash_map_internal::full(m))
			hash_map_internal::grow(m);
	}

	template <typename TKey, typename TValue, typename Hash> void remove(HashMap<TKey, TValue, Hash>& m, const TKey& key)
	{
	}

	template <typename TKey, typename TValue, typename Hash> void clear(HashMap<TKey, TValue, Hash>& m)
	{
		vector::clear(m._data);
	}
} // namespace hash_map

template <typename TKey, typename TValue, typename Hash>
inline HashMap<TKey, TValue, Hash>::HashMap(Allocator& a)
	: _hash(a)
	, _data(a)
{
}

}
