/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/types.h"
#include <string.h> // memcpy

namespace crown
{
/// Functions to manipulate HashSet.
///
/// @ingroup Containers
namespace hash_set
{
	/// Returns the number of items in the set @a m.
	template <typename TKey, typename Hash, typename KeyEqual> u32 size(const HashSet<TKey, Hash, KeyEqual>& m);

	/// Returns the maximum number of items the set @a m can hold.
	template <typename TKey, typename Hash, typename KeyEqual> u32 capacity(const HashSet<TKey, Hash, KeyEqual>& m);

	/// Returns whether the given @a key exists in the set @a m.
	template <typename TKey, typename Hash, typename KeyEqual> bool has(const HashSet<TKey, Hash, KeyEqual>& m, const TKey& key);

	/// Inserts the @a key in the set if it does not exist.
	template <typename TKey, typename Hash, typename KeyEqual> void insert(HashSet<TKey, Hash, KeyEqual>& m, const TKey& key);

	/// Removes the @a key from the set if it exists.
	template <typename TKey, typename Hash, typename KeyEqual> void remove(HashSet<TKey, Hash, KeyEqual>& m, const TKey& key);

	/// Removes all the items in the set.
	///
	/// @note
	/// Calls destructor on the items.
	template <typename TKey, typename Hash, typename KeyEqual> void clear(HashSet<TKey, Hash, KeyEqual>& m);

	/// Returns whether the @a entry in the set @a m contains data or is a hole.
	/// If the entry is a hole you should not touch data in the entry.
	template <typename TKey, typename Hash, typename KeyEqual> bool is_hole(const HashSet<TKey, Hash, KeyEqual>& m, const TKey* entry);

	/// Returns a pointer to the first item in the set, can be used to
	/// efficiently iterate over the elements (in random order).
	/// @note
	/// You should skip invalid items with HASH_SET_SKIP_HOLE().
	template <typename TKey, typename Hash, typename KeyEqual> const TKey* begin(const HashSet<TKey, Hash, KeyEqual>& m);
	template <typename TKey, typename Hash, typename KeyEqual> const TKey* end(const HashSet<TKey, Hash, KeyEqual>& m);

} // namespace hash_set

namespace hash_set_internal
{
	const u32 END_OF_LIST = 0xffffffffu;
	const u32 DELETED = 0x80000000u;
	const u32 FREE = 0x00000000u;

	template <typename TKey, typename Hash>
	inline u32 key_hash(const TKey& key)
	{
		const Hash hash;
		return hash(key);
	}

	template <typename TKey, typename KeyEqual>
	inline bool key_equals(const TKey& key_a, const TKey& key_b)
	{
		const KeyEqual equal;
		return equal(key_a, key_b);
	}

	inline bool is_deleted(u32 index)
	{
		// MSB set indicates that this hash is a "tombstone"
		return (index >> 31) != 0;
	}

	template <typename TKey, typename Hash, typename KeyEqual>
	inline u32 probe_distance(const HashSet<TKey, Hash, KeyEqual>& m, u32 hash, u32 slot_index)
	{
		const u32 hash_i = hash & m._mask;
		return (slot_index + m._capacity - hash_i) & m._mask;
	}

	template <typename TKey, typename Hash, typename KeyEqual>
	u32 find(const HashSet<TKey, Hash, KeyEqual>& m, const TKey& key)
	{
		if (m._size == 0)
			return END_OF_LIST;

		const u32 hash = key_hash<TKey, Hash>(key);
		u32 hash_i = hash & m._mask;
		u32 dist = 0;
		for(;;)
		{
			if (m._index[hash_i].index == FREE)
				return END_OF_LIST;
			else if (dist > probe_distance(m, m._index[hash_i].hash, hash_i))
				return END_OF_LIST;
			else if (!is_deleted(m._index[hash_i].index) && m._index[hash_i].hash == hash && key_equals<TKey, KeyEqual>(m._data[hash_i], key))
				return hash_i;

			hash_i = (hash_i + 1) & m._mask;
			++dist;
		}
	}

