/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"
#include "core/filesystem/file.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/memory/allocator.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "resource/compile_options.inl"
#include "resource/config_resource.h"
#include "resource/types.h"

namespace crown
{
namespace config_resource_internal
{
	void* load(File& file, Allocator& a)
	{
		const u32 size = file.size();
		char* res = (char*)a.allocate(size + 1);
		file.read(res, size);
		res[size] = '\0';
		return res;
	}

	void unload(Allocator& a, void* resource)
	{
		a.deallocate(resource);
	}

} // namespace config_resource_internal

#if CROWN_CAN_COMPILE
namespace config_resource_internal
{
	s32 compile(CompileOptions& opts)
	{
		Buffer buf = opts.read();

		TempAllocator1024 ta;
		JsonObject boot(ta);
		sjson::parse(boot, buf);

		const char* boot_script_json  = boot["boot_script"];
		const char* boot_package_json = boot["boot_package"];
		DATA_COMPILER_ASSERT(boot_script_json != NULL, opts, "'boot_script' must be specified.");
		DATA_COMPILER_ASSERT(boot_package_json != NULL, opts, "'boot_package' must be specified.");

		DynamicString boot_script(ta);
		DynamicString boot_package(ta);
		sjson::parse_string(boot_script, boot_script_json);
		sjson::parse_string(boot_package, boot_package_json);
		DATA_COMPILER_ASSERT_RESOURCE_EXISTS("lua", boot_script.c_str(), opts);
		DATA_COMPILER_ASSERT_RESOURCE_EXISTS("package", boot_package.c_str(), opts);

		opts.write(buf);

		return 0;
	}

} // namespace config_resource_internal
#endif // CROWN_CAN_COMPILE

} // namespace crown
