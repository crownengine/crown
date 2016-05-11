/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "dynamic_string.h"
#include "guid.h"
#include "platform.h"

#if CROWN_PLATFORM_POSIX
	#include <fcntl.h>
	#include <unistd.h>
	#include <errno.h>
#elif CROWN_PLATFORM_WINDOWS
	#include <objbase.h>
#endif // CROWN_PLATFORM_POSIX

namespace crown
{
namespace guid
{
	Guid new_guid()
	{
		Guid guid;
#if CROWN_PLATFORM_POSIX
		int fd = open("/dev/urandom", O_RDONLY);
		CE_ASSERT(fd != -1, "open: erron = %d", errno);
		read(fd, &guid, sizeof(guid));
		close(fd);
		guid.data3 = (guid.data3 & 0x4fffu) | 0x4000u;
		guid.data4 = (guid.data4 & 0x3fffffffffffffffu) | 0x8000000000000000u;
#elif CROWN_PLATFORM_WINDOWS
		HRESULT hr = CoCreateGuid((GUID*)&guid);
		CE_ASSERT(hr == S_OK, "CoCreateGuid: error");
		CE_UNUSED(hr);
#endif // CROWN_PLATFORM_POSIX
		return guid;
	}

	Guid parse(const char* str)
	{
		Guid guid;
		try_parse(str, guid);
		return guid;
	}

	bool try_parse(const char* str, Guid& guid)
	{
		CE_ASSERT_NOT_NULL(str);
		u32 a, b, c, d, e, f;
		int num = sscanf(str, "%8x-%4x-%4x-%4x-%4x%8x", &a, &b, &c, &d, &e, &f);
		guid.data1 = a;
		guid.data2 = (u16)(b & 0x0000ffffu);
		guid.data3 = (u16)(c & 0x0000ffffu);
		guid.data4 = (u64)(d & 0x0000ffffu) << 48 | (u64)(e & 0x0000ffffu) << 32 | (u64)f;
		return num == 6;
	}

	void to_string(const Guid& guid, DynamicString& s)
	{
		char str[36+1];
		snprintf(str, sizeof(str), "%.8x-%.4x-%.4x-%.4x-%.4x%.8x"
			, guid.data1
			, guid.data2
			, guid.data3
			, (u16)((guid.data4 & 0xffff000000000000u) >> 48)
			, (u16)((guid.data4 & 0x0000ffff00000000u) >> 32)
			, (u32)((guid.data4 & 0x00000000ffffffffu) >>  0)
			);
		s.set(str, sizeof(str)-1);
	}
}

} // namespace crown
