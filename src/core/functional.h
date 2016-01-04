/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

namespace crown
{

template<typename T>
struct equal_to
{
	bool operator()(const T& a, const T& b) const
	{
		return a == b;
	};
};

template<typename T>
struct not_equal_to
{
	bool operator()(const T& a, const T& b) const
	{
		return a != b;
	};
};

template <typename T>
struct greater
{
	bool operator()(const T& a, const T& b) const
	{
		return a > b;
	};
};

template<typename T>
struct less
{
	bool operator()(const T& a, const T& b) const
	{
		return a < b;
	};
};

template<typename T>
struct greater_equal
{
	bool operator()(const T& a, const T& b) const
	{
		return a >= b;
	};
};

template<typename T>
struct less_equal
{
	bool operator()(const T& a, const T& b) const
	{
		return a <= b;
	};
};

} // namespace crown
