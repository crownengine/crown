/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

namespace Crown
{
[Compact]
public struct Guid
{
	uint32 data1;
	uint16 data2;
	uint16 data3;
	uint64 data4;

	public static Guid new_guid()
	{
		// FIXME: Replace Rand with something better.
		Rand rnd = new Rand();
		uint32 a = rnd.next_int();
		uint32 b = rnd.next_int();
		uint64 c = rnd.next_int();
		uint64 d = rnd.next_int();

		uint32 d1 = a;
		uint16 d2 = (uint16)((b & 0xffff0000u) >> 16);
		uint16 d3 = (uint16)((b & 0x0000ffffu) >>  0);
		uint64 d4 = (c << 32 | d);

		d3 = (d3 & 0x4fffu) | 0x4000u;
		d4 = (d4 & 0x3fffffffffffffffu) | 0x8000000000000000u;
		return { d1, d2, d3, d4 };
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
		uint32 d1 = a;
		uint16 d2 = (uint16)(b & 0x0000ffffu);
		uint16 d3 = (uint16)(c & 0x0000ffffu);
		uint64 d4 = (uint64)(d & 0x0000ffffu) << 48 | (uint64)(e & 0x0000ffffu) << 32 | (uint64)f;
		guid = { d1, d2, d3, d4 };
		return num == 6;
	}

	public string to_string()
	{
		return "%.8x-%.4x-%.4x-%.4x-%.4x%.8x".printf(data1
			, data2
			, data3
			, (uint32)((data4 & 0xffff000000000000u) >> 48)
			, (uint32)((data4 & 0x0000ffff00000000u) >> 32)
			, (uint32)((data4 & 0x00000000ffffffffu) >>  0)
			);
	}

	public static uint hash_func(Guid? id)
	{
		uint32 d1 = (uint32)(id.data1 & 0xff) << 24;
		uint32 d2 = (uint32)(id.data2 & 0xff) << 16;
		uint32 d3 = (uint32)(id.data3 & 0xff) <<  8;
		uint32 d4 = (uint32)(id.data4 & 0xff) <<  0;
		return d1 | d2 | d3 | d4;
	}

	public static bool equal_func(Guid? a, Guid? b)
	{
		return a.data1 == b.data1
			&& a.data2 == b.data2
			&& a.data3 == b.data3
			&& a.data4 == b.data4
			;
	}

	public static int compare_func(Guid a, Guid b)
	{
		if (a.data1 != b.data1)
			return a.data1 < b.data1 ? -1 : 1;
		if (a.data2 != b.data2)
			return a.data2 < b.data2 ? -1 : 1;
		if (a.data3 != b.data3)
			return a.data3 < b.data3 ? -1 : 1;
		if (a.data4 != b.data4)
			return a.data4 < b.data4 ? -1 : 1;

		return 0;
	}
}

const Guid GUID_ZERO = { 0, 0, 0, 0 };

}
