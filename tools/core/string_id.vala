/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
[Compact]
public struct StringId32
{
	uint32 _id;

	public StringId32(string str)
	{
		hash(str.data, str.length);
	}

	public void hash(uint8[] str, uint len)
	{
		_id = (uint32)(murmur64(str, str.length, 0) & 0xffffffffu);
	}

	public static uint hash_func(StringId32? id)
	{
		return (uint)id._id;
	}

	public static bool equal_func(StringId32? a, StringId32? b)
	{
		return a._id == b._id;
	}

	public string to_string()
	{
		return "%.8x".printf(_id);
	}
}

[Compact]
public struct StringId64
{
	uint64 _id;

	public StringId64(string str)
	{
		hash(str.data, str.length);
	}

	public void hash(uint8[] str, uint len)
	{
		_id = murmur64(str, str.length, 0);
	}

	public static uint hash_func(StringId64? id)
	{
		return (uint)id._id;
	}

	public static bool equal_func(StringId64? a, StringId64? b)
	{
		return a._id == b._id;
	}

	public string to_string()
	{
		return "%.8x%.8x".printf((uint32)((_id & 0xffffffff00000000u) >> 32)
			, (uint32)((_id & 0x00000000ffffffffu))
			);
	}
}

} /* namespace Crown */
