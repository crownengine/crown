/*
 * Copyright (c) 2012-2023 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
[Compact]
public struct Guid
{
	uint64 data1;
	uint64 data2;

	public static Guid new_guid()
	{
		// FIXME: Replace Rand with something better.
		Rand rnd = new Rand();
		uint64 a = rnd.next_int();
		uint64 b = rnd.next_int();
		uint64 c = rnd.next_int();
		uint64 d = rnd.next_int();

		uint64 d1;
		d1  = a << 32;
		d1 |= b <<  0;
		uint64 d2;
		d2  = c << 32;
		d2 |= d <<  0;

		d1 = (d1 & 0xffffffffffff4fffu) | 0x4000u;
		d2 = (d2 & 0x3fffffffffffffffu) | 0x8000000000000000u;
		return { d1, d2 };
	}

	public static Guid parse(string guid)
	{
		Guid g;
		bool success = Guid.try_parse(guid, out g);
		assert(success);
		return g;
	}

	public static bool try_parse(string str, out Guid guid)
	{
		uint32 a = 0;
		uint32 b = 0;
		uint32 c = 0;
		uint32 d = 0;
		uint32 e = 0;
		uint32 f = 0;
		int num = str.scanf("%8x-%4x-%4x-%4x-%4x%8x", &a, &b, &c, &d, &e, &f);
		uint64 d1;
		d1  = ((uint64)a) << 32;
		d1 |= ((uint64)b) << 16;
		d1 |= ((uint64)c) <<  0;
		uint64 d2;
		d2  = ((uint64)d) << 48;
		d2 |= ((uint64)e) << 32;
		d2 |= ((uint64)f) <<  0;
		guid = { d1, d2 };
		return num == 6;
	}

	public string to_string()
	{
		return "%.8x-%.4x-%.4x-%.4x-%.4x%.8x".printf((uint32)((data1 & 0xffffffff00000000u) >> 32)
			, (uint16)((data1 & 0x00000000ffff0000u) >> 16)
			, (uint16)((data1 & 0x000000000000ffffu) >>  0)
			, (uint16)((data2 & 0xffff000000000000u) >> 48)
			, (uint16)((data2 & 0x0000ffff00000000u) >> 32)
			, (uint32)((data2 & 0x00000000ffffffffu) >>  0)
			);
	}

	public static uint hash_func(Guid? id)
	{
		return (uint)(id.data1 ^ id.data2);
	}

	public static bool equal_func(Guid? a, Guid? b)
	{
		return a.data1 == b.data1
			&& a.data2 == b.data2
			;
	}

	public static int compare_func(Guid a, Guid b)
	{
		if (a.data1 != b.data1)
			return a.data1 < b.data1 ? -1 : 1;
		if (a.data2 != b.data2)
			return a.data2 < b.data2 ? -1 : 1;

		return 0;
	}
}

public const Guid GUID_ZERO = { 0u, 0u };

} /* namespace Crown */
