/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/memory/types.h"

namespace crown
{
template <typename T1, typename T2, int T1Aware, int T2Aware>
struct Pair
{
};

template <typename T1, typename T2>
struct Pair<T1, T2, 0, 0>
{
	ALLOCATOR_AWARE;

	T1 first;
	T2 second;

	///
	Pair(T1& f, T2& s);

	///
	Pair(Allocator& /*a*/);
};

template <typename T1, typename T2>
struct Pair<T1, T2, 1, 0>
{
	ALLOCATOR_AWARE;

	T1 first;
	T2 second;

	///
	Pair(T1& f, T2& s);

	///
	Pair(Allocator& a);
};

template <typename T1, typename T2>
struct Pair<T1, T2, 0, 1>
{
	ALLOCATOR_AWARE;

	T1 first;
	T2 second;

	///
	Pair(T1& f, T2& s);

	///
	Pair(Allocator& a);
};

template <typename T1, typename T2>
struct Pair<T1, T2, 1, 1>
{
	ALLOCATOR_AWARE;

	T1 first;
	T2 second;

	///
	Pair(T1& f, T2& s);

	///
	Pair(Allocator& a);
};

#define PAIR(first, second) Pair<first, second, IS_ALLOCATOR_AWARE(first), IS_ALLOCATOR_AWARE(second)>

} // namespace crown
