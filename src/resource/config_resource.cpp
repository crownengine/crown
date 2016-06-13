/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "allocator.h"
#include "compile_options.h"
#include "config_resource.h"
#include "json_object.h"
#include "resource_types.h"
#include "sjson.h"

namespace crown
{
namespace config_resource
{
	void compile(const char* path, CompileOptions& opts)
	{
		Buffer buf = opts.read(path);

		TempAllocator1024 ta;
		JsonObject boot(ta);
		sjson::parse(buf, boot);

		const char* boot_script_json  = boot["boot_script"];
		const char* boot_package_json = boot["boot_package"];
		RESOURCE_COMPILER_ASSERT(boot_script_json != NULL, opts, "'boot_script' must be specified.");
		RESOURCE_COMPILER_ASSERT(boot_package_json != NULL, opts, "'boot_package' must be specified.");

		DynamicString boot_script(ta);
		DynamicString boot_package(ta);
		sjson::parse_string(boot_script_json, boot_script);
		sjson::parse_string(boot_package_json, boot_package);
		RESOURCE_COMPILER_ASSERT_RESOURCE_EXISTS(RESOURCE_EXTENSION_SCRIPT, boot_script.c_str(), opts);
		RESOURCE_COMPILER_ASSERT_RESOURCE_EXISTS(RESOURCE_EXTENSION_PACKAGE, boot_package.c_str(), opts);

		opts.write(buf);
	}

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
} // namespace config_resource

} // namespace crown
