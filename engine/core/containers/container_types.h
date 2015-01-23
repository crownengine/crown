/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "memory_types.h"
#include "functional.h"

namespace crown
{

/// @defgroup Containers Containers

/// Dynamic array of POD items.
/// @note
/// Does not call constructors/destructors so it is not very suitable for non-POD items.
///
/// @ingroup Containers
template <typename T>
struct Array
{
	Array(Allocator& allocator);

	/// Allocates capacity * sizeof(T) bytes.
	Array(Allocator& allocator, uint32_t capacity);
	Array(const Array<T>& other);
	~Array();

	/// Random access by index
	T& operator[](uint32_t index);
	const T& operator[](uint32_t index) const;

	Array<T>& operator=(const Array<T>& other);

	Allocator* _allocator;
	uint32_t _capacity;
	uint32_t _size;
	T* _array;
};

/// Dynamic array of objects.
/// @note
/// Calls constructors and destructors, not suitable for performance-critical stuff.
/// If your data is POD, use Array<T> instead.
///
/// @ingroup Containers
template <typename T>
struct Vector
{
	Vector(Allocator& allocator);
	Vector(Allocator& allocator, uint32_t capacity);
	Vector(const Vector<T>& other);
	~Vector();

	/// Random access by index
	T& operator[](uint32_t index);
	const T& operator[](uint32_t index) const;

	const Vector<T>& operator=(const Vector<T>& other);

	Allocator* _allocator;
	uint32_t _capacity;
	uint32_t _size;
	T* _array;
};

/// Circular buffer double-ended queue of POD items.
/// @note
/// Does not call constructors/destructors so it is not very suitable for non-POD items.
///
/// @ingroup Containers
template <typename T>
struct Queue
{
	Queue(Allocator& allocator);

	/// Random access by index
	T& operator[](uint32_t index);
	const T& operator[](uint32_t index) const;

	uint32_t _read;
	uint32_t _size;
	Array<T> _queue;
};

/// Priority queue of POD items.
///
/// @ingroup Containers
template <typename T>
struct PriorityQueue
{
	PriorityQueue(Allocator& a);

	Array<T> _queue;
};

/// Hash from an uint64_t to POD items. If you want to use a generic key
/// item, use a hash function to map that item to an uint64_t.
///
/// @ingroup Containers
template<typename T>
struct Hash
{
	Hash(Allocator &a);

	struct Entry
	{
		uint64_t key;
		uint32_t next;
		T value;
	};

	Array<uint32_t> _hash;
	Array<Entry> _data;
};

/// Map from key to value. Uses a Vector internally, so, definitely
/// not suited to performance-critical stuff.
///
/// @ingroup Containers
template <typename TKey, typename TValue>
struct Map
{
	Map(Allocator& a);

	const TValue& operator[](const TKey& key) const;

	struct Node
	{
		TKey key;
		TValue value;
		uint32_t left;
		uint32_t right;
		uint32_t parent;
		uint32_t color;
	};

	uint32_t _root;
	uint32_t _sentinel;
	Vector<Node> _data;
};

/// Sorted map from key to POD items.
/// Items are not automatically sorted, you need to call sort_map::sort().
/// @ingroup Containers.
template <typename TKey, typename TValue, class Compare = less<TKey> >
struct SortMap
{
	SortMap(Allocator& a);

	struct Entry
	{
		TKey key;
		TValue value;
	};

	Array<Entry> _data;
#ifdef CROWN_DEBUG
	bool _is_sorted;
#endif
};

} // namespace crown
