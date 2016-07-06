/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "functional.h"
#include "memory_types.h"
#include "pair.h"
#include "types.h"

/// @defgroup Containers Containers
/// @ingroup Core
namespace crown
{
/// Dynamic array of POD items.
///
/// @note
/// Does not call constructors/destructors, uses
/// memcpy to move stuff around.
///
/// @ingroup Containers
template <typename T>
struct Array
{
	ALLOCATOR_AWARE;

	Allocator* _allocator;
	u32 _capacity;
	u32 _size;
	T* _data;

	Array(Allocator& a);
	Array(Allocator& a, u32 capacity);
	Array(const Array<T>& other);
	~Array();
	T& operator[](u32 index);
	const T& operator[](u32 index) const;
	Array<T>& operator=(const Array<T>& other);
};

typedef Array<char> Buffer;

/// Dynamic array of objects.
///
/// @note
/// Calls constructors and destructors.
/// If your data is POD, use Array<T> instead.
///
/// @ingroup Containers
template <typename T>
struct Vector
{
	ALLOCATOR_AWARE;

	Allocator* _allocator;
	u32 _capacity;
	u32 _size;
	T* _data;

	Vector(Allocator& a);
	Vector(Allocator& a, u32 capacity);
	Vector(const Vector<T>& other);
	~Vector();
	T& operator[](u32 index);
	const T& operator[](u32 index) const;
	const Vector<T>& operator=(const Vector<T>& other);
};

/// Circular buffer double-ended queue of POD items.
///
/// @ingroup Containers
template <typename T>
struct Queue
{
	ALLOCATOR_AWARE;

	u32 _read;
	u32 _size;
	Array<T> _queue;

	Queue(Allocator& a);
	T& operator[](u32 index);
	const T& operator[](u32 index) const;
};

/// Map from key to value. Uses a Vector internally, so, definitely
/// not suited to performance-critical stuff.
///
/// @ingroup Containers
template <typename TKey, typename TValue>
struct Map
{
	ALLOCATOR_AWARE;

	struct Node
	{
		ALLOCATOR_AWARE;

		PAIR(TKey, TValue) pair;
		u32 left;
		u32 right;
		u32 parent;
		u32 color;

		Node(Allocator& a)
			: pair(a)
		{
		}
	};

	u32 _root;
	u32 _sentinel;
	Vector<Node> _data;

	Map(Allocator& a);
	const TValue& operator[](const TKey& key) const;
};

/// Hash map.
///
/// @ingroup Containers
template <typename TKey, typename TValue, class Hash = hash<TKey> >
struct HashMap
{
	ALLOCATOR_AWARE;

	struct Entry
	{
		ALLOCATOR_AWARE;

		PAIR(TKey, TValue) pair;

		Entry(Allocator& a)
			: pair(a)
		{
		}
	};

	struct Index
	{
		u32 hash;
		u32 index;
	};

	Allocator* _allocator;
	u32 _capacity;
	u32 _size;
	u32 _mask;
	Index* _index;
	Entry* _data;

	HashMap(Allocator& a);
	~HashMap();
	const TValue& operator[](const TKey& key) const;
};

/// Vector of sorted items.
///
/// @note
/// Items are not automatically sorted, you have to call sort_map::sort()
/// whenever you are done inserting/removing items.
///
/// @ingroup Containers.
template <typename TKey, typename TValue, class Compare = less<TKey> >
struct SortMap
{
	ALLOCATOR_AWARE;

	struct Entry
	{
		ALLOCATOR_AWARE;

		PAIR(TKey, TValue) pair;

		Entry(Allocator& a)
			: pair(a)
		{
		}
	};

	Vector<Entry> _data;
#if CROWN_DEBUG
	bool _is_sorted;
#endif

	SortMap(Allocator& a);
};

} // namespace crown