	template <typename TKey, typename Hash, typename KeyEqual>
	void insert(HashSet<TKey, Hash, KeyEqual>& m, u32 hash, const TKey& key)
	{
		char new_item[sizeof(TKey)];
		construct<TKey>(new_item, *m._allocator, IS_ALLOCATOR_AWARE_TYPE(TKey)());
		*(TKey*)new_item = key;

		u32 hash_i = hash & m._mask;
		u32 dist = 0;
		for(;;)
		{
			if (m._index[hash_i].index == FREE)
				goto INSERT_AND_RETURN;

			// If the existing elem has probed less than us, then swap places with existing
			// elem, and keep going to find another slot for that elem.
			u32 existing_elem_probe_dist = probe_distance(m, m._index[hash_i].hash, hash_i);
			if (is_deleted(m._index[hash_i].index) || existing_elem_probe_dist < dist)
			{
				if (is_deleted(m._index[hash_i].index))
					goto INSERT_AND_RETURN;

				exchange(hash, m._index[hash_i].hash);
				m._index[hash_i].index = 0x0123abcd;
				// swap
				{
					char c[sizeof(TKey)];
					memcpy((void*)&c,               (void*)new_item,         sizeof(TKey));
					memcpy((void*)new_item,         (void*)&m._data[hash_i], sizeof(TKey));
					memcpy((void*)&m._data[hash_i], (void*)&c,               sizeof(TKey));
				}

				dist = existing_elem_probe_dist;
			}

			hash_i = (hash_i + 1) & m._mask;
			++dist;
		}

	INSERT_AND_RETURN:
		memcpy((void*)(m._data + hash_i), &new_item, sizeof(new_item));
		m._index[hash_i].hash = hash;
		m._index[hash_i].index = 0x0123abcd;
		char empty[sizeof(TKey)];
		construct<TKey>(empty, *m._allocator, IS_ALLOCATOR_AWARE_TYPE(TKey)());
		memcpy((void*)&new_item, &empty, sizeof(new_item));
		((TKey*)new_item)->~TKey();
		return;
	}

	template <typename TKey, typename Hash, typename KeyEqual>
	void rehash(HashSet<TKey, Hash, KeyEqual>& m, u32 new_capacity)
	{
		typedef typename HashSet<TKey, Hash, KeyEqual>::Index Index;

		HashSet<TKey, Hash, KeyEqual> nm(*m._allocator);
		const u32 size = new_capacity * (sizeof(Index) + sizeof(TKey)) + alignof(Index) + alignof(TKey);
		nm._buffer = (char*)nm._allocator->allocate(size);
		nm._index = (Index*)memory::align_top(nm._buffer, alignof(Index));
		nm._data = (TKey*)memory::align_top(nm._index + new_capacity, alignof(TKey));

		// Flag all elements as free
		for (u32 i = 0; i < new_capacity; ++i)
		{
			nm._index[i].hash = 0;
			nm._index[i].index = FREE;
		}

		nm._capacity = new_capacity;
		nm._size = m._size;
		nm._mask = new_capacity - 1;

		for (u32 i = 0; i < m._capacity; ++i)
		{
			TKey& e = m._data[i];
			const u32 hash = m._index[i].hash;
			const u32 index = m._index[i].index;

			if (index != FREE && !is_deleted(index))
				hash_set_internal::insert(nm, hash, e);
		}

		HashSet<TKey, Hash, KeyEqual> empty(*m._allocator);
		m.~HashSet<TKey, Hash, KeyEqual>();
		memcpy((void*)&m, (void*)&nm, sizeof(HashSet<TKey, Hash, KeyEqual>));
		memcpy((void*)&nm, (void*)&empty, sizeof(HashSet<TKey, Hash, KeyEqual>));
	}

	template <typename TKey, typename Hash, typename KeyEqual>
	void grow(HashSet<TKey, Hash, KeyEqual>& m)
	{
		const u32 new_capacity = (m._capacity == 0 ? 16 : m._capacity * 2);
		rehash(m, new_capacity);
	}

	template <typename TKey, typename Hash, typename KeyEqual>
	bool full(const HashSet<TKey, Hash, KeyEqual>& m)
	{
		return m._size >= m._capacity * 0.9f;
	}

} // namespace hash_set_internal

namespace hash_set
{
	template <typename TKey, typename Hash, typename KeyEqual>
	u32 size(const HashSet<TKey, Hash, KeyEqual>& m)
	{
		return m._size;
	}

	template <typename TKey, typename Hash, typename KeyEqual>
	u32 capacity(const HashSet<TKey, Hash, KeyEqual>& m)
	{
		return m._capacity;
	}

	template <typename TKey, typename Hash, typename KeyEqual>
	bool has(const HashSet<TKey, Hash, KeyEqual>& m, const TKey& key)
	{
		return hash_set_internal::find(m, key) != hash_set_internal::END_OF_LIST;
	}

	template <typename TKey, typename Hash, typename KeyEqual>
	void insert(HashSet<TKey, Hash, KeyEqual>& m, const TKey& key)
	{
		if (m._capacity == 0)
			hash_set_internal::grow(m);

		// Find or make
		const u32 i = hash_set_internal::find(m, key);
		if (i == hash_set_internal::END_OF_LIST)
		{
			hash_set_internal::insert(m, hash_set_internal::key_hash<TKey, Hash>(key), key);
			++m._size;
		}
		if (hash_set_internal::full(m))
			hash_set_internal::grow(m);
	}

