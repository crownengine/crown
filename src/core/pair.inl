/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/pair.h"
#include <string.h> // memcpy

namespace crown
{
template <typename T1, typename T2>
inline Pair<T1, T2, 0, 0>::Pair(T1& f, T2& s)
	: first(f)
	, second(s)
{
}

template <typename T1, typename T2>
inline Pair<T1, T2, 0, 0>::Pair(Allocator& /*a*/)
	: first()
	, second()
{
}

template <typename T1, typename T2>
inline Pair<T1, T2, 1, 0>::Pair(T1& f, T2& s)
	: first(f)
	, second(s)
{
}

template <typename T1, typename T2>
inline Pair<T1, T2, 1, 0>::Pair(Allocator& a)
	: first(a)
	, second()
{
}

template <typename T1, typename T2>
inline Pair<T1, T2, 0, 1>::Pair(T1& f, T2& s)
	: first(f)
	, second(s)
{
}

template <typename T1, typename T2>
inline Pair<T1, T2, 0, 1>::Pair(Allocator& a)
	: first()
	, second(a)
{
}

template <typename T1, typename T2>
inline Pair<T1, T2, 1, 1>::Pair(T1& f, T2& s)
	: first(f)
	, second(s)
{
}

template <typename T1, typename T2>
inline Pair<T1, T2, 1, 1>::Pair(Allocator& a)
	: first(a)
	, second(a)
{
}

template <typename T1, typename T2>
inline void swap(Pair<T1, T2, 0, 0>& a, Pair<T1, T2, 0, 0>& b)
{
	char c[sizeof(a)];
	memcpy((void*)&c, (void*)&a, sizeof(a));
	memcpy((void*)&a, (void*)&b, sizeof(a));
	memcpy((void*)&b, (void*)&c, sizeof(a));
}

template <typename T1, typename T2>
inline void swap(Pair<T1, T2, 0, 1>& a, Pair<T1, T2, 0, 1>& b)
{
	char c[sizeof(a)];
	memcpy((void*)&c, (void*)&a, sizeof(a));
	memcpy((void*)&a, (void*)&b, sizeof(a));
	memcpy((void*)&b, (void*)&c, sizeof(a));
}

template <typename T1, typename T2>
inline void swap(Pair<T1, T2, 1, 0>& a, Pair<T1, T2, 1, 0>& b)
{
	char c[sizeof(a)];
	memcpy((void*)&c, (void*)&a, sizeof(a));
	memcpy((void*)&a, (void*)&b, sizeof(a));
	memcpy((void*)&b, (void*)&c, sizeof(a));
}

template <typename T1, typename T2>
inline void swap(Pair<T1, T2, 1, 1>& a, Pair<T1, T2, 1, 1>& b)
{
	char c[sizeof(a)];
	memcpy((void*)&c, (void*)&a, sizeof(a));
	memcpy((void*)&a, (void*)&b, sizeof(a));
	memcpy((void*)&b, (void*)&c, sizeof(a));
}

} // namespace crown
