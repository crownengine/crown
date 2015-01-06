/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

namespace crown
{

template<typename T>
struct less
{
	bool operator()(const T& a, const T& b) const
	{
		return a < b;
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

} // namespace crown
