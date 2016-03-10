/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

/*
 * Copyright (C) 2012 Bitsquid AB
 * License: https://bitbucket.org/bitsquid/foundation/src/default/LICENCSE
 */

#pragma once

#include "array.h"
#include "container_types.h"

namespace crown
{
	/// Functions to manipulate Hash.
	///
	/// The hash function stores its data in a "list-in-an-array" where
	/// indices are used instead of pointers.
	///
	/// When items are removed, the array-list is repacked to always keep
	/// it tightly ordered.
	///
	/// @ingroup Containers
	namespace hash
	{
		/// Returns true if the specified key exists in the hash.
		template<typename T> bool has(const Hash<T>& h, u64 key);

		/// Returns the value stored for the specified key, or deffault if the key
		/// does not exist in the hash.
		template<typename T> const T& get(const Hash<T>& h, u64 key, const T& deffault);

		/// Sets the value for the key.
		template<typename T> void set(Hash<T>& h, u64 key, const T& value);

		/// Removes the key from the hash if it exists.
		template<typename T> void remove(Hash<T>& h, u64 key);

		/// Resizes the hash lookup table to the specified size.
		/// (The table will grow automatically when 70 % full.)
		template<typename T> void reserve(Hash<T>& h, u32 size);

		/// Remove all elements from the hash.
		template<typename T> void clear(Hash<T>& h);

		/// Returns a pointer to the first entry in the hash table, can be used to
		/// efficiently iterate over the elements (in random order).
		template<typename T> const typename Hash<T>::Entry* begin(const Hash<T>& h);
		template<typename T> const typename Hash<T>::Entry* end(const Hash<T>& h);
	}

	/// Functions to manipulate Hash as a multi-hash.
	///
	/// @ingroup Containers
	namespace multi_hash
	{
		/// Finds the first entry with the specified key.
		template<typename T> const typename Hash<T>::Entry* find_first(const Hash<T>& h, u64 key);

		/// Finds the next entry with the same key as e.
		template<typename T> const typename Hash<T>::Entry* find_next(const Hash<T>& h, const typename Hash<T>::Entry* e);

		/// Returns the number of entries with the key.
		template<typename T> u32 count(const Hash<T>& h, u64 key);

		/// Returns all the entries with the specified key.
		/// Use a TempAllocator for the array to avoid allocating memory.
		template<typename T> void get(const Hash<T>& h, u64 key, Array<T> &items);

		/// Inserts the value as an aditional value for the key.
		template<typename T> void insert(Hash<T>& h, u64 key, const T& value);

		/// Removes the specified entry.
		template<typename T> void remove(Hash<T>& h, const typename Hash<T>::Entry* e);

		/// Removes all entries with the specified key.
		template<typename T> void remove_all(Hash<T>& h, u64 key);
	}

	namespace hash_internal
	{
		const u32 END_OF_LIST = 0xffffffffu;

		struct FindResult
		{
			u32 hash_i;
			u32 data_prev;
			u32 data_i;
		};

		template<typename T> u32 add_entry(Hash<T>& h, u64 key)
		{
			typename Hash<T>::Entry e;
			e.key = key;
			e.next = END_OF_LIST;
			u32 ei = array::size(h._data);
			array::push_back(h._data, e);
			return ei;
		}

		template<typename T> FindResult find(const Hash<T>& h, u64 key)
		{
			FindResult fr;
			fr.hash_i = END_OF_LIST;
			fr.data_prev = END_OF_LIST;
			fr.data_i = END_OF_LIST;

			if (array::size(h._hash) == 0)
				return fr;

			fr.hash_i = key % array::size(h._hash);
			fr.data_i = h._hash[fr.hash_i];
			while (fr.data_i != END_OF_LIST) {
				if (h._data[fr.data_i].key == key)
					return fr;
				fr.data_prev = fr.data_i;
				fr.data_i = h._data[fr.data_i].next;
			}
			return fr;
		}

		template<typename T> FindResult find(const Hash<T>& h, const typename Hash<T>::Entry* e)
		{
			FindResult fr;
			fr.hash_i = END_OF_LIST;
			fr.data_prev = END_OF_LIST;
			fr.data_i = END_OF_LIST;

			if (array::size(h._hash) == 0)
				return fr;

			fr.hash_i = e->key % array::size(h._hash);
			fr.data_i = h._hash[fr.hash_i];
			while (fr.data_i != END_OF_LIST) {
				if (&h._data[fr.data_i] == e)
					return fr;
				fr.data_prev = fr.data_i;
				fr.data_i = h._data[fr.data_i].next;
			}
			return fr;
		}

		template<typename T> void erase(Hash<T>& h, const FindResult &fr)
		{
			/// Fix from: https://bitbucket.org/bitsquid/foundation/issues/5/hash-erase-an-array-size-update-is-missing
			if (fr.data_prev == END_OF_LIST)
				h._hash[fr.hash_i] = h._data[fr.data_i].next;
			else
				h._data[fr.data_prev].next = h._data[fr.data_i].next;

			array::pop_back(h._data);

			if (fr.data_i == array::size(h._data)) return;

			h._data[fr.data_i] = h._data[array::size(h._data)];

			FindResult last = find(h, &h._data[array::size(h._data)]);

			if (last.data_prev != END_OF_LIST)
				h._data[last.data_prev].next = fr.data_i;
			else
				h._hash[last.hash_i] = fr.data_i;
		}

		template<typename T> u32 find_or_fail(const Hash<T>& h, u64 key)
		{
			return find(h, key).data_i;
		}

