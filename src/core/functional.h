/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "murmur.h"
#include "types.h"

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

// Hash functions
template <typename T>
struct hash;

template<>
struct hash<bool>
{
	u32 operator()(const bool val) const
	{
		return (u32)val;
	}
};

template<>
struct hash<s8>
{
	u32 operator()(const s8 val) const
	{
		return (u32)val;
	}
};

template<>
struct hash<u8>
{
	u32 operator()(const u8 val) const
	{
		return (u32)val;
	}
};

template<>
struct hash<s16>
{
	u32 operator()(const s16 val) const
	{
		return (u32)val;
	}
};

template<>
struct hash<u16>
{
	u32 operator()(const u16 val) const
	{
		return (u32)val;
	}
};

template<>
struct hash<s32>
{
	u32 operator()(const s32 val) const
	{
		return (u32)val;
	}
};

template<>
struct hash<u32>
{
	u32 operator()(const u32 val) const
	{
		return (u32)val;
	}
};

template<>
struct hash<s64>
{
	u32 operator()(const s64 val) const
	{
		return (u32)val;
	}
};

template<>
struct hash<u64>
{
	u32 operator()(const u64 val) const
	{
		return (u32)val;
	}
};

template<>
struct hash<f32>
{
	u32 operator()(const f32 val) const
	{
		return val == 0.0f ? 0 : murmur32(&val, sizeof(val), 0);
	}
};

template<>
struct hash<f64>
{
	u32 operator()(const f64 val) const
	{
		return val == 0.0 ? 0 : murmur32(&val, sizeof(val), 0);
	}
};

} // namespace crown
