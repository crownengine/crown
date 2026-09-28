#pragma once

// Resource I/O/refcounts are simulated. No filesystem, GPU or background loader
// is used. The complete compiler and the library-owner implementation are real.
#include "test_sdk.h"

#define RESOURCE_TYPE_SHADER crown::StringId64(UINT64_C(0xcce8d5b5f5ae333f))

namespace crown
{
struct ResourcePackage;
struct ResourceManager
{
	struct CanGetFlags { enum Enum { STRICT = 1 }; };
	std::map<u64, u32> references;
	std::vector<u64> pending;
	std::vector<std::string> events;

	void load(ResourcePackage *, StringId64 type, StringId64 id, u32)
	{
		assert(type == RESOURCE_TYPE_SHADER);
		events.push_back("load:" + std::to_string(id._id));
		if (references[id._id] != 0)
			++references[id._id];
		else
			pending.push_back(id._id);
	}

	void complete_requests()
	{
		for (u64 id : pending) {
			assert(references[id] == 0);
			references[id] = 1;
			events.push_back("online:" + std::to_string(id));
		}
		pending.clear();
	}

	bool can_get(StringId64 type, StringId64 id, u32 flags)
	{
		assert(type == RESOURCE_TYPE_SHADER && flags == CanGetFlags::STRICT);
		return references[id._id] != 0;
	}

	void unload(StringId64 type, StringId64 id)
	{
		assert(type == RESOURCE_TYPE_SHADER && references[id._id] != 0);
		events.push_back("unload:" + std::to_string(id._id));
		--references[id._id];
	}
};
} // namespace crown