		template<typename T> u32 find_or_make(Hash<T>& h, u64 key)
		{
			const FindResult fr = find(h, key);
			if (fr.data_i != END_OF_LIST)
				return fr.data_i;

			u32 i = add_entry(h, key);
			if (fr.data_prev == END_OF_LIST)
				h._hash[fr.hash_i] = i;
			else
				h._data[fr.data_prev].next = i;
			return i;
		}

		template<typename T> u32 make(Hash<T>& h, u64 key)
		{
			const FindResult fr = find(h, key);
			const u32 i = add_entry(h, key);

			if (fr.data_prev == END_OF_LIST)
				h._hash[fr.hash_i] = i;
			else
				h._data[fr.data_prev].next = i;

			h._data[i].next = fr.data_i;
			return i;
		}

		template<typename T> void find_and_erase(Hash<T>& h, u64 key)
		{
			const FindResult fr = find(h, key);
			if (fr.data_i != END_OF_LIST)
				erase(h, fr);
		}

		template<typename T> void rehash(Hash<T>& h, u32 new_size)
		{
			Hash<T> nh(*h._hash._allocator);
			array::resize(nh._hash, new_size);
			array::reserve(nh._data, array::size(h._data));
			for (u32 i=0; i<new_size; ++i)
				nh._hash[i] = END_OF_LIST;
			for (u32 i=0; i<array::size(h._data); ++i) {
				const typename Hash<T>::Entry &e = h._data[i];
				multi_hash::insert(nh, e.key, e.value);
			}

			Hash<T> empty(*h._hash._allocator);
			h.~Hash<T>();
			memcpy(&h, &nh, sizeof(Hash<T>));
			memcpy(&nh, &empty, sizeof(Hash<T>));
		}

		template<typename T> bool full(const Hash<T>& h)
		{
			const f32 max_load_factor = 0.7f;
			return array::size(h._data) >= array::size(h._hash) * max_load_factor;
		}

		template<typename T> void grow(Hash<T>& h)
		{
			const u32 new_size = array::size(h._data) * 2 + 10;
			rehash(h, new_size);
		}
	}

	namespace hash
	{
		template<typename T> bool has(const Hash<T>& h, u64 key)
		{
			return hash_internal::find_or_fail(h, key) != hash_internal::END_OF_LIST;
		}

		template<typename T> const T& get(const Hash<T>& h, u64 key, const T& deffault)
		{
			const u32 i = hash_internal::find_or_fail(h, key);
			return i == hash_internal::END_OF_LIST ? deffault : h._data[i].value;
		}

		template<typename T> void set(Hash<T>& h, u64 key, const T& value)
		{
			if (array::size(h._hash) == 0)
				hash_internal::grow(h);

			const u32 i = hash_internal::find_or_make(h, key);
			h._data[i].value = value;
			if (hash_internal::full(h))
				hash_internal::grow(h);
		}

		template<typename T> void remove(Hash<T>& h, u64 key)
		{
			hash_internal::find_and_erase(h, key);
		}

		template<typename T> void reserve(Hash<T>& h, u32 size)
		{
			hash_internal::rehash(h, size);
		}

		template<typename T> void clear(Hash<T>& h)
		{
			array::clear(h._data);
			array::clear(h._hash);
		}

		template<typename T> const typename Hash<T>::Entry* begin(const Hash<T>& h)
		{
			return array::begin(h._data);
		}

		template<typename T> const typename Hash<T>::Entry* end(const Hash<T>& h)
		{
			return array::end(h._data);
		}
	}

	namespace multi_hash
	{
		template<typename T> const typename Hash<T>::Entry* find_first(const Hash<T>& h, u64 key)
		{
			const u32 i = hash_internal::find_or_fail(h, key);
			return i == hash_internal::END_OF_LIST ? 0 : &h._data[i];
		}

		template<typename T> const typename Hash<T>::Entry* find_next(const Hash<T>& h, const typename Hash<T>::Entry* e)
		{
			u32 i = e->next;
			while (i != hash_internal::END_OF_LIST) {
				if (h._data[i].key == e->key)
					return &h._data[i];
				i = h._data[i].next;
			}
			return 0;
		}

		template<typename T> u32 count(const Hash<T>& h, u64 key)
		{
			u32 i = 0;
			const typename Hash<T>::Entry* e = find_first(h, key);
			while (e) {
				++i;
				e = find_next(h, e);
			}
			return i;
		}

		template<typename T> void get(const Hash<T>& h, u64 key, Array<T> &items)
		{
			const typename Hash<T>::Entry* e = find_first(h, key);
			while (e) {
				array::push_back(items, e->value);
				e = find_next(h, e);
			}
		}

		template<typename T> void insert(Hash<T>& h, u64 key, const T& value)
		{
			if (array::size(h._hash) == 0)
				hash_internal::grow(h);

			const u32 i = hash_internal::make(h, key);
			h._data[i].value = value;
			if (hash_internal::full(h))
				hash_internal::grow(h);
		}

		template<typename T> void remove(Hash<T>& h, const typename Hash<T>::Entry* e)
		{
			const hash_internal::FindResult fr = hash_internal::find(h, e);
			if (fr.data_i != hash_internal::END_OF_LIST)
				hash_internal::erase(h, fr);
		}

		template<typename T> void remove_all(Hash<T>& h, u64 key)
		{
			while (hash::has(h, key))
				hash::remove(h, key);
		}
	}

	template <typename T> Hash<T>::Hash(Allocator &a)
		: _hash(a)
		, _data(a)
	{
	}

} // namespace crown
