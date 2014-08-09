/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
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

#include "array.h"
#include <algorithm>

namespace crown
{

/// Functions to manipulate SortMap.
///
/// @ingroup Containers
namespace sort_map
{
	/// Returns whether the @a key exists in the map.
	template <typename TKey, typename TValue, typename Compare> bool has(const SortMap<TKey, TValue, Compare>& m, const TKey& key);

	/// Returns the value for the given @a key or @a deffault if
	/// the key does not exist in the map.
	template <typename TKey, typename TValue, typename Compare> const TValue& get(const SortMap<TKey, TValue, Compare>& m, const TKey& key, const TValue& deffault);

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
	const uint32_t END_OF_LIST = 0xFFFFFFFFu;

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
			return comp(a.key, b.key);
		}

		bool operator()(const typename SortMap<TKey, TValue, Compare>::Entry& a,
			const TKey& key) const
		{
			return comp(a.key, key);
		}

		Compare comp;
	};

	template <typename TKey, typename TValue, typename Compare>
	inline FindResult find(const SortMap<TKey, TValue, Compare>& m, const TKey& key)
	{
		CE_ASSERT(m.m_is_sorted, "Map not sorted");

		FindResult result;
		result.item_i = END_OF_LIST;

		const typename SortMap<TKey, TValue, Compare>::Entry* first =
			std::lower_bound(array::begin(m.m_data), array::end(m.m_data), key,
			sort_map_internal::CompareEntry<TKey, TValue, Compare>());

		if (first != array::end(m.m_data) && !(key < first->key))
		 	result.item_i = first - array::begin(m.m_data);

		return result;
	}
} // namespace sort_map_internal

namespace sort_map
{
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

		return m.m_data[result.item_i].value;
	}

	template <typename TKey, typename TValue, typename Compare>
	inline void sort(SortMap<TKey, TValue, Compare>& m)
	{
		std::sort(array::begin(m.m_data), array::end(m.m_data),
			sort_map_internal::CompareEntry<TKey, TValue, Compare>());
		m.m_is_sorted = true;
	}

	template <typename TKey, typename TValue, typename Compare>
	inline void set(SortMap<TKey, TValue, Compare>& m, const TKey& key, const TValue& val)
	{
		sort_map_internal::FindResult result = sort_map_internal::find(m, key);

		if (result.item_i == sort_map_internal::END_OF_LIST)
		{
			typename SortMap<TKey, TValue, Compare>::Entry e;
			e.key = key;
			e.value = val;
			array::push_back(m.m_data, e);
		}
		else
		{
			m.m_data[result.item_i].value = val;
		}

		m.m_is_sorted = false;
	}

	template <typename TKey, typename TValue, typename Compare>
	inline void remove(SortMap<TKey, TValue, Compare>& m, const TKey& key)
	{
		sort_map_internal::FindResult result = sort_map_internal::find(m, key);

		if (result.item_i == sort_map_internal::END_OF_LIST)
			return;

		if (array::size(m.m_data))
		{
			m.m_data[result.item_i] = m.m_data[array::size(m.m_data) - 1];
			array::pop_back(m.m_data);
		}

		m.m_is_sorted = false;
	}

	template <typename TKey, typename TValue, typename Compare>
	inline void clear(SortMap<TKey, TValue, Compare>& m)
	{
		array::clear(m.m_data);
		m.m_is_sorted = true;
	}

	template <typename TKey, typename TValue, typename Compare>
	inline const typename SortMap<TKey, TValue, Compare>::Entry* begin(const SortMap<TKey, TValue, Compare>& m)
	{
		return array::begin(m.m_data);
	}

	template <typename TKey, typename TValue, typename Compare>
	inline const typename SortMap<TKey, TValue, Compare>::Entry* end(const SortMap<TKey, TValue, Compare>& m)
	{
		return array::end(m.m_data);
	}
} // namespace sort_map

template <typename TKey, typename TValue, typename Compare>
inline SortMap<TKey, TValue, Compare>::SortMap(Allocator& a)
	: m_is_sorted(true), m_data(a)
{
}

} // namespace crown
