/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "memory.h"

namespace crown
{

template <typename T1, typename T2, int T1Aware, int T2Aware>
struct Pair
{
};

template <typename T1, typename T2>
struct Pair<T1, T2, 0, 0>
{
	Pair(T1 f, T2 s) : first(f), second(s)
	{
	}

	Pair(Allocator& /*a*/)
	{
	}

	T1 first;
	T2 second;
};

template <typename T1, typename T2>
struct Pair<T1, T2, 1, 0>
{
	Pair(T1 f, T2 s) : first(f), second(s)
	{
	}

	Pair(Allocator& a)
		: first(a)
	{
	}

	T1 first;
	T2 second;
};

template <typename T1, typename T2>
struct Pair<T1, T2, 0, 1>
{
	Pair(T1 f, T2 s) : first(f), second(s)
	{
	}

	Pair(Allocator& a)
		: second(a)
	{
	}

	T1 first;
	T2 second;
};

template <typename T1, typename T2>
struct Pair<T1, T2, 1, 1>
{
	Pair(T1 f, T2 s) : first(f), second(s)
	{
	}

	Pair(Allocator& a)
		: first(a)
		, second(a)
	{
	}

	T1 first;
	T2 second;
};

#define PAIR(first, second) Pair<first, second, IS_ALLOCATOR_AWARE(first), IS_ALLOCATOR_AWARE(second)>

template <typename T1, typename T2>
PAIR(T1, T2) make_pair(Allocator& a, T1 first, T2 second)
{
	PAIR(T1, T2) pair(a);
	pair.first = first;
	pair.second = second;
	return pair;
}

} // namespace crown
