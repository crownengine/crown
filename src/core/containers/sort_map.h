/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "vector.h"
#include <algorithm>

namespace crown
{
/// Functions to manipulate SortMap.
///
/// @ingroup Containers
namespace sort_map
{
	/// Returns the number of items in the map @m.
	template <typename TKey, typename TValue, typename Compare> uint32_t size(const SortMap<TKey, TValue, Compare>& m);

	/// Returns whether the @a key exists in the map.
	template <typename TKey, typename TValue, typename Compare> bool has(const SortMap<TKey, TValue, Compare>& m, const TKey& key);

	/// Returns the value for the given @a key or @a deffault if
	/// the key does not exist in the map.
	template <typename TKey, typename TValue, typename Compare> const TValue& get(const SortMap<TKey, TValue, Compare>& m, const TKey& key, const TValue& deffault);

	/// Returns the value for the given @a key or @a deffault if
	/// the key does not exist in the map.
	template <typename TKey, typename TValue, typename Compare> TValue& get(SortMap<TKey, TValue, Compare>& m, const TKey& key, const TValue& deffault);

	/// Sorts the keys in the map.
	template <typename TKey, typename TValue, typename Compare> void sort(SortMap<TKey, TValue, Compare>& m);

	template <typename TKey, typename TValue, typename Compare> void set(SortMap<TKey, TValue, Compare>& m, const TKey& key, const TValue& val);

	/// Removes the @a key from the map if it exists.
	template <typename TKey, typename TValue, typename Compare> void remove(SortMap<TKey, TValue, Compare>& m, const TKey& key);

	/// Removes all the items in the map.
	template <typename TKey, typename TValue, typename Compare> void clear(SortMap<TKey, TValue, Compare>& m);

	/// Returns a pointer to the first item in the map, can be used to
	/// efficiently iterate over the elements.
	template <typename TKey, typename TValue, typename Compare> const typename SortMap<TKey, TValue, Compare>::Entry* begin(const SortMap<TKey, TValue, Compare>& m);
	template <typename TKey, typename TValue, typename Compare> const typename SortMap<TKey, TValue, Compare>::Entry* end(const SortMap<TKey, TValue, Compare>& m);
} // namespace sort_map

namespace sort_map_internal
{
	const uint32_t END_OF_LIST = 0xffffffffu;

	struct FindResult
	{
		uint32_t item_i;
	};

	template <typename TKey, typename TValue, typename Compare>
	struct CompareEntry
	{
		bool operator()(const typename SortMap<TKey, TValue, Compare>::Entry& a,
			const typename SortMap<TKey, TValue, Compare>::Entry& b) const
		{
			return comp(a.pair.first, b.pair.first);
		}

		bool operator()(const typename SortMap<TKey, TValue, Compare>::Entry& a,
			const TKey& key) const
		{
			return comp(a.pair.first, key);
		}

		Compare comp;
	};

	template <typename TKey, typename TValue, typename Compare>
	inline FindResult find(const SortMap<TKey, TValue, Compare>& m, const TKey& key)
	{
		CE_ASSERT(m._is_sorted, "Map not sorted");

		FindResult result;
		result.item_i = END_OF_LIST;

		const typename SortMap<TKey, TValue, Compare>::Entry* first =
			std::lower_bound(vector::begin(m._data), vector::end(m._data), key,
			sort_map_internal::CompareEntry<TKey, TValue, Compare>());

		if (first != vector::end(m._data) && !(key < first->pair.first))
			result.item_i = uint32_t(first - vector::begin(m._data));

		return result;
	}
} // namespace sort_map_internal

namespace sort_map
{
	template <typename TKey, typename TValue, typename Compare>
	inline uint32_t size(const SortMap<TKey, TValue, Compare>& m)
	{
		return vector::size(m._data);
	}

	template <typename TKey, typename TValue, typename Compare>
	inline bool has(const SortMap<TKey, TValue, Compare>& m, const TKey& key)
	{
		return sort_map_internal::find(m, key).item_i != sort_map_internal::END_OF_LIST;
	}

	template <typename TKey, typename TValue, typename Compare>
	const TValue& get(const SortMap<TKey, TValue, Compare>& m, const TKey& key, const TValue& deffault)
	{
		sort_map_internal::FindResult result = sort_map_internal::find(m, key);

		if (result.item_i == sort_map_internal::END_OF_LIST)
			return deffault;

		return m._data[result.item_i].pair.second;
	}

	template <typename TKey, typename TValue, typename Compare>
	TValue& get(SortMap<TKey, TValue, Compare>& m, const TKey& key, const TValue& deffault)
	{
		return const_cast<TValue&>(get(static_cast<const SortMap<TKey, TValue, Compare>&>(m), key, deffault));
	}

	template <typename TKey, typename TValue, typename Compare>
	inline void sort(SortMap<TKey, TValue, Compare>& m)
	{
		std::sort(vector::begin(m._data), vector::end(m._data),
			sort_map_internal::CompareEntry<TKey, TValue, Compare>());
#if CROWN_DEBUG
		m._is_sorted = true;
#endif // CROWN_DEBUG
	}

	template <typename TKey, typename TValue, typename Compare>
	inline void set(SortMap<TKey, TValue, Compare>& m, const TKey& key, const TValue& val)
	{
		sort_map_internal::FindResult result = sort_map_internal::find(m, key);

		if (result.item_i == sort_map_internal::END_OF_LIST)
		{
			typename SortMap<TKey, TValue, Compare>::Entry e(*m._data._allocator);
			e.pair.first = key;
			e.pair.second = val;
			vector::push_back(m._data, e);
		}
		else
		{
			m._data[result.item_i].pair.second = val;
		}
#if CROWN_DEBUG
		m._is_sorted = false;
#endif // CROWN_DEBUG
	}

	template <typename TKey, typename TValue, typename Compare>
	inline void remove(SortMap<TKey, TValue, Compare>& m, const TKey& key)
	{
		sort_map_internal::FindResult result = sort_map_internal::find(m, key);

		if (result.item_i == sort_map_internal::END_OF_LIST)
			return;

		if (vector::size(m._data))
		{
			m._data[result.item_i] = m._data[vector::size(m._data) - 1];
			vector::pop_back(m._data);
		}
#if CROWN_DEBUG
		m._is_sorted = false;
#endif // CROWN_DEBUG
	}

	template <typename TKey, typename TValue, typename Compare>
	inline void clear(SortMap<TKey, TValue, Compare>& m)
	{
		vector::clear(m._data);
#if CROWN_DEBUG
		m._is_sorted = true;
#endif // CROWN_DEBUG
	}

	template <typename TKey, typename TValue, typename Compare>
	inline const typename SortMap<TKey, TValue, Compare>::Entry* begin(const SortMap<TKey, TValue, Compare>& m)
	{
		return vector::begin(m._data);
	}

	template <typename TKey, typename TValue, typename Compare>
	inline const typename SortMap<TKey, TValue, Compare>::Entry* end(const SortMap<TKey, TValue, Compare>& m)
	{
		return vector::end(m._data);
	}
} // namespace sort_map

template <typename TKey, typename TValue, typename Compare>
inline SortMap<TKey, TValue, Compare>::SortMap(Allocator& a)
	: _data(a)
#if CROWN_DEBUG
	, _is_sorted(true)
#endif // CROWN_DEBUG
{
}

} // namespace crown
