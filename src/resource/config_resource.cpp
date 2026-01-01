/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"
#include "core/filesystem/file.h"
#include "core/filesystem/filesystem.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/memory/allocator.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "resource/compile_options.inl"
#include "resource/types.h"

namespace crown
{
namespace config_resource_internal
{
	void *load(File &file, Allocator &a)
	{
		const u32 size = file.size();
		char *res = (char *)a.allocate(size + 1);
		file.read(res, size);
		res[size] = '\0';
		return res;
	}

	void unload(Allocator &a, void *resource)
	{
		a.deallocate(resource);
	}

} // namespace config_resource_internal

#if CROWN_CAN_COMPILE
namespace config_resource_internal
{
	s32 compile(CompileOptions &opts)
	{
		Buffer buf = opts.read();

		TempAllocator1024 ta;
		JsonObject boot(ta);
		RETURN_IF_ERROR(sjson::parse(boot, buf), opts);

		const char *boot_script_json  = boot["boot_script"];
		const char *boot_package_json = boot["boot_package"];
		RETURN_IF_FALSE(boot_script_json != NULL, opts, "'boot_script' must be specified.");
		RETURN_IF_FALSE(boot_package_json != NULL, opts, "'boot_package' must be specified.");

		DynamicString boot_script(ta);
		DynamicString boot_package(ta);
		RETURN_IF_ERROR(sjson::parse_string(boot_script, boot_script_json), opts);
		RETURN_IF_ERROR(sjson::parse_string(boot_package, boot_package_json), opts);
		RETURN_IF_RESOURCE_MISSING("lua", boot_script.c_str(), opts);
		RETURN_IF_RESOURCE_MISSING("package", boot_package.c_str(), opts);

		if (opts._bundle) {
			TempAllocator256 ta;
			DynamicString dest(ta);
			destination_path(dest, opts._resource_id);

			File *config = opts._data_filesystem.open(dest.c_str(), FileOpenMode::READ);
			file::copy(opts._output, *config, config->size());
			opts._data_filesystem.close(*config);
		} else {
			opts.write(buf);
		}

		return 0;
	}

} // namespace config_resource_internal
#endif // if CROWN_CAN_COMPILE

} // namespace crown
