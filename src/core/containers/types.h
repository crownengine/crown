/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/functional.h"
#include "core/memory/types.h"
#include "core/pair.h"
#include "core/types.h"

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
template <typename TKey, typename TValue, typename Hash = hash<TKey>, typename KeyEqual = equal_to<TKey> >
struct HashMap
{
	ALLOCATOR_AWARE;

	typedef PAIR(TKey, TValue) Entry;

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
	char* _buffer;

	HashMap(Allocator& a);
	~HashMap();
};

/// Hash set.
///
/// @ingroup Containers
template <typename TKey, typename Hash = hash<TKey>, typename KeyEqual = equal_to<TKey> >
struct HashSet
{
	ALLOCATOR_AWARE;

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
	TKey* _data;
	char* _buffer;

	HashSet(Allocator& a);
	~HashSet();
};

/// Vector of sorted items.
///
/// @note
/// Items are not automatically sorted, you have to call sort_map::sort()
/// whenever you are done inserting/removing items.
///
/// @ingroup Containers.
template <typename TKey, typename TValue, typename Compare = less<TKey> >
struct SortMap
{
	ALLOCATOR_AWARE;

	typedef PAIR(TKey, TValue) Entry;

	Vector<Entry> _data;
#if CROWN_DEBUG
	bool _is_sorted;
#endif

	SortMap(Allocator& a);
};

/// Node in an intrusive linked list.
///
/// @ingroup Containers
struct ListNode
{
	ListNode* next;
	ListNode* prev;
};

} // namespace crown