	template <typename TKey, typename Hash, typename KeyEqual>
	void remove(HashSet<TKey, Hash, KeyEqual>& m, const TKey& key)
	{
		const u32 i = hash_set_internal::find(m, key);
		if (i == hash_set_internal::END_OF_LIST)
			return;

		m._data[i].~TKey();
		m._index[i].index |= hash_set_internal::DELETED;
		--m._size;
	}

	template <typename TKey, typename Hash, typename KeyEqual>
	void clear(HashSet<TKey, Hash, KeyEqual>& m)
	{
		for (u32 i = 0; i < m._capacity; ++i)
		{
			if (m._index[i].index == 0x0123abcd)
				m._data[i].~TKey();
			m._index[i].index = hash_set_internal::FREE;
		}

		m._size = 0;
	}

	template <typename TKey, typename Hash, typename KeyEqual>
	bool is_hole(const HashSet<TKey, Hash, KeyEqual>& m, const TKey* entry)
	{
		const u32 ii = u32(entry - m._data);
		const u32 index = m._index[ii].index;

		return index == hash_set_internal::FREE || hash_set_internal::is_deleted(index);
	}

	template <typename TKey, typename Hash, typename KeyEqual>
	inline const TKey* begin(const HashSet<TKey, Hash, KeyEqual>& m)
	{
		return m._data;
	}

	template <typename TKey, typename Hash, typename KeyEqual>
	inline const TKey* end(const HashSet<TKey, Hash, KeyEqual>& m)
	{
		return m._data + m._capacity;
	}

} // namespace hash_set

template <typename TKey, typename Hash, typename KeyEqual>
inline HashSet<TKey, Hash, KeyEqual>::HashSet(Allocator& a)
	: _allocator(&a)
	, _capacity(0)
	, _size(0)
	, _mask(0)
	, _index(NULL)
	, _data(NULL)
	, _buffer(NULL)
{
}

template <typename TKey, typename Hash, typename KeyEqual>
HashSet<TKey, Hash, KeyEqual>::HashSet(const HashSet& other)
	: _allocator(other._allocator)
	, _capacity(0)
	, _size(0)
	, _mask(0)
	, _index(NULL)
	, _data(NULL)
	, _buffer(NULL)
{
	_capacity = other._capacity;
	_size = other._size;
	_mask = other._mask;

	if (other._capacity > 0)
	{
		_allocator->deallocate(_buffer);
		const u32 size = other._capacity * (sizeof(Index) + sizeof(TKey)) + alignof(Index) + alignof(TKey);
		_buffer = (char*)_allocator->allocate(size);
		_index = (Index*)memory::align_top(_buffer, alignof(Index));
		_data = (TKey*)memory::align_top(_index + _capacity, alignof(TKey));

		memcpy(_index, other._index, sizeof(Index)*other._capacity);
		for (u32 i = 0; i < other._capacity; ++i)
		{
			const u32 index = other._index[i].index;
			if (index != hash_set_internal::FREE && !hash_set_internal::is_deleted(index))
				new (&_data[i]) TKey(other._data[i]);
		}
	}
}

template <typename TKey, typename Hash, typename KeyEqual>
inline HashSet<TKey, Hash, KeyEqual>::~HashSet()
{
	for (u32 i = 0; i < _capacity; ++i)
	{
		if (_index[i].index == 0x0123abcd)
			_data[i].~TKey();
	}

	_allocator->deallocate(_buffer);
}

template <typename TKey, typename Hash, typename KeyEqual>
HashSet<TKey, Hash, KeyEqual>& HashSet<TKey, Hash, KeyEqual>::operator=(const HashSet<TKey, Hash, KeyEqual>& other)
{
	_capacity = other._capacity;
	_size = other._size;
	_mask = other._mask;

	if (other._capacity > 0)
	{
		_allocator->deallocate(_buffer);
		const u32 size = other._capacity * (sizeof(Index) + sizeof(TKey)) + alignof(Index) + alignof(TKey);
		_buffer = (char*)_allocator->allocate(size);
		_index = (Index*)memory::align_top(_buffer, alignof(Index));
		_data = (TKey*)memory::align_top(_index + _capacity, alignof(TKey));

		memcpy(_index, other._index, sizeof(Index)*other._capacity);
		for (u32 i = 0; i < other._capacity; ++i)
		{
			const u32 index = other._index[i].index;
			if (index != hash_set_internal::FREE && !hash_set_internal::is_deleted(index))
			{
				construct<TKey>(_data + i, *_allocator, IS_ALLOCATOR_AWARE_TYPE(TKey)());
				_data[i] = other._data[i];
			}
		}
	}
	return *this;
}

#define HASH_SET_SKIP_HOLE(m, cur) \
	if (hash_set::is_hole(m, cur)) \
		continue

} // namespace crown
