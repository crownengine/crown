/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
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
	HashMap(const HashMap& other);
	~HashMap();
	HashMap<TKey, TValue, Hash, KeyEqual>& operator=(const HashMap<TKey, TValue, Hash, KeyEqual>& other);
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
	HashSet(const HashSet& other);
	~HashSet();
	HashSet<TKey, Hash, KeyEqual>& operator=(const HashSet<TKey, Hash, KeyEqual>& other);
};

} // namespace crown
